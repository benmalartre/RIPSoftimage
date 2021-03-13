# -------------------------------------------------------------------
# Utils
# -------------------------------------------------------------------
from Globals import XSI
from Globals import XSIMath
from Globals import XSIFactory
from win32com.client import constants
from Constants import *
import math


def GetPluginPath(name):
	""" Get plugin path gievn plugin name
	"""
	plugins = XSI.Plugins
	for plugin in plugins:
		if plugin.Name == name:
			return plugin.OriginPath
	return ""


def GetPrefixFromSide(side):
	""" Get prefix from side
	"""
	prefix = 'M_'
	if side == LEFT:
		prefix = 'L_'
	elif side == RIGHT:
		prefix = 'R_'
	return prefix


def GetClusterByName(obj, name):
	""" Get cluster by name
	"""
	return obj.ActivePrimitive.Geometry.Clusters(name)


def CreateClusterFromSelection(name='ClsFromSelection'):
	""" Create cluster from selection
	"""
	selection = XSI.Selection(0)
	if not selection:
		XSI.LogMessage('[Utils] CreateClusterFromSelection failed as nothing selected !')
		return None
		
	obj = selection.SubComponent.Parent3DObject
	if obj.ActivePrimitive.Geometry.Clusters(name):
		cluster = obj.ActivePrimitive.Geometry.Clusters(name)
		return cluster
	
	if selection.Type == 'pntSubComponent':
		sub = selection.SubComponent
		if sub.Parent3DObject.FullName == obj.FullName:
			cluster = sub.CreateCluster(name)
			return cluster

	elif selection.Type == 'polySubComponent':
		sub = selection.SubComponent
		if sub.Parent3DObject.FullName == obj.FullName:
			cluster = sub.CreateCluster(name)
			return cluster


def CreateAlwaysCompleteCluster(obj, _type=constants.siVertexCluster, name='AlwaysCompleteCls'):
	""" Create alwyas-complete cluster
	"""
	geom = obj.ActivePrimitive.Geometry
	if not geom:
		XSI.LogMessage('[CreateAlwaysCompleteCluster] : input object invalid !')
		return False

	# check if cluster doesn't exists
	if geom.Clusters(name):
		cls = geom.Clusters(name)
	else:
		cls = geom.AddCluster(_type, name)
	return cls


# ----------------------------------------------------
# Create Complete But Not Always Cluster
# ----------------------------------------------------
def CreateCompleteButNotAlwaysCluster(obj, _type=constants.siVertexCluster, name='CompleteButNotAlwaysCls'):
	geom = obj.ActivePrimitive.Geometry
	if not geom:
		XSI.LogMessage('[CreateCompleteButNotAlwaysCluster] : input object invalid !')
		return None

	# check if cluster doesn't exists
	if geom.Clusters(name):
		cluster = geom.Clusters(name)
		return cluster
	else:
		if _type == constants.siVertexCluster:
			indices = geom.Points.IndexArray
			cluster = geom.AddCluster(_type, name, indices)
			return cluster
		elif _type == constants.siEdgeCluster:
			indices = geom.Segments.IndexArray
			cluster = geom.AddCluster(_type, name, indices)
			return cluster
		elif _type == constants.siPolygonCluster:
			indices = geom.Facets.IndexArray
			cluster = geom.AddCluster(_type, name, indices)
			return cluster
		else:
			XSI.LogMessage('[CreateAlwaysCompleteCluster] : cluster type {} isn\'t supported !'.format(_type))
			return None


# ----------------------------------------------------
# Update Cluster Component
# ----------------------------------------------------
def UpdateClusterComponent(cluster, subcomponent, method):
	# methods
	# 0 = Exclusive Add Selected Points
	# 1 = Add Selected Points
	# 2 = Remove Selected Points
	if method == 0:
		XSI.SIRemoveFromCluster(cluster, cluster.CreateSubComponent())
		XSI.SIAddToCluster(cluster, subcomponent)
	elif method == 1:
		XSI.SIAddToCluster(cluster, subcomponent)
	elif method == 2:
		XSI.SIRemoveFromCluster(cluster, subcomponent)


# ----------------------------------------------------
# Mirror Vertex Cluster
# ----------------------------------------------------
def MirrorVertexCluster(cluster, weightmap=False, name="WeightMap"):
	parent_object = cluster.Parent3DObject
	geometry = parent_object.ActivePrimitive.Geometry
	
	symmetry_weightmap = GetSymmetryMap(parent_object)
	symmetry_elements = symmetry_weightmap.Elements.Array
	
	cluster_name = cluster.Name
	symmetry_cluster = None
	self_symmetry = False
	
	if cluster_name.find('L_', 0, 2) > -1:
		symmetry_name = cluster_name.replace('L_', 'R_')
		symmetry_cluster = parent_object.ActivePrimitive.Geometry.Clusters(symmetry_name)
		
	elif cluster_name.find('R_', 0, 2) > -1:
		symmetry_name = cluster_name.replace('R_', 'L_')
		symmetry_cluster = parent_object.ActivePrimitive.Geometry.Clusters(symmetry_name)
		
	else:
		symmetry_cluster = cluster
		self_symmetry = True
		
	# symmetrize cluster on itself from left to right
	if self_symmetry:
		to_add = []
		position = geometry.Points.PositionArray
		for i in range(len(position[0])):
			if not position[0][i] <= 0:
				continue
			# find symmetric point
			symmetry_point = int(symmetry_elements[0][i])
			
			# check if symmetry point is in cluster
			if cluster.Elements.FindIndex(symmetry_point) > -1:
				to_add.append(i)
			
		sub_comp = geometry.CreateSubComponent(constants.siVertexCluster, to_add)
		UpdateClusterComponent(cluster, sub_comp, 1)
		if weightmap:
			weightmap = GetWeightMap(parent_object, name, 1.0, 0.0, 1.0, cluster)
			SymmetrizeWeights(weightmap, 0, False)

	# symmetrize on another cluster
	else:
		elements = cluster.Elements.Array
		tmp = [0 for _ in elements]
		for i in range(len(elements)):  
			tmp[i] = symmetry_elements[0][elements[i]]
		
		sub_comp = geometry.CreateSubComponent(constants.siVertexCluster, tmp)
		UpdateClusterComponent(symmetry_cluster, sub_comp, 0)
		if weightmap:
			weightmap = GetWeightMap(parent_object, name, 1.0, 0.0, 1.0, cluster)
			symmetry_weightmap = GetWeightMap(parent_object, name, 1.0, 0.0, 1.0, symmetry_cluster)
			if weightmap and symmetry_weightmap:
				symmetry_weightmap.Elements.Array = weightmap.Elements.Array
			else:
				SetWeightMap(symmetry_weightmap, 1.0, 0.0, 1.0)


# ----------------------------------------------------
# Static Kinematic State
# ----------------------------------------------------
def ResetStaticKinematicState(objs):
	out_kinematic_state = []
	for obj in objs:
		static_kinestate = obj.Properties('Static_KineState')
		if not static_kinestate:
			static_kinestate = obj.AddProperty('Static Kinematic State Property')
			static_kinestate.Name = 'Static_KineState'
		
		kine = obj.Kinematics.Global
		static_kinestate.Parameters('PosX').Value = kine.Parameters('PosX').Value
		static_kinestate.Parameters('PosY').Value = kine.Parameters('PosY').Value
		static_kinestate.Parameters('PosZ').Value = kine.Parameters('PosZ').Value
		static_kinestate.Parameters('OriX').Value = kine.Parameters('RotX').Value
		static_kinestate.Parameters('OriY').Value = kine.Parameters('RotY').Value
		static_kinestate.Parameters('OriZ').Value = kine.Parameters('RotZ').Value
		static_kinestate.Parameters('SclX').Value = kine.Parameters('SclX').Value
		static_kinestate.Parameters('SclY').Value = kine.Parameters('SclY').Value
		static_kinestate.Parameters('SclZ').Value = kine.Parameters('SclZ').Value
		out_kinematic_state.append(static_kinestate)
		
	return out_kinematic_state


# ----------------------------------------------------
# Wirecolor
# ----------------------------------------------------
def SetWireColor(obj, R, G, B):
	p = obj.AddProperty("Display Property")
	p.Parameters("WireColorR").Value = R
	p.Parameters("WireColorG").Value = G
	p.Parameters("WireColorB").Value = B


def GetWireColor(obj):
	outColor = [0, 0, 0]
	outColor[0] = obj.Properties('Display').Parameters("WireColorR").Value
	outColor[1] = obj.Properties('Display').Parameters("WireColorG").Value
	outColor[2] = obj.Properties('Display').Parameters("WireColorB").Value
	return outColor


def GetCustomColor(obj):
	outColor = [0, 0, 0]
	outColor[0] = obj.ActivePrimitive.Parameters("R").Value
	outColor[1] = obj.ActivePrimitive.Parameters("G").Value
	outColor[2] = obj.ActivePrimitive.Parameters("B").Value
	return outColor


# ----------------------------------------------------
# Match Transform
# ----------------------------------------------------
def MatchTransform(Obj, Target):
	T = Target.Kinematics.Global.Transform
	Obj.Kinematics.Global.Transform = T


def MatchRotation(Obj,Target):
    T = Target.Kinematics.Global.Transform
    O = Obj.Kinematics.Global.Transform
    R = XSIMath.CreateRotation()
    T.GetRotation(R)
    O.SetRotation(R)
    Obj.Kinematics.Global.Transform = O


def MatchPosition(Obj,Target):
    T = Target.Kinematics.Global.Transform
    O = Obj.Kinematics.Global.Transform
    O.SetTranslation(T.Translation)
    Obj.Kinematics.Global.Transform = O


# ----------------------------------------------------
# Set Transform
# ----------------------------------------------------
def SetPosition(obj, pos):
	T = obj.Kinematics.Global.GetTransform2(obj)
	T.SetTranslation(pos)
	obj.Kinematics.Global.PutTransform2(obj, T)


def SetOrientation(obj, rot):
	T = obj.Kinematics.Global.GetTransform2(obj)
	T.SetRotation(rot)
	obj.Kinematics.Global.PutTransform2(obj, T)


def SetScaling(obj, scl):
	T = obj.Kinematics.Global.GetTransform2(obj)
	T.SetScaling(scl)
	obj.Kinematics.Global.PutTransform2(obj, T)


# ----------------------------------------------------
# Simple Geometry
# ----------------------------------------------------
def SimpleSymmetry(obj, target, axis):
	oG = obj.Kinematics.Global
	tG = target.Kinematics.Global
	
	if axis == XY_AXIS:
		x = "- "+str(tG.Parameters("posx"))
		oG.Parameters("posx").AddExpression(x)
		y = str(tG.Parameters("posy"))
		oG.Parameters("posy").AddExpression(y)
		z = str(tG.Parameters("posz"))
		oG.Parameters("posz").AddExpression(z)
		
		x = str(tG.Parameters("rotx"))
		oG.Parameters("rotx").AddExpression(x)
		y = "- "+str(tG.Parameters("roty"))
		oG.Parameters("roty").AddExpression(y)
		z = "- "+str(tG.Parameters("rotz"))
		oG.Parameters("rotz").AddExpression(z)
		
		x = str(tG.Parameters("sclx"))
		oG.Parameters("sclx").AddExpression(x)
		y = str(tG.Parameters("scly"))
		oG.Parameters("scly").AddExpression(y)
		z = str(tG.Parameters("sclz"))
		oG.Parameters("sclz").AddExpression(z)
		
	elif axis == YZ_AXIS:
		x = str(tG.Parameters("posx"))
		oG.Parameters("posx").AddExpression(x)
		y = str(tG.Parameters("posy"))
		oG.Parameters("posy").AddExpression(y)
		z = "- "+str(tG.Parameters("posz"))
		oG.Parameters("posz").AddExpression(z)
		
		x = "- "+str(tG.Parameters("rotx"))
		oG.Parameters("rotx").AddExpression(x)
		y = "- "+str(tG.Parameters("roty"))
		oG.Parameters("roty").AddExpression(y)
		z = str(tG.Parameters("rotz"))
		oG.Parameters("rotz").AddExpression(z)
		
		x = str(tG.Parameters("sclx"))
		oG.Parameters("sclx").AddExpression(x)
		y = str(tG.Parameters("scly"))
		oG.Parameters("scly").AddExpression(y)
		z = str(tG.Parameters("sclz"))
		oG.Parameters("sclz").AddExpression(z)

	elif axis == XZ_AXIS:
		x = str(tG.Parameters("posx"))
		oG.Parameters("posx").AddExpression(x)
		y = "- "+str(tG.Parameters("posy"))
		oG.Parameters("posy").AddExpression(y)
		z = str(tG.Parameters("posz"))
		oG.Parameters("posz").AddExpression(z)
		
		x = "- "+str(tG.Parameters("rotx"))
		oG.Parameters("rotx").AddExpression(x)
		y = "- "+str(tG.Parameters("roty"))
		oG.Parameters("roty").AddExpression(y)
		z = str(tG.Parameters("rotz"))
		oG.Parameters("rotz").AddExpression(z)
		
		x = str(tG.Parameters("sclx"))
		oG.Parameters("sclx").AddExpression(x)
		y = str(tG.Parameters("scly"))
		oG.Parameters("scly").AddExpression(y)
		z = str(tG.Parameters("sclz"))
		oG.Parameters("sclz").AddExpression(z)


# ----------------------------------------------------
# Match Symmetry
# ----------------------------------------------------
def MatchSymmetry(obj, target, axis):
	oG = obj.Kinematics.Global
	tG = target.Kinematics.Global
	
	if axis == XY_AXIS:
		x = - tG.Parameters("posx").Value
		oG.Parameters("posx").Value = x
		y = tG.Parameters("posy").Value
		oG.Parameters("posy").Value = y
		z = tG.Parameters("posz").Value
		oG.Parameters("posz").Value = z
		
		x = tG.Parameters("rotx").Value
		oG.Parameters("rotx").Value = x
		y = -tG.Parameters("roty").Value
		oG.Parameters("roty").Value = y
		z = -tG.Parameters("rotz").Value
		oG.Parameters("rotz").Value = z
		
		x = tG.Parameters("sclx").Value
		oG.Parameters("sclx").Value = x
		y = tG.Parameters("scly").Value
		oG.Parameters("scly").Value = y
		z = tG.Parameters("sclz").Value
		oG.Parameters("sclz").Value = z
		
	elif axis == YZ_AXIS:
		x = tG.Parameters("posx").Value
		oG.Parameters("posx").Value = x
		y = tG.Parameters("posy").Value
		oG.Parameters("posy").Value = y
		z = -tG.Parameters("posz").Value
		oG.Parameters("posz").Value = z
		
		x = -tG.Parameters("rotx").Value
		oG.Parameters("rotx").Value = x
		y = -tG.Parameters("roty").Value
		oG.Parameters("roty").Value = y
		z = tG.Parameters("rotz").Value
		oG.Parameters("rotz").Value = z
		
		x = tG.Parameters("sclx").Value
		oG.Parameters("sclx").Value = x
		y = tG.Parameters("scly").Value
		oG.Parameters("scly").Value = y
		z = tG.Parameters("sclz").Value
		oG.Parameters("sclz").Value = z
	elif axis == XZ_AXIS:
		x = tG.Parameters("posx").Value
		oG.Parameters("posx").Value = x
		y = - tG.Parameters("posy").Value
		oG.Parameters("posy").Value = y
		z = tG.Parameters("posz").Value
		oG.Parameters("posz").Value = z
		
		x = -tG.Parameters("rotx").Value
		oG.Parameters("rotx").Value = x
		y = - tG.Parameters("roty").Value
		oG.Parameters("roty").Value = y
		z = tG.Parameters("rotz").Value
		oG.Parameters("rotz").Value = z
		
		x = tG.Parameters("sclx").Value
		oG.Parameters("sclx").Value = x
		y = tG.Parameters("scly").Value
		oG.Parameters("scly").Value = y
		z = tG.Parameters("sclz").Value
		oG.Parameters("sclz").Value = z


# ----------------------------------------------------
# Get Positions
# ----------------------------------------------------
def GetPositions(objects):
	outPos = []
	for o in objects:
		t = o.Kinematics.Global.Transform
		p = XSIMath.CreateVector3()
		t.GetTranslation(p)
		outPos.append(p)
		
	return outPos


# ----------------------------------------------------
# Get Average Position
# ----------------------------------------------------
def GetAveragePosition(positions):
	x = XSIMath.CreateVector3()
	for p in positions:
		x.AddInPlace(p)
	
	x.ScaleInPlace(1.0/len(positions))
	return x


# ----------------------------------------------------
# Get Orientation from Picked Position
# ----------------------------------------------------
def GetOrientationFromPickPosition(pos):
	if isinstance(pos, list) and len(pos) > 1:
		rot = []

		vm = XSI.Desktop.ActiveLayout.Views("vm")
		focus = vm.GetAttributeValue("focusedviewport")
		if focus:
			cam = vm.GetAttributeValue("activecamera:"+focus)
		else:
			cam = "Right"

		y = XSIMath.CreateVector3(0, 1, 0)

		if cam == "Right":
			y.Set(1, 0, 0)
		elif cam == "Top":
			y.Set(0, 1, 0)
		elif cam == "Front":
			y.Set(0, 0, 1)
		else:
			x1 = XSIMath.CreateVector3()
			x2 = XSIMath.CreateVector3()
			x1.Sub(pos[1], pos[0])
			x2.Sub(pos[len(pos)-1], pos[len(pos)-2])

			y.Cross(x1, x2)
			y.NormalizeInPlace()

		for idx in range(len(pos)-1):
			start = pos[idx]
			end = pos[idx+1]
			delta = XSIMath.CreateVector3()
			delta.Sub(end, start)
			'''
			n = AddNull(xsi.ActiveSceneRoot,4,1,"Start")
			t = n.Kinematics.Global.Transform
			t.SetTranslation(start)
			n.Kinematics.Global.Transform = t
			
			n = AddNull(xsi.ActiveSceneRoot,4,1,"End")
			t = n.Kinematics.Global.Transform
			t.SetTranslation(end)
			n.Kinematics.Global.Transform = t
			'''
			r = GetRotationFromTwoVectors(delta, y)
			rot.append(r)

		# duplicate the last array item
		rot.append(rot[len(rot)-1])

		return rot
	else:
		return XSIMath.CreateRotation()


# ----------------------------------------------------
# Rotation From Two Vectors
# ----------------------------------------------------
def GetRotationFromTwoVectors(direction, up=XSIMath.CreateVector3(0, 1, 0)):
	cross = XSIMath.CreateVector3()
	side = XSIMath.CreateVector3()

	direction.NormalizeInPlace()
	up.NormalizeInPlace()
	cross.Cross(direction, up)
	cross.NormalizeInPlace()
	side.Cross(cross, direction)
	side.NormalizeInPlace()
	
	m = XSIMath.CreateMatrix3()
	m.Set(cross.X, cross.Y, cross.Z, direction.X, direction.Y, direction.Z, side.X, side.Y, side.Z)
	
	r = XSIMath.CreateRotation()
	r.SetFromMatrix3(m)
	return r


# ----------------------------------------------------
# Project On Plane
# ----------------------------------------------------
def ProjectOnPlane(inPoint, inPlaneNormal):
	vProjection = XSIMath.CreateVector3()
	pProjection = XSIMath.CreateVector3()

	dot = inPoint.Dot(inPlaneNormal)
	scaleChange = (dot/math.pow(inPlaneNormal.Length(), 2))
	vProjection.Scale(scaleChange, inPlaneNormal)
	pProjection.Sub(inPoint, vProjection)
	return pProjection


# ----------------------------------------------------
# Selection
# ----------------------------------------------------
def SelectionToCollection(filter=None):
	out = XSIFactory.CreateActiveXObject("XSI.Collection")
	sel = XSI.Selection
	for s in sel:
		if filter:
			if s.Type == filter:
				out.Add(s)
		else:
			out.Add(s)
	return out


# ----------------------------------------------------
# Get Distance
# ----------------------------------------------------
def DistanceBetweenTwoObjects(start, end):
	if not start or not end:
		XSI.LogMessage("[Distance between two objects] Invalid Inputs!!", constants.siWarning)
	sT = start.Kinematics.Global.Transform
	eT = end.Kinematics.Global.Transform
	delta = XSIMath.CreateVector3()
	delta.Sub(eT.Translation, sT.Translation)
	return delta.Length()


# ----------------------------------------------------
# Visibility
# ----------------------------------------------------
def SetVisibility(objects, viewvis=True, rendvis=True, selectability=True):
	for o in objects:
		if o.IsClassOf(constants.siX3DObjectID):
			v = o.Properties("Visibility")
			v.Parameters("viewvis").Value = viewvis
			v.Parameters("rendvis").Value = rendvis
			v.Parameters("selectability").Value = selectability


# ----------------------------------------------------
# Make Rig Null
# ----------------------------------------------------
def MakeRigNull(inParent=None, inType=0, inName="Null"):
	if not inParent:
		inParent = XSI.ActiveSceneRoot
	n = inParent.AddNull(inName)
	MatchTransform(n, inParent)
	
	# buffer null
	if inType == 0:
		n.ActivePrimitive.Size = .1
		n.ActivePrimitive.Primary_Icon = 0
		
	# control null
	elif inType == 1:
		n.ActivePrimitive.Size = .1
		n.ActivePrimitive.Primary_Icon = 2
		SetWireColor(n, 1, 0, 0)
	
	# deformer null
	elif inType == 2:
		n.ActivePrimitive.Size = .1
		n.ActivePrimitive.Primary_Icon = 4
		SetWireColor(n, 0.7, 1, 0.7)
	return n


# ----------------------------------------------------
# Add Null
# ----------------------------------------------------
def AddNull(inParent=None, inType=0, inSize=1, inName="Null", inR=0, inG=0, inB=0):
	if not inParent:
		inParent = XSI.ActiveSceneRoot
		
	n = inParent.AddNull(inName)
	MatchTransform(n, inParent)
	
	n.ActivePrimitive.Size = inSize
	n.ActivePrimitive.Primary_Icon = inType
		
	SetWireColor(n, inR, inG, inB)
	return n


# ----------------------------------------------------
# Group Setup
# ----------------------------------------------------
def GroupSetup(inModel, inObjects=None, inGroupName="Group"):
	group = inModel.Groups(inGroupName)
	if not group:
		group = inModel.AddGroup()
		group.Name = inGroupName
	if inObjects:
		for o in inObjects:
			group.AddMember(o)
	return group


# ----------------------------------------------------
# Object In Group
# ----------------------------------------------------
def ObjectInGroup(obj, group):
	for m in group.Members:
		if m.FullName == obj.FullName:
			return True
	return False


# ----------------------------------------------------
# Pick Session
# ----------------------------------------------------
def PickElement(inFilter=constants.siGenericObjectFilter, inMessage="Pick Element"):
	rtn = XSI.PickElement(inFilter, inMessage, inMessage)
	element = rtn.Value("PickedElement")
	button = rtn.Value("ButtonPressed")

	if button:
		return element
	
	return None


def PickMultiElement(inType=None, inMessage="Pick Multi Element"):
	outObj = []
	check = 0

	while not check:
		pickElement = PickElement(inType, inMessage)
		if pickElement:
			outObj.append(pickElement)
		else:
			check = 1
		
	return outObj


def PickPosition(inMessage="Pick Position"):
	if not inMessage:
		inMessage = "Pick Position"
	picked = XSI.PickPosition(inMessage, inMessage, None, None, None, None)
	button = picked("ButtonPressed")
	if button:
		x = picked("PosX")
		y = picked("PosY")
		z = picked("PosZ")
		outVec = XSIMath.CreateVector3(x, y, z)
		return outVec
	else:
		return None


def PickPositionInVolume(inMessage = "Pick Position"):
	# enable snap option
	XSI.SetValue("preferences.SnapProperties.Enable2D", True, None)
	XSI.SetValue("preferences.SnapProperties.Enable", True, None)
	
	if not inMessage:
		inMessage = "Pick Position"

	picked = XSI.PickPosition(inMessage, inMessage, None, None, None, None)
	
	button = picked("ButtonPressed")
	if button:
		x = picked("PosX")
		y = picked("PosY")
		z = picked("PosZ")
	else:
		return None
	outVec = XSIMath.CreateVector3(x, y, z)

	# disable snap option
	XSI.SetValue("preferences.SnapProperties.Enable2D", False, None)
	XSI.SetValue("preferences.SnapProperties.Enable", False, None)
	
	return outVec


# ----------------------------------------------------
# Pick Multi Positions
# ----------------------------------------------------
def PickMultiPosition(inNb=-1, inBoundingVolume=False):
	outPos = []
	check = 0
	while not check:
		inNb -= 1
		if inBoundingVolume:
			pickPosition = PickPositionInVolume("Pick Position")
		else:
			pickPosition = PickPosition("Pick Position")
		if pickPosition:
			outPos.append(pickPosition)
		else:
			check = 1
			
		if inNb == 0:
			check = True
			
	return outPos


# ----------------------------------------------------
# Cluster Center Target
# ----------------------------------------------------
def ClusterCenterTarget(inPntSubComponent=None, inTarget=None, inName="PntCls"):
	if not inPntSubComponent:
		sel = XSI.Selection(0)
		if not sel or not sel.Type == "pntSubComponent":
			XSI.LogMessage("[ClusterCenterTarget] Invalid Selection : Need some points selected ---> Aborted!")
			return
		else:
			inPntSubComponent = sel.SubComponent
	if not inTarget:
		pick = PickElement(constants.siObjectFilter, "Pick Target Object")
		if not pick:
			XSI.LogMessage("[ClusterCenterTarget] Invalid Target : Need one target object ---> Aborted!")
			return
		else:
			inTarget = pick
	obj = inPntSubComponent.Parent3DObject
	geo = obj.ActivePrimitive.Geometry
	elem = inPntSubComponent.ElementArray
	cls = geo.AddCluster(constants.siVertexCluster, inName, elem)
	XSI.ApplyOp("ClusterCenter", str(cls) + ";" + str(inTarget), 0, 0, None, 2)


# ----------------------------------------------------
# Build Curve On Objects Position
# ----------------------------------------------------
def BuildCurveOnObjectsPosition(inObjects, inDegree=0, inClose=0, inConstraint=1, inOffset=XSIMath.CreateVector3()):
	if len(inObjects) < 2:
		XSI.LogMessage("BuildCurveOnPoints aborted :: Not enough input objects!!", constants.siError)
		return
		
	if len(inObjects) < 4:
		inDegree = 0
		
	pPos = GetPositions(inObjects)

	aPos = []
	for p in pPos:
		aPos.append(p.X+inOffset.X)
		aPos.append(p.Y+inOffset.Y)
		aPos.append(p.Z+inOffset.Z)
		aPos.append(1)

	if inDegree == 0:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(aPos, None, inClose, 1)
		
	else:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(aPos, None, inClose, 3)
	
	if inConstraint == 1:
		crvGeom = outCrv.ActivePrimitive.Geometry
		for p in crvGeom.Points:
			oCluster = crvGeom.AddCluster(constants.siVertexCluster, "Pnt"+str(p.Index+1), p.Index)
			XSI.ApplyOp("ClusterCenter", str(oCluster) + ";" + str(inObjects[p.Index]), 0, 0, None, 2)

	return outCrv


# ----------------------------------------------------
# Build Curve On Positions
# ----------------------------------------------------
def BuildCurveOnPositions(inPositions, inDegree=0, inClose=0, inOffset=XSIMath.CreateVector3()):
	if len(inPositions) < 2:
		XSI.LogMessage("BuildCurveOnPosition aborted :: Not enough input positions!!", constants.siError)
		return
		
	if len(inPositions) < 4:
		inDegree = 0
		
	aPos = []
	for p in inPositions:
		aPos.append(p.X+inOffset.X)
		aPos.append(p.Y+inOffset.Y)
		aPos.append(p.Z+inOffset.Z)
		aPos.append(1)
	
	if inDegree == 0:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(aPos, None, inClose, 1)
	else:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(aPos, None, inClose, 3)

	return outCrv


# ----------------------------------------------------
# Build Curve On Symmetrized Positions
# ----------------------------------------------------
def BuildCurveOnSymmetrizedPositions(inPositions, inAxis = 0, inDegree=0, inClose=0):

	if len(inPositions) < 1:
		XSI.LogMessage("BuildCurveOnSymmetrizedPosition aborted :: Not enough input positions!!", constants.siError)
		return
	axis = XSIMath.CreateVector3(-1, 1, 1)
	if inAxis == 1:
		axis.Set(1, -1, 1)
	elif inAxis == 2:
		axis.Set(1, 1, -1)
	newPos = []
	last = len(inPositions)-1
	for i in range(len(inPositions)):
		p = inPositions[last-i]
		newPos.append(p.X*axis.X)
		newPos.append(p.Y*axis.Y)
		newPos.append(p.Z*axis.Z)
		newPos.append(1)
	# middle pos
	newPos.append(0)
	newPos.append(inPositions[0].Y)
	newPos.append(inPositions[0].Z)
	newPos.append(1)
	
	for p in inPositions:
		newPos.append(p.X)
		newPos.append(p.Y)
		newPos.append(p.Z)
		newPos.append(1)
		
	if inDegree == 0:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(newPos, None, inClose, 1)
		
	else:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(newPos, None, inClose, 3)
		
	return outCrv


# ----------------------------------------------------
# Replace Curve Geometry
# ----------------------------------------------------
def ReplaceCurveGeometry(dst, src):
	if src and src.Type == "crvlist" and dst and dst.Type == "crvlist":
		datas = src.ActivePrimitive.Geometry.Get2()
		XSI.FreezeObj(dst)
		dst.ActivePrimitive.Geometry.Set(datas[0], datas[1], datas[2], datas[3], datas[4], datas[5], datas[6], datas[7])


# ----------------------------------------------------
# Get Curve Length
# ----------------------------------------------------
def GetCurveLength(crv):	
	if crv and crv.Type == "crvlist":
		return crv.ActivePrimitive.Geometry.Length


# ----------------------------------------------------
# Get Symmetry Map
# ----------------------------------------------------
def GetSymmetryMap(inObject):
	symmetrymaps = XSI.FindObjects(None, "{2EBA6DE4-B7EA-4877-80FE-FC768F32729F}")

	for symap in symmetrymaps:
		if symap.Parent3DObject.IsEqualTo(inObject):
			return symap
				
	# create cluster if not exist
	cluster = inObject.ActivePrimitive.Geometry.Clusters("SymmetryMapCls")
	if not cluster:
		cluster = inObject.ActivePrimitive.Geometry.AddCluster(constants.siVertexCluster, "SymmetryMapCls")

	symap = XSI.CreateSymmetryMap("SymmetryMap", cluster, "SymmetryMap")(0)
	return symap


# ----------------------------------------------------
# Get Weight Map
# ----------------------------------------------------
def GetWeightMap(inObject, name="WeightMap", value=0, inMin=0, inMax=1, cls=None):
	cluster = None
	
	# if cls provided check only it
	if cls:
		cluster = cls
		for w in cls.LocalProperties.Filter("wtmap"):
			if w.Name == name:
				return w
	
	else:
		XSI.LogMessage(inObject)
		if inObject.ActivePrimitive.Geometry.Clusters:
			# check all cluster for weightmap already exists
			for cls in inObject.ActivePrimitive.Geometry.Clusters.Filter("pnt"):
				if cls.Name == "WeightMapCls":
					cluster = cls
				for w in cls.LocalProperties.Filter("wtmap"):
					if w.Name == name:
						return w
	
	# create cluster if not exist
	if not cluster:
		cluster = inObject.ActivePrimitive.Geometry.AddCluster(constants.siVertexCluster, "WeightMapCls")
	
	# create and set weight map 
	wm = cluster.AddProperty("Weight Map Property", False, name)

	# clamp between -1 and 1  (maximum possible values)
	if inMin < -1:
		inMin = -1
	elif inMin > 0:
		inMin = 0
	if inMax > 1:
		inMax = 1
	elif inMax < 0:
		inMax = 0

	wm.Parameters("wmin").Value = inMin
	wm.Parameters("wmax").Value = inMax

	elems_tuple = wm.Elements.Array
	elems = [value for _ in range(len(elems_tuple[0])) for _ in range(len(elems_tuple))]
	wm.Elements.Array = elems
	
	XSI.FreezeObj(wm)
	
	return wm
	
# ----------------------------------------------------
# Copy Weight Map
# ----------------------------------------------------
def CopyWeightMap(src, dst, wmName):
	# first check geometry input
	snbp = src.ActivePrimitive.Geometry.Points.Count
	dnbp = dst.ActivePrimitive.Geometry.Points.Count
	
	if not snbp == dnbp:
		XSI.LogMessage("[Utils.py] Copy Weight Map Aborted!!!(Objects doesn't have same number of points)", constants.siWarning)
		return False
	
	swm = GetWeightMap(src, wmName)
	dwm = GetWeightMap(dst, wmName)
	
	if swm and dwm:
		XSI.LogMessage(swm)
		XSI.LogMessage(dwm)
		XSI.LogMessage(dwm.Elements.Array)
		dwm.Elements.Array = swm.Elements.Array
		XSI.LogMessage(dwm.Elements.Array)
	

# ----------------------------------------------------
# Set Weight Map
# ----------------------------------------------------
def SetWeightMap(weightmap, value=1.0, inMin=0.0, inMax=1.0):
	# create and set weight map 
	weightmap.Parameters("wmin").Value = inMin
	weightmap.Parameters("wmax").Value = inMax
	elems_tuple = weightmap.Elements.Array
	elems = [value for _ in range(len(elems_tuple[0])) for _ in range(len(elems_tuple))]
	weightmap.Elements.Array = elems
	
	XSI.FreezeObj(weightmap)
	
	return weightmap


# ----------------------------------------------------
# Set Weight Map Value on selected points
# ----------------------------------------------------
def SetWeightOnSelectedPoints(weightmap, weight=0, pntsubcomponent=None):
	if not weightmap or not weightmap.Type == "wtmap":
		XSI.LogMessage("[SetWeightOnSelectedPoints] Invalid Weight Map ---> Aborted!")
		return
			
	if not pntsubcomponent:
		sel = XSI.Selection(0)
		if not sel or not sel.Type == "pntSubComponent":
			XSI.LogMessage("[SetWeightOnSelectedPoints] Invalid Selection : Need some points selected ---> Aborted!")
			return
		else:
			pntsubcomponent = sel.SubComponent
		
	weights = [w for w in weightmap.Elements.Array[0]]

	# process
	for i in pntsubcomponent.ComponentCollection.IndexArray:
		weights[i] = weight

	weightmap.Elements.Array = weights


# ----------------------------------------------------
# Symmetrize Weights
# ----------------------------------------------------
# Side = O : LeftToRight
# Side = 1 : RightToLeft
# Side = 2 : BothSide (Mirror)
# ----------------------------------------------------
def SymmetrizeWeights(weightmap, side=0, invert=False, axis=0):
	if not weightmap.Type == "wtmap":
		XSI.LogMessage("[Symmetrize Weights] ERROR :  Input NOT a Weight Map", constants.siError)
		return
		
	# get symmetry map
	obj = weightmap.Parent3DObject
	symap = GetSymmetryMap(obj)
	
	geom = obj.ActivePrimitive.Geometry
	pos = geom.Points.PositionArray
	
	cls = weightmap.Parent
	elems = cls.Elements.Array
	
	factor = 1
	if invert:
		factor = -1
	
	# Get Arrays
	weights = [w for w in weightmap.Elements.Array[0]]
	tmp = [w for w in weightmap.Elements.Array[0]]
	sym = symap.Elements.Array
	
	# Process
	for i in range(len(weights)):
		if side == 0:
			if pos[axis][elems[i]] > 0:
				idx = int(sym[0][elems[i]])
				idx = cls.FindIndex(idx)
				tmp[idx] = weights[i]*factor
				
		elif side == 1:
			if pos[axis][elems[i]] < 0:
				idx = int(sym[0][elems[i]])
				idx = cls.FindIndex(idx)
				tmp[idx] = weights[i]*factor
						
		else:
			tmp[int(sym[0][elems[i]])] = weights[i]*factor

	weightmap.Elements.Array = tmp


# ----------------------------------------------------
# Mirror Copy Weights
# ----------------------------------------------------
def MirrorCopyWeights(sourcemap, targetmap):
	# get symmetry map
	symap = GetSymmetryMap(sourcemap.Parent3DObject)
	
	# Get Arrays
	weights = [w for w in sourcemap.Elements.Array[0]]
	sym = symap.Elements.Array
	tmp = [0 for w in weights]
	
	# Process
	for i in range(len(weights)):
		tmp[int(sym[0][i])] = weights[i]

	targetmap.Elements.Array = tmp


# ----------------------------------------------------
# Get Color Map
# ----------------------------------------------------
def GetColorMap(inObject, name="ColorMap", R=1, G=1, B=1):
	polymsh = inObject.ActivePrimitive.Geometry
	
	# check if colormap already exists
	for col in polymsh.VertexColors:
		if col.Name == name:
			return col
	
	# create and set color map if not exists
	col = polymsh.AddVertexColor(name)
	polymsh.CurrentVertexColor = col
	color_tupple = col.Elements.Array

	colors = []
	for i in range(len(color_tupple[0])):
		colors.append(R)
		colors.append(G)
		colors.append(B)
		colors.append(1)
		
	col.Elements.Array = colors
	return col 


# ----------------------------------------------------
# Get Texture Map
# ----------------------------------------------------
def GetTextureMap(obj, name="TextureMap"):
	props = obj.LocalProperties
	for p in props:
		if p.Name == name and p.Type == "TextureProp":
			return p
	
	p = XSI.Create2DMap(obj, name, "siDefaultPropagation")(0)
	return p


# ----------------------------------------------------
# Copy Mesh
# ----------------------------------------------------
def GetMeshCopy(obj, transform=False):
	model = obj.model
	mesh = model.AddGeometry("Cube", "MeshSurface", obj.Name+"_Copy")
	XSI.FreezeObj(mesh)
	
	datas = obj.ActivePrimitive.Geometry.Get2()
	tra = obj.Kinematics.Global.Transform
	
	# get the datas
	vertices = datas[0]
	polys = datas[1]
	
	mesh.ActivePrimitive.Geometry.Set(vertices, polys)
	
	if transform:
		mesh.Kinematics.Global.Transform = tra
	
	return mesh


def GetMeshDuplicate(mesh):
	parent = mesh.Parent3DObject
	dup = parent.AddPrimitive("EmptyPolygonMesh")
	datas = mesh.ActivePrimitive.Geometry.Get2()
	dup.ActivePrimitive.Geometry.Set(datas[0], datas[1])

	return dup


# ----------------------------------------------------
# Subdivide Mesh
# ----------------------------------------------------
def GetSubdividedMesh(mesh):
	op = XSI.ApplyOp("MeshSubdivide", mesh)
	subd = op(0).Parent3DObject
	subd.Name = mesh.name+"_Subdivided"
	return subd


# ----------------------------------------------------
# Geometry Match
# ----------------------------------------------------
def EpsilonEqual(a, b, precision):
	diff = a-b
	if diff > precision:
		return False
	return True


def CheckGeometryMatch(a, b, precision):
	ag = a.ActivePrimitive.Geometry
	bg = b.ActivePrimitive.Geometry
	
	ap = ag.Points
	bp = bg.Points
	
	if not ap.Count == bp.Count:
		XSI.LogMessage("Geometries (" + a.FullName + "," + b.FullName + ") DOESN'T match : they don't have the same number of points")
		return False
		
	apa = ap.PositionArray
	bpa = bp.PositionArray
	
	for i in range(len(apa[0])/3):
		ax = apa[0][i*3]
		ay = apa[0][i*3+1]
		az = apa[0][i*3+2]
		
		bx = bpa[0][i*3]
		by = bpa[0][i*3+1]
		bz = bpa[0][i*3+2]
		
		if not EpsilonEqual(ax, bx, precision) or not EpsilonEqual(ay, by, precision) or not EpsilonEqual(az, bz, precision):
			XSI.LogMessage("Geometries (" + a.FullName + "," + b.FullName + ") DOESN'T match : Points are not at the same location")
			return False
			
	return True


# ----------------------------------------------------
# Get Object From Expression
# -------------------------------------------
def GetObjectFromExpression(param):
	if param:
		all_sources = param.Sources
		for source in all_sources:
			if source and source.Type == "Expression":
				expr = source.Parameters("Definition").Value
				target = XSI.Dictionary.GetObject(expr)
				obj = target.Parent3DObject
				return obj

	return None


# ----------------------------------------------------
# Time
# ----------------------------------------------------
def GetStartFrame():
	control = XSI.Dictionary.GetObject("PlayControl")
	return control.In.Value


def GetEndFrame():
	control = XSI.Dictionary.GetObject("PlayControl")
	return control.Out.Value


def GetCurrentFrame():
	control = XSI.Dictionary.GetObject("PlayControl")
	return control.Current.Value


def SetStartFrame(frame):
	control = XSI.Dictionary.GetObject("PlayControl")
	control.In.Value = frame


def SetEndFrame(frame):
	control = XSI.Dictionary.GetObject("PlayControl")
	control.Out.Value = frame


def SetCurrentFrame(frame):
	control = XSI.Dictionary.GetObject("PlayControl")
	control.Current.Value = frame


# ----------------------------------------------------
# Delta
# ----------------------------------------------------
def DeactivateDelta(model):
	try:
		delta = XSI.Dictionary.GetObject(model.FullName + ".Delta")
		previous = delta.Parameters("persist_modifications").Value
		mask = previous - 1

		delta.Parameters("persist_modifications").Value = mask
		return previous
	except RuntimeError:
		return -1


def ReactivateDelta(model, previous):
	try:
		delta = XSI.Dictionary.GetObject(model.FullName + ".Delta")
		delta.Parameters("persist_modifications").Value = previous
	except RuntimeError:
		pass


def SetScriptingLogState(state):
	previous = XSI.Preferences.GetPreferenceValue("scripting.cmdlog")
	XSI.Preferences.SetPreferenceValue("scripting.cmdlog", state)
	return previous
