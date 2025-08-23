import math
from win32com.client import constants
import Constants
from Globals import XSI
from Globals import XSIMath
from Globals import XSIFactory


def GetPluginPath(name):
	""" Get plugin path given plugin name
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
	if side == Constants.LEFT:
		prefix = 'L_'
	elif side == Constants.RIGHT:
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


def MirrorVertexCluster(cluster, weight_map=False, name="WeightMap"):
	parent_object = cluster.Parent3DObject
	geometry = parent_object.ActivePrimitive.Geometry
	
	symmetry_weight_map = GetSymmetryMap(parent_object)
	symmetry_elements = symmetry_weight_map.Elements.Array
	
	cluster_name = cluster.Name
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
		
	if self_symmetry:
		to_add = []
		position = geometry.Points.PositionArray
		for i in range(len(position[0])):
			if not position[0][i] <= 0:
				continue
			symmetry_point = int(symmetry_elements[0][i])
			
			if cluster.Elements.FindIndex(symmetry_point) > -1:
				to_add.append(i)
			
		sub_comp = geometry.CreateSubComponent(constants.siVertexCluster, to_add)
		UpdateClusterComponent(cluster, sub_comp, 1)
		if weight_map:
			weight_map = GetWeightMap(parent_object, name, 1.0, 0.0, 1.0, cluster)
			SymmetrizeWeights(weight_map, 0, False)

	else:
		elements = cluster.Elements.Array
		tmp = [0 for _ in elements]
		for i in range(len(elements)):  
			tmp[i] = symmetry_elements[0][elements[i]]
		
		sub_comp = geometry.CreateSubComponent(constants.siVertexCluster, tmp)
		UpdateClusterComponent(symmetry_cluster, sub_comp, 0)
		if weight_map:
			weight_map = GetWeightMap(parent_object, name, 1.0, 0.0, 1.0, cluster)
			symmetry_weight_map = GetWeightMap(parent_object, name, 1.0, 0.0, 1.0, symmetry_cluster)
			if weight_map and symmetry_weight_map:
				symmetry_weight_map.Elements.Array = weight_map.Elements.Array
			else:
				SetWeightMap(symmetry_weight_map, 1.0, 0.0, 1.0)


def ResetStaticKinematicState(objs):
	out_kinematic_state = []
	for obj in objs:
		static_kine_state = obj.Properties('Static_KineState')
		if not static_kine_state:
			static_kine_state = obj.AddProperty('Static Kinematic State Property')
			static_kine_state.Name = 'Static_KineState'
		
		kine = obj.Kinematics.Global
		static_kine_state.Parameters('PosX').Value = kine.Parameters('PosX').Value
		static_kine_state.Parameters('PosY').Value = kine.Parameters('PosY').Value
		static_kine_state.Parameters('PosZ').Value = kine.Parameters('PosZ').Value
		static_kine_state.Parameters('OriX').Value = kine.Parameters('RotX').Value
		static_kine_state.Parameters('OriY').Value = kine.Parameters('RotY').Value
		static_kine_state.Parameters('OriZ').Value = kine.Parameters('RotZ').Value
		static_kine_state.Parameters('SclX').Value = kine.Parameters('SclX').Value
		static_kine_state.Parameters('SclY').Value = kine.Parameters('SclY').Value
		static_kine_state.Parameters('SclZ').Value = kine.Parameters('SclZ').Value
		out_kinematic_state.append(static_kine_state)
		
	return out_kinematic_state


def SetWireColor(obj, red, green, blue):
	p = obj.AddProperty("Display Property")
	p.Parameters("WireColorR").Value = red
	p.Parameters("WireColorG").Value = green
	p.Parameters("WireColorB").Value = blue


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


def MatchTransform(obj, target):
	transform = target.Kinematics.Global.Transform
	obj.Kinematics.Global.Transform = transform


def MatchTransformWithOffset(obj, target, offset):
	transform = target.Kinematics.Global.Transform

	offset_matrix = XSIMath.CreateMatrix4()
	offset_transform = XSIMath.CreateTransform()
	offset_matrix.Set(*offset)
	offset_matrix.MulInPlace(transform.Matrix4)
	offset_transform.SetMatrix4(offset_matrix)
	obj.Kinematics.Global.Transform = offset_transform


def MatchRotation(obj, target):
	transform = target.Kinematics.Global.Transform
	other = obj.Kinematics.Global.Transform
	rotation = XSIMath.CreateRotation()
	transform.GetRotation(rotation)
	other.SetRotation(rotation)
	obj.Kinematics.Global.Transform = other


def MatchPosition(obj, target):
	transform = target.Kinematics.Global.Transform
	other = obj.Kinematics.Global.Transform
	other.SetTranslation(transform.Translation)
	obj.Kinematics.Global.Transform = other


def SetPosition(obj, pos):
	transform = obj.Kinematics.Global.GetTransform2(obj)
	transform.SetTranslation(pos)
	obj.Kinematics.Global.PutTransform2(obj, transform)


def SetOrientation(obj, rot):
	transform = obj.Kinematics.Global.GetTransform2(obj)
	transform.SetRotation(rot)
	obj.Kinematics.Global.PutTransform2(obj, transform)


def SetScaling(obj, scl):
	transform = obj.Kinematics.Global.GetTransform2(obj)
	transform.SetScaling(scl)
	obj.Kinematics.Global.PutTransform2(obj, transform)


def SimpleSymmetry(obj, target, axis):
	obj_trs = obj.Kinematics.Global
	tgt_trs = target.Kinematics.Global
	
	if axis == Constants.XY_AXIS:
		x = "- "+str(tgt_trs.Parameters("posx"))
		obj_trs.Parameters("posx").AddExpression(x)
		y = str(tgt_trs.Parameters("posy"))
		obj_trs.Parameters("posy").AddExpression(y)
		z = str(tgt_trs.Parameters("posz"))
		obj_trs.Parameters("posz").AddExpression(z)
		
		x = str(tgt_trs.Parameters("rotx"))
		obj_trs.Parameters("rotx").AddExpression(x)
		y = "- "+str(tgt_trs.Parameters("roty"))
		obj_trs.Parameters("roty").AddExpression(y)
		z = "- "+str(tgt_trs.Parameters("rotz"))
		obj_trs.Parameters("rotz").AddExpression(z)
		
		x = str(tgt_trs.Parameters("sclx"))
		obj_trs.Parameters("sclx").AddExpression(x)
		y = str(tgt_trs.Parameters("scly"))
		obj_trs.Parameters("scly").AddExpression(y)
		z = str(tgt_trs.Parameters("sclz"))
		obj_trs.Parameters("sclz").AddExpression(z)
		
	elif axis == Constants.YZ_AXIS:
		x = str(tgt_trs.Parameters("posx"))
		obj_trs.Parameters("posx").AddExpression(x)
		y = str(tgt_trs.Parameters("posy"))
		obj_trs.Parameters("posy").AddExpression(y)
		z = "- "+str(tgt_trs.Parameters("posz"))
		obj_trs.Parameters("posz").AddExpression(z)
		
		x = "- "+str(tgt_trs.Parameters("rotx"))
		obj_trs.Parameters("rotx").AddExpression(x)
		y = "- "+str(tgt_trs.Parameters("roty"))
		obj_trs.Parameters("roty").AddExpression(y)
		z = str(tgt_trs.Parameters("rotz"))
		obj_trs.Parameters("rotz").AddExpression(z)
		
		x = str(tgt_trs.Parameters("sclx"))
		obj_trs.Parameters("sclx").AddExpression(x)
		y = str(tgt_trs.Parameters("scly"))
		obj_trs.Parameters("scly").AddExpression(y)
		z = str(tgt_trs.Parameters("sclz"))
		obj_trs.Parameters("sclz").AddExpression(z)

	elif axis == Constants.XZ_AXIS:
		x = str(tgt_trs.Parameters("posx"))
		obj_trs.Parameters("posx").AddExpression(x)
		y = "- "+str(tgt_trs.Parameters("posy"))
		obj_trs.Parameters("posy").AddExpression(y)
		z = str(tgt_trs.Parameters("posz"))
		obj_trs.Parameters("posz").AddExpression(z)
		
		x = "- "+str(tgt_trs.Parameters("rotx"))
		obj_trs.Parameters("rotx").AddExpression(x)
		y = "- "+str(tgt_trs.Parameters("roty"))
		obj_trs.Parameters("roty").AddExpression(y)
		z = str(tgt_trs.Parameters("rotz"))
		obj_trs.Parameters("rotz").AddExpression(z)
		
		x = str(tgt_trs.Parameters("sclx"))
		obj_trs.Parameters("sclx").AddExpression(x)
		y = str(tgt_trs.Parameters("scly"))
		obj_trs.Parameters("scly").AddExpression(y)
		z = str(tgt_trs.Parameters("sclz"))
		obj_trs.Parameters("sclz").AddExpression(z)


def MatchSymmetry(obj, target, axis):
	obj_trs = obj.Kinematics.Global
	tgt_trs = target.Kinematics.Global
	
	if axis == Constants.XY_AXIS:
		x = - tgt_trs.Parameters("posx").Value
		obj_trs.Parameters("posx").Value = x
		y = tgt_trs.Parameters("posy").Value
		obj_trs.Parameters("posy").Value = y
		z = tgt_trs.Parameters("posz").Value
		obj_trs.Parameters("posz").Value = z
		
		x = tgt_trs.Parameters("rotx").Value
		obj_trs.Parameters("rotx").Value = x
		y = -tgt_trs.Parameters("roty").Value
		obj_trs.Parameters("roty").Value = y
		z = -tgt_trs.Parameters("rotz").Value
		obj_trs.Parameters("rotz").Value = z
		
		x = tgt_trs.Parameters("sclx").Value
		obj_trs.Parameters("sclx").Value = x
		y = tgt_trs.Parameters("scly").Value
		obj_trs.Parameters("scly").Value = y
		z = tgt_trs.Parameters("sclz").Value
		obj_trs.Parameters("sclz").Value = z
		
	elif axis == Constants.YZ_AXIS:
		x = tgt_trs.Parameters("posx").Value
		obj_trs.Parameters("posx").Value = x
		y = tgt_trs.Parameters("posy").Value
		obj_trs.Parameters("posy").Value = y
		z = -tgt_trs.Parameters("posz").Value
		obj_trs.Parameters("posz").Value = z
		
		x = -tgt_trs.Parameters("rotx").Value
		obj_trs.Parameters("rotx").Value = x
		y = -tgt_trs.Parameters("roty").Value
		obj_trs.Parameters("roty").Value = y
		z = tgt_trs.Parameters("rotz").Value
		obj_trs.Parameters("rotz").Value = z
		
		x = tgt_trs.Parameters("sclx").Value
		obj_trs.Parameters("sclx").Value = x
		y = tgt_trs.Parameters("scly").Value
		obj_trs.Parameters("scly").Value = y
		z = tgt_trs.Parameters("sclz").Value
		obj_trs.Parameters("sclz").Value = z
	elif axis == Constants.XZ_AXIS:
		x = tgt_trs.Parameters("posx").Value
		obj_trs.Parameters("posx").Value = x
		y = - tgt_trs.Parameters("posy").Value
		obj_trs.Parameters("posy").Value = y
		z = tgt_trs.Parameters("posz").Value
		obj_trs.Parameters("posz").Value = z
		
		x = -tgt_trs.Parameters("rotx").Value
		obj_trs.Parameters("rotx").Value = x
		y = - tgt_trs.Parameters("roty").Value
		obj_trs.Parameters("roty").Value = y
		z = tgt_trs.Parameters("rotz").Value
		obj_trs.Parameters("rotz").Value = z
		
		x = tgt_trs.Parameters("sclx").Value
		obj_trs.Parameters("sclx").Value = x
		y = tgt_trs.Parameters("scly").Value
		obj_trs.Parameters("scly").Value = y
		z = tgt_trs.Parameters("sclz").Value
		obj_trs.Parameters("sclz").Value = z


def GetPositions(objects):
	positions = []
	for o in objects:
		t = o.Kinematics.Global.Transform
		p = XSIMath.CreateVector3()
		t.GetTranslation(p)
		positions.append(p)
		
	return positions


def GetAveragePosition(positions):
	x = XSIMath.CreateVector3()
	for p in positions:
		x.AddInPlace(p)
	
	x.ScaleInPlace(1.0 / len(positions))
	return x


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
			n = AddNull(XSI.ActiveSceneRoot,4,1,"Start")
			t = n.Kinematics.Global.Transform
			t.SetTranslation(start)
			n.Kinematics.Global.Transform = t
			
			n = AddNull(XSI.ActiveSceneRoot,4,1,"End")
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


def ProjectOnPlane(point, normal):
	vProjection = XSIMath.CreateVector3()
	pProjection = XSIMath.CreateVector3()

	dot = point.Dot(normal)
	scaleChange = (dot / math.pow(normal.Length(), 2))
	vProjection.Scale(scaleChange, normal)
	pProjection.Sub(point, vProjection)
	return pProjection


def SelectionToCollection(filtering=None):
	result = XSIFactory.CreateActiveXObject("XSI.Collection")
	selection = XSI.Selection
	for selected in selection:
		if filtering:
			if selected.Type == filtering:
				result.Add(selected)
		else:
			result.Add(selected)
	return result


def DistanceBetweenTwoObjects(start, end):
	if not start or not end:
		XSI.LogMessage("[Distance between two objects] Invalid Inputs!!", constants.siWarning)
	sT = start.Kinematics.Global.Transform
	eT = end.Kinematics.Global.Transform
	delta = XSIMath.CreateVector3()
	delta.Sub(eT.Translation, sT.Translation)
	return delta.Length()


def SetVisibility(objects, view_vis=True, render_vis=True, selectable=True):
	for o in objects:
		if o.IsClassOf(constants.siX3DObjectID):
			v = o.Properties("Visibility")
			v.Parameters("viewvis").Value = view_vis
			v.Parameters("rendvis").Value = render_vis
			v.Parameters("selectability").Value = selectable


def MakeRigNull(parent=None, null_type=0, name="Null"):
	if not parent:
		parent = XSI.ActiveSceneRoot
	n = parent.AddNull(name)
	MatchTransform(n, parent)
	
	# buffer null
	if null_type == 0:
		n.ActivePrimitive.Size = .1
		n.ActivePrimitive.Primary_Icon = 0
		
	# control null
	elif null_type == 1:
		n.ActivePrimitive.Size = .1
		n.ActivePrimitive.Primary_Icon = 2
		SetWireColor(n, 1, 0, 0)
	
	# deformer null
	elif null_type == 2:
		n.ActivePrimitive.Size = .1
		n.ActivePrimitive.Primary_Icon = 4
		SetWireColor(n, 0.7, 1, 0.7)
	return n


def AddNull(parent=None, null_type=0, null_size=1, name="Null", red_col=0, green_col=0, blue_col=0):
	if not parent:
		parent = XSI.ActiveSceneRoot
		
	n = parent.AddNull(name)
	MatchTransform(n, parent)
	
	n.ActivePrimitive.Size = null_size
	n.ActivePrimitive.Primary_Icon = null_type
		
	SetWireColor(n, red_col, green_col, blue_col)
	return n


def GroupSetup(model, objects=None, group_name="Group"):
	group = model.Groups(group_name)
	if not group:
		group = model.AddGroup()
		group.Name = group_name
	if objects:
		for o in objects:
			group.AddMember(o)
	return group


def ObjectInGroup(obj, group):
	for m in group.Members:
		if m.FullName == obj.FullName:
			return True
	return False


def PickElement(pick_filter=constants.siGenericObjectFilter, message="Pick Element"):
	rtn = XSI.PickElement(pick_filter, message, message)
	element = rtn.Value("PickedElement")
	button = rtn.Value("ButtonPressed")

	if button:
		return element
	
	return None


def PickMultiElement(pick_type=None, message="Pick Multi Element"):
	outObj = []
	check = 0

	while not check:
		pickElement = PickElement(pick_type, message)
		if pickElement:
			outObj.append(pickElement)
		else:
			check = 1
		
	return outObj


def PickPosition(message="Pick Position"):
	if not message:
		message = "Pick Position"
	picked = XSI.PickPosition(message, message, None, None, None, None)
	button = picked("ButtonPressed")
	if button:
		x = picked("PosX")
		y = picked("PosY")
		z = picked("PosZ")
		outVec = XSIMath.CreateVector3(x, y, z)
		return outVec
	else:
		return None


def PickPositionInVolume(message ="Pick Position"):
	XSI.SetValue("preferences.SnapProperties.Enable2D", True, None)
	XSI.SetValue("preferences.SnapProperties.Enable", True, None)
	
	if not message:
		message = "Pick Position"

	picked = XSI.PickPosition(message, message, None, None, None, None)
	
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


def PickMultiPosition(number=-1, bounding_volume=False):
	outPos = []
	check = 0
	while not check:
		number -= 1
		if bounding_volume:
			pickPosition = PickPositionInVolume("Pick Position")
		else:
			pickPosition = PickPosition("Pick Position")
		if pickPosition:
			outPos.append(pickPosition)
		else:
			check = 1
			
		if number == 0:
			check = True
			
	return outPos


def ClusterCenterTarget(pnt_sub_component=None, target=None, name="PntCls"):
	if not pnt_sub_component:
		sel = XSI.Selection(0)
		if not sel or not sel.Type == "pntSubComponent":
			XSI.LogMessage("[ClusterCenterTarget] Invalid Selection : Need some points selected ---> Aborted!")
			return
		else:
			pnt_sub_component = sel.SubComponent
	if not target:
		pick = PickElement(constants.siObjectFilter, "Pick Target Object")
		if not pick:
			XSI.LogMessage("[ClusterCenterTarget] Invalid Target : Need one target object ---> Aborted!")
			return
		else:
			target = pick
	obj = pnt_sub_component.Parent3DObject
	geo = obj.ActivePrimitive.Geometry
	elem = pnt_sub_component.ElementArray
	cls = geo.AddCluster(constants.siVertexCluster, name, elem)
	XSI.ApplyOp("ClusterCenter", str(cls) + ";" + str(target), 0, 0, None, 2)


def BuildCurveOnObjectsPosition(objects, degree=0, close=0, constraint=1, offset=XSIMath.CreateVector3()):
	if len(objects) < 2:
		XSI.LogMessage("BuildCurveOnPoints aborted :: Not enough input objects!!", constants.siError)
		return
		
	if len(objects) < 4:
		degree = 0
		
	pPos = GetPositions(objects)

	aPos = []
	for p in pPos:
		aPos.append(p.X + offset.X)
		aPos.append(p.Y + offset.Y)
		aPos.append(p.Z + offset.Z)
		aPos.append(1)

	if degree == 0:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(aPos, None, close, 1)
		
	else:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(aPos, None, close, 3)
	
	if constraint == 1:
		crvGeom = outCrv.ActivePrimitive.Geometry
		for p in crvGeom.Points:
			oCluster = crvGeom.AddCluster(constants.siVertexCluster, "Pnt"+str(p.Index+1), p.Index)
			XSI.ApplyOp("ClusterCenter", str(oCluster) + ";" + str(objects[p.Index]), 0, 0, None, 2)

	return outCrv


def BuildCurveOnPositions(positions, degree=0, close=0, offset=XSIMath.CreateVector3()):
	if len(positions) < 2:
		XSI.LogMessage("BuildCurveOnPosition aborted :: Not enough input positions!!", constants.siError)
		return
		
	if len(positions) < 4:
		degree = 0
		
	aPos = []
	for p in positions:
		aPos.append(p.X + offset.X)
		aPos.append(p.Y + offset.Y)
		aPos.append(p.Z + offset.Z)
		aPos.append(1)
	
	if degree == 0:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(aPos, None, close, 1)
	else:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(aPos, None, close, 3)

	return outCrv


def BuildCurveOnSymmetrizedPositions(positions, axis=0, degree=0, close=0):

	if len(positions) < 1:
		XSI.LogMessage("BuildCurveOnSymmetrizedPosition aborted :: Not enough input positions!!", constants.siError)
		return
	axis = XSIMath.CreateVector3(-1, 1, 1)
	if axis == 1:
		axis.Set(1, -1, 1)
	elif axis == 2:
		axis.Set(1, 1, -1)
	newPos = []
	last = len(positions) - 1
	for i in range(len(positions)):
		p = positions[last - i]
		newPos.append(p.X*axis.X)
		newPos.append(p.Y*axis.Y)
		newPos.append(p.Z*axis.Z)
		newPos.append(1)

	newPos.append(0)
	newPos.append(positions[0].Y)
	newPos.append(positions[0].Z)
	newPos.append(1)
	
	for p in positions:
		newPos.append(p.X)
		newPos.append(p.Y)
		newPos.append(p.Z)
		newPos.append(1)
		
	if degree == 0:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(newPos, None, close, 1)
		
	else:
		outCrv = XSI.ActiveSceneRoot.AddNurbsCurve(newPos, None, close, 3)
		
	return outCrv


def ReplaceCurveGeometry(dst, src):
	if src and src.Type == "crvlist" and dst and dst.Type == "crvlist":
		datas = src.ActivePrimitive.Geometry.Get2()
		XSI.FreezeObj(dst)
		dst.ActivePrimitive.Geometry.Set(datas[0], datas[1], datas[2], datas[3], datas[4], datas[5], datas[6], datas[7])


def GetCurveLength(crv):	
	if crv and crv.Type == "crvlist":
		return crv.ActivePrimitive.Geometry.Length


def GetSymmetryMap(target):
	symmetry_maps = XSI.FindObjects(None, "{2EBA6DE4-B7EA-4877-80FE-FC768F32729F}")

	for symmetry_map in symmetry_maps:
		if symmetry_map.Parent3DObject.IsEqualTo(target):
			return symmetry_map
				
	# create cluster if not exist
	cluster = target.ActivePrimitive.Geometry.Clusters("SymmetryMapCls")
	if not cluster:
		cluster = target.ActivePrimitive.Geometry.AddCluster(constants.siVertexCluster, "SymmetryMapCls")

	symmetry_map = XSI.CreateSymmetryMap("SymmetryMap", cluster, "SymmetryMap")(0)
	return symmetry_map


def GetWeightMap(target, name="WeightMap", value=0, minimum=0, maximum=1, cls=None):
	cluster = None

	if cls:
		cluster = cls
		for w in cls.LocalProperties.Filter("wtmap"):
			if w.Name == name:
				return w
	
	else:
		XSI.LogMessage(target)
		if target.ActivePrimitive.Geometry.Clusters:
			# check all cluster for weightmap already exists
			for cls in target.ActivePrimitive.Geometry.Clusters.Filter("pnt"):
				if cls.Name == "WeightMapCls":
					cluster = cls
				for w in cls.LocalProperties.Filter("wtmap"):
					if w.Name == name:
						return w
	
	# create cluster if not exist
	if not cluster:
		cluster = target.ActivePrimitive.Geometry.AddCluster(constants.siVertexCluster, "WeightMapCls")
	
	# create and set weight map 
	wm = cluster.AddProperty("Weight Map Property", False, name)

	# clamp between -1 and 1  (maximum possible values)
	if minimum < -1:
		minimum = -1
	elif minimum > 0:
		minimum = 0
	if maximum > 1:
		maximum = 1
	elif maximum < 0:
		maximum = 0

	wm.Parameters("wmin").Value = minimum
	wm.Parameters("wmax").Value = maximum

	elems_tuple = wm.Elements.Array
	elems = [value for _ in range(len(elems_tuple[0])) for _ in range(len(elems_tuple))]
	wm.Elements.Array = elems
	
	XSI.FreezeObj(wm)
	
	return wm


def CopyWeightMap(src, dst, name):
	# first check geometry input
	src_n = src.ActivePrimitive.Geometry.Points.Count
	dst_n = dst.ActivePrimitive.Geometry.Points.Count
	
	if not src_n == dst_n:
		XSI.LogMessage(
			"[Utils.py] Copy Weight Map Aborted!!!(Objects doesn't have same number of points)",
			constants.siWarning)
		return False
	
	swm = GetWeightMap(src, name)
	dwm = GetWeightMap(dst, name)
	
	if swm and dwm:
		XSI.LogMessage(swm)
		XSI.LogMessage(dwm)
		XSI.LogMessage(dwm.Elements.Array)
		dwm.Elements.Array = swm.Elements.Array
		XSI.LogMessage(dwm.Elements.Array)
	

def SetWeightMap(weight_map, value=1.0, minimum=0.0, maximum=1.0):
	# create and set weight map 
	weight_map.Parameters("wmin").Value = minimum
	weight_map.Parameters("wmax").Value = maximum
	elements_tuple = weight_map.Elements.Array
	elements = [value for _ in range(len(elements_tuple[0])) for _ in range(len(elements_tuple))]
	weight_map.Elements.Array = elements
	
	XSI.FreezeObj(weight_map)
	
	return weight_map


def SetWeightOnSelectedPoints(weight_map, weight=0, pnt_sub_component=None):
	if not weight_map or not weight_map.Type == "wtmap":
		XSI.LogMessage("[SetWeightOnSelectedPoints] Invalid Weight Map ---> Aborted!")
		return
			
	if not pnt_sub_component:
		selection = XSI.Selection(0)
		if not selection or not selection.Type == "pntSubComponent":
			XSI.LogMessage("[SetWeightOnSelectedPoints] Invalid Selection : Need some points selected ---> Aborted!")
			return
		else:
			pnt_sub_component = selection.SubComponent
		
	weights = [w for w in weight_map.Elements.Array[0]]

	# process
	for i in pnt_sub_component.ComponentCollection.IndexArray:
		weights[i] = weight

	weight_map.Elements.Array = weights


# ----------------------------------------------------
# Symmetrize Weights
# ----------------------------------------------------
# Side = O : LeftToRight
# Side = 1 : RightToLeft
# Side = 2 : BothSide (Mirror)
# ----------------------------------------------------
def SymmetrizeWeights(weight_map, side=0, invert=False, axis=0):
	if not weight_map.Type == "wtmap":
		XSI.LogMessage("[Symmetrize Weights] ERROR :  Input NOT a Weight Map", constants.siError)
		return
		
	# get symmetry map
	obj = weight_map.Parent3DObject
	sym_map = GetSymmetryMap(obj)
	
	geom = obj.ActivePrimitive.Geometry
	pos = geom.Points.PositionArray
	
	cluster = weight_map.Parent
	elements = cluster.Elements.Array
	
	factor = 1
	if invert:
		factor = -1
	
	# Get Arrays
	weights = [w for w in weight_map.Elements.Array[0]]
	tmp = [w for w in weight_map.Elements.Array[0]]
	sym = sym_map.Elements.Array
	
	# Process
	for i in range(len(weights)):
		if side == 0:
			if pos[axis][elements[i]] > 0:
				idx = int(sym[0][elements[i]])
				idx = cluster.FindIndex(idx)
				tmp[idx] = weights[i]*factor
				
		elif side == 1:
			if pos[axis][elements[i]] < 0:
				idx = int(sym[0][elements[i]])
				idx = cluster.FindIndex(idx)
				tmp[idx] = weights[i]*factor
						
		else:
			tmp[int(sym[0][elements[i]])] = weights[i]*factor

	weight_map.Elements.Array = tmp


# ----------------------------------------------------
# Mirror Copy Weights
# ----------------------------------------------------
def MirrorCopyWeights(source_map, target_map):
	# get symmetry map
	sym_map = GetSymmetryMap(source_map.Parent3DObject)
	
	# Get Arrays
	weights = [w for w in source_map.Elements.Array[0]]
	sym_elements = sym_map.Elements.Array
	tmp = [0 for _ in weights]
	
	# Process
	for i in range(len(weights)):
		tmp[int(sym_elements[0][i])] = weights[i]

	target_map.Elements.Array = tmp


# ----------------------------------------------------
# Get Color Map
# ----------------------------------------------------
def GetColorMap(obj, name="ColorMap", red_c=1, green_c=1, blue_c=1):
	polymesh = obj.ActivePrimitive.Geometry
	
	# check if colormap already exists
	for vertex_color_prop in polymesh.VertexColors:
		if vertex_color_prop.Name == name:
			return vertex_color_prop
	
	# create and set color map if not exists
	vertex_color_prop = polymesh.AddVertexColor(name)
	polymesh.CurrentVertexColor = vertex_color_prop
	vertex_colors = vertex_color_prop.Elements.Array

	colors = []
	for i in range(len(vertex_colors[0])):
		colors.append(red_c)
		colors.append(green_c)
		colors.append(blue_c)
		colors.append(1)
		
	vertex_color_prop.Elements.Array = colors
	return vertex_color_prop


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
