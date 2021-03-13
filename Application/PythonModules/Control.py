# -------------------------------------------------------------------
# Control
# -------------------------------------------------------------------
from win32com.client import constants
from Globals import XSI
from Globals import XSIMath
from Globals import XSIFactory
from Constants import *
import Element
import Utils
import ICETree
import Icon


class IRControl(Element.IRElement):
	def __init__(self, crv, parent, name, icon=0, mode=CTRL_FK, side=MIDDLE, suffix='_XX'):
		super(IRControl, self).__init__(crv, parent, name, side, False, suffix)

		self.type = ID_CONTROL
		self.mode = mode
		self.list = 'ControlList'
		self.chooser = 'ControlChooser'
		self.size = 1.0
		self.icon = RIG_CONTROL_ICON[icon]
		if self.crv:
			self.prop = self.crv.Properties(PROP_CONTROL)

		self.root = self.model.FindChild('Controls', constants.siNullPrimType)

	def PickPosition(self):
		self.position = Utils.PickPosition()
		self.rotation = Utils.GetOrientationFromPickPosition(self.position)
		return True

	def CreateGuide(self):
		builder = self.GetBuilder()
		if self.CheckElementExist():
			XSI.LogMessage('[CreateControlElement] : ' + self.fullname + '_Ctrl already exist ---> aborted...!')
			return False

		t = XSIMath.CreateTransform()
		t.SetTranslation(self.position)

		self.crv = RIG_CONTROL_ICON[builder.Parameters("ControlIconType").Value]
		self.GetControlColor()
		color = [self.colorrik, self.colorgik, self.colorbik]
		self.size = builder.Parameters("GlobalSize").Value
		self.crv = Icon.IRIcon(self.parent, self.fullname+"_Ctrl", t, color, self.icon, self.size, self.size, self.size)

		Utils.GroupSetup(self.model, [self.crv], "Control_Curves")
		return True

	def GetFromGuide(self, crv):
		self.crv = crv
		self.position = crv.Kinematics.Global.Transform.Translation
		self.fullname = crv.name.replace("_Ctrl", "")
		if self.fullname[:2] == "L_":
			self.side = LEFT
		elif self.fullname[:2] == "R_":
			self.side = RIGHT
		else:
			self.side = MIDDLE
			
		if self.fullname.find("IK") > -1:
			self.type = CTRL_IK
		elif self.fullname.find("FK") > -1:
			self.type = CTRL_FK
		else:
			self.type = CTRL_SRT
		self.GetControlColor()

	def Symmetrize(self):
		if self.side == MIDDLE:
			XSI.LogMessage("[CreateControlElement] : Can't symmetrize middle elements ---> aborted...!")
			return None
		
		axis = self.builder.Parameters("SymmetryAxis").Value
		
		self.symparent = self.model.FindChild(self.ctrl.Parent.Name.replace(self.prefix, self.symprefix))
		if not self.symparent:
			self.symparent = self.ctrl.Parent
			
		color = [self.colorrik, self.colorgik, self.colorbik]
		self.symctrl = Icon.IRIcon(
			self.symparent,
			self.symfullname+"_Ctrl",
			XSIMath.CreateTransform(),
			color,
			self.icon,
			self.size,
			self.size,
			self.size
		)
		Utils.SimpleSymmetry(self.symctrl, self.ctrl, axis)
		'''
		self.symguidepoints = XSIFactory.CreateActiveXObject("XSI.Collection")
		self.parent = self.model.FindChild(self.guidepoints[0].Parent.Name.replace(self.prefix,self.symprefix))
		if not self.parent:
			self.parent = self.guidepoints[0].Parent

		points = self.AddGuidePoints()
		
		# simple symmetry constraint via expression
		lmSimpleSymmetry(self.guidepoints[nbp+id],self.guidepoints[id])
		lmSimpleSymmetry(self.guideshapes[nbp+id],self.guideshapes[id])
				
		self.symcrv = lmBuildCurveOnPoints(points,self.curvetype,0,1)
		self.guidecurves.append(self.symcrv)
		#lmSetVisibility([self.symcrv],False,False,False)
		
		self.model.AddChild(self.symcrv)
		lmSetWireColor(self.symcrv,0.525,0.8,0.8)
		self.symcrv.Name = self.symfullname+self.extension
		
		#add rig element property
		self.symprop = self.symcrv.AddProperty("lmRigElement")
		self.symprop.Parameters("ElementType").AddExpression(self.prop.Parameters("ElementType"))
		self.symprop.Parameters("ElementName").Value = self.symfullname
		self.symprop.Parameters("Divisions").AddExpression(self.prop.Parameters("Divisions"))
		self.symprop.Parameters("Side").Value = 1-self.side
		'''

	def ConnectSymmetry(self, elem):
		pass

	def AddToRig(self):
		XSI.LogMessage("[RigControl] : Add to Rig called...")

	def RemoveFromRig(self):
		XSI.LogMessage("[RigControl] : Remove Control Element Called...")

	def GetControlColor(self):
		builder = self.GetBuilder()
		if self.side == RIGHT:
			self.colorrik = builder.Parameters("R_ColorR_IK").Value
			self.colorgik = builder.Parameters("R_ColorG_IK").Value
			self.colorbik = builder.Parameters("R_ColorB_IK").Value
			
			self.colorrfk = builder.Parameters("R_ColorR_FK").Value
			self.colorgfk = builder.Parameters("R_ColorG_FK").Value
			self.colorbfk = builder.Parameters("R_ColorB_FK").Value
			
		elif self.side == LEFT:
			self.colorrik = builder.Parameters("L_ColorR_IK").Value
			self.colorgik = builder.Parameters("L_ColorG_IK").Value
			self.colorbik = builder.Parameters("L_ColorB_IK").Value
			
			self.colorrfk = builder.Parameters("L_ColorR_FK").Value
			self.colorgfk = builder.Parameters("L_ColorG_FK").Value
			self.colorbfk = builder.Parameters("L_ColorB_FK").Value
			
		else:
			self.colorrik = builder.Parameters("M_ColorR_IK").Value
			self.colorgik = builder.Parameters("M_ColorG_IK").Value
			self.colorbik = builder.Parameters("M_ColorB_IK").Value
			
			self.colorrfk = builder.Parameters("M_ColorR_FK").Value
			self.colorgfk = builder.Parameters("M_ColorG_FK").Value
			self.colorbfk = builder.Parameters("M_ColorB_FK").Value

	def SetControlColor(self):
		if self.type == CTRL_FK:
			Utils.SetWireColor(self.ctrl, self.colorrfk, self.colorgfk, self.colorbfk)
		else:
			Utils.SetWireColor(self.ctrl, self.colorrik, self.colorgik, self.colorbik)
			

# -------------------------------------------------------------------
# Get Control Color
# -------------------------------------------------------------------
def GetControlColor(side, type, builder):
	color = [0, 0, 0]
	if side == RIGHT:
		if type == CTRL_IK:
			color[0] = builder.Parameters("R_ColorR_IK").Value
			color[1] = builder.Parameters("R_ColorG_IK").Value
			color[2] = builder.Parameters("R_ColorB_IK").Value
		elif type == CTRL_FK:
			color[0] = builder.Parameters("R_ColorR_FK").Value
			color[1] = builder.Parameters("R_ColorG_FK").Value
			color[2] = builder.Parameters("R_ColorB_FK").Value
		
	elif side == LEFT:
		if type == CTRL_IK:
			color[0] = builder.Parameters("L_ColorR_IK").Value
			color[1] = builder.Parameters("L_ColorG_IK").Value
			color[2] = builder.Parameters("L_ColorB_IK").Value
		elif type == CTRL_FK:
			color[0] = builder.Parameters("L_ColorR_FK").Value
			color[1] = builder.Parameters("L_ColorG_FK").Value
			color[2] = builder.Parameters("L_ColorB_FK").Value
		
	else:
		if type == CTRL_IK:
			color[0] = builder.Parameters("M_ColorR_IK").Value
			color[1] = builder.Parameters("M_ColorG_IK").Value
			color[2] = builder.Parameters("M_ColorB_IK").Value
		elif type == CTRL_FK:
			color[0] = builder.Parameters("M_ColorR_FK").Value
			color[1] = builder.Parameters("M_ColorG_FK").Value
			color[2] = builder.Parameters("M_ColorB_FK").Value


# -------------------------------------------------------------------
# build IK 2 Bones
# -------------------------------------------------------------------
def BuildIK2BonesRig(parent, crv, ppg):
	name = crv.Name.replace("_Crv", "")
	side = MIDDLE
	if name.find("L_", 0, 2) > -1:
		side = LEFT
	elif name.find("R_", 0, 2) > -1:
		side = RIGHT
		
	prefix = "M_"
	if side == LEFT:
		prefix = "L_"
	elif side == RIGHT:
		prefix = "R_"
		
	ppg.Parameters("GlobalSize").Value

	r = ppg.Parameters(prefix+"ColorR_IK").Value
	g = ppg.Parameters(prefix+"ColorG_IK").Value
	b = ppg.Parameters(prefix+"ColorB_IK").Value
	
	pnts = crv.ActivePrimitive.Geometry.Points
	if pnts.Count == 3:
		p0 = pnts[0].Position
		p1 = pnts[1].Position
		p2 = pnts[2].Position
	else:
		last = pnts.Count-1
		p0 = pnts[0].Position
		pos = []
		for i in range(1,last):
			pos.append(pnts[i].Position)
		p1 = Utils.GetAveragePosition(pos)
		p2 = pnts[last].Position
	
	dir1 = XSIMath.CreateVector3()
	dir1.Sub(p0, p1)
	dir2 = XSIMath.CreateVector3()
	dir2.Sub(p1, p2)
	
	length = dir1.Length() + dir2.Length()
	
	plane = XSIMath.CreateVector3()
	plane.Cross(dir1, dir2)
	plane.NormalizeInPlace()
	
	offset = XSIMath.CreateVector3()
	offset.Normalize(dir1)
	offset.ScaleInPlace(-length*0.5)
	offset.AddInPlace(p1)
	
	l1 = dir1.Length()
	l2 = dir2.Length()
	
	transform = XSIMath.CreateTransform()
	transform.SetTranslation(p0)
	
	root = Icon.IRIcon(parent, name+"_Root_Ctrl", transform, [r, g, b], "cube", 0.1*length, 0.1*length, 0.1*length)
	ik = Utils.MakeRigNull(parent, 1, name+"_IK")
	
	transform.SetTranslation(p1)
	ik.Kinematics.Global.Transform = transform
	
	tree = ICETree.CreateICETree(ik, "IK", 2)
	ik_node = XSI.AddICECompoundNode("IK2BonesKinematics", tree)
	XSI.ConnectICENodes(str(tree) + ".port1", str(ik_node) + ".Execute")
	
	# set ik values
	XSI.SetValue(str(ik_node) + ".Reference", "this_model." + name + "_Root_Ctrl", "")
	XSI.SetValue(str(ik_node) + ".Reference1", "this_model." + name + "_Eff_Ctrl", "")
	XSI.SetValue(str(ik_node) + ".Reference2", "this_model." + name + "_UpV_Ctrl", "")
	XSI.SetValue(str(ik_node) + ".LengthA", l1, "")
	XSI.SetValue(str(ik_node) + ".LengthB", l2, "")

	knee = Icon.IRIcon(ik, name+"_Knee_Ctrl", transform, [r, g, b], "sphere", 0.1*length, 0.1*length, 0.1*length)
	
	transform.SetTranslation(offset)
	upv = Icon.IRIcon(root, name+"_UpV_Ctrl", transform, [r, g, b], "pyramid", 0.05*length, 0.05*length, 0.05*length)
	
	transform.SetTranslation(p2)
	eff = Icon.IRIcon(parent, name+"_Eff_Ctrl", transform, [r, g, b], "square", length, length, length)
	
	Utils.ResetStaticKinematicState([root, knee, eff])
	Utils.GroupSetup(parent.Model, [root, knee, eff], "IK_Rig")
	XSI.SetNeutralPose([root, knee, eff, upv], "siSRT", "")


# -------------------------------------------------------------------
# Build FK RiG
# -------------------------------------------------------------------
def BuildFKRig(parent, crv, ppg, side=MIDDLE, simple=False):
	prefix = 'M_'
	if side == LEFT:
		prefix = 'L_'
	elif side == RIGHT:
		prefix = 'R_'
			
	r = ppg.Parameters('{}ColorR_FK'.format(prefix)).Value
	g = ppg.Parameters('{}ColorG_FK'.format(prefix)).Value
	b = ppg.Parameters('{}ColorB_FK'.format(prefix)).Value
			
	model = parent.Model
	builder = model.Properties('RigBuilder')
	if not builder:
		XSI.LogMessage('[RigControl] Invalid parent object, aborted !')
		return False
		
	elem_ppg = crv.Properties('RigElement')
	if not elem_ppg:
		XSI.LogMessage('[RigControl] Guide curve is not a rig element, aborted !')
		return False
		
	ctrls = XSIFactory.CreateActiveXObject('XSI.Collection')
	
	basename = crv.Name.replace('_Crv', '')
	vertices = crv.ActivePrimitive.Geometry.Points
	
	up_x = elem_ppg.Parameters('UpVectorX').Value
	up_y = elem_ppg.Parameters('UpVectorY').Value
	up_z = elem_ppg.Parameters('UpVectorZ').Value
	
	up_vector = XSIMath.CreateVector3(up_x, up_y, up_z)
	direction = XSIMath.CreateVector3()
	for idx, vertex in enumerate(vertices):
		transform = XSIMath.CreateTransform()
		p0 = vertex.Position
		transform.SetTranslation(p0)
		
		if idx < vertices.Count-1:
			p1 = vertices[idx+1].Position
			direction.Sub(p1, p0)
			direction.NormalizeInPlace()
			rot = Utils.GetRotationFromTwoVectors(direction, up_vector)
			transform.SetRotation(rot)
			fk = Icon.simple(
				parent,
				'{}_FK{}_Ctrl'.format(basename, idx+1),
				[r, g, b],
				transform,
				p1
			)
		else:
			fk = Icon.simple(
				parent,
				'{}_FK{}_Ctrl'.format(basename, idx+1),
				[r, g, b],
				transform,
				p0
			)
		parent = fk
		ctrls.Add(fk) 
		
	Utils.ResetStaticKinematicState(ctrls)
	XSI.SetNeutralPose(ctrls, 'siSRT', '')
	Utils.GroupSetup(model, ctrls, inGroupName='FK_Rig')

	return True


# -------------------------------------------------------------------
# Build SRT RiG (Points on Curve)
# -------------------------------------------------------------------
def BuildSRTRig(parent, crv, ppg, side=MIDDLE, simple=False):
	prefix = 'M_'
	if side == LEFT:
		prefix = 'L_'
	elif side == RIGHT:
		prefix = 'R_'
			
	r = ppg.Parameters('{}ColorR_FK'.format(prefix)).Value
	g = ppg.Parameters('{}ColorG_FK'.format(prefix)).Value
	b = ppg.Parameters('{}ColorB_FK'.format(prefix)).Value
			
	model = parent.Model
	builder = model.Properties('RigBuilder')
	if not builder:
		XSI.LogMessage('[RigControl] : Invalid Parent Object ---> Aborted!')
		return False
		
	elem_ppg = crv.Properties('RigElement')
	if not elem_ppg:
		XSI.LogMessage('[RigControl] : Guide Curve is NOT a Rig Element ---> Aborted!')
		return False
		
	controls = XSIFactory.CreateActiveXObject('XSI.Collection')
	
	basename = crv.Name.replace('_Crv', '')
	vertices = crv.ActivePrimitive.Geometry.Points
	for idx, vertex in enumerate(vertices):
		transform = XSIMath.CreateTransform()
		p0 = vertex.Position
		transform.SetTranslation(p0)
		srt = Icon.IRIcon(
			parent,
			'{}{}_Ctrl'.format(basename, idx),
			transform,
			[r, g, b],
			'sphere',
			0.1,
			0.1,
			0.1
		)
		controls.Add(srt)
		
	Utils.ResetStaticKinematicState(controls)
	XSI.SetNeutralPose(controls, 'siSRT', '')
	Utils.GroupSetup(model, controls, inGroupName='FK_Rig')


# -------------------------------------------------------------------
# Build FootRoll RiG
# -------------------------------------------------------------------
def BuildFootRollRig(parent, crv, ppg, side=MIDDLE, simple=False):
	prefix = 'M_'
	if side == LEFT:
		prefix = 'L_'
	elif side == RIGHT:
		prefix = 'R_'
		
	basename = crv.Name.replace("_Crv", "")
	
	r = ppg.Parameters('{}ColorR_FK'.format(prefix)).Value
	g = ppg.Parameters('{}ColorG_FK'.format(prefix)).Value
	b = ppg.Parameters('{}ColorB_FK'.format(prefix)).Value

	axis = XSIMath.CreateVector3()
	d1 = XSIMath.CreateVector3()
	d2 = XSIMath.CreateVector3()
	
	points = crv.ActivePrimitive.Geometry.Points
	pos = []
	for point in points:
		pos.append(point.Position)
		
	last = points.Count-1
	d1.Sub(pos[1], pos[0])
	d2.Sub(pos[last], pos[0])
	
	d1.NormalizeInPlace()
	d2.NormalizeInPlace()
	
	axis.Cross(d1, d2)
	
	# build root control
	min_z = pos[0].Z
	for i, point in enumerate(pos):
		if i == 0:
			continue
		if point.Z < min_z:
			min_z = point.Z
			
	average = Utils.GetAveragePosition(pos)
	average.Z = min_z
	
	length = Utils.GetCurveLength(crv)
	transform = XSIMath.CreateTransform()
	transform.SetTranslation(average)
	main = Icon.IRIcon(
		parent,
		'{}_MainControl'.format(basename),
		transform,
		[r, g, b],
		'circle',
		0.5*length,
		0.5*length,
		0.5*length
	)
	parent = main
	parents = []
	
	ctrls = []
	# build reverse structure
	for i in range(0, last):
		transform.SetTranslation(pos[last-i])
		parent = Icon.IRIcon(
			parent,
			'{}{}{}_Ctrl'.format(prefix, basename, i),
			transform,
			[r, g, b],
			'bendedarrow2',
			0.2*length,
			0.2*length,
			0.2*length
		)
		parents.append(parent)
		ctrls.append(parent)
			
	# build final control structure
	fks = []
	for i in range(0, last+1):
		transform.SetTranslation(pos[last-i])
		if i == 0:
			icon = Icon.IRIcon(
				main,
				'{}{}{}_Ctrl'.format(prefix, basename, i),
				transform,
				[r, g, b],
				"sphere",
				0.1*length,
				0.1*length,
				0.1*length
			)
			fks.append(icon)
			ctrls.append(icon)
		else:
			icon = Icon.IRIcon(
				parents[i-1],
				'{}{}{}_Ctrl'.format(prefix, basename, i),
				transform,
				[r, g, b],
				'sphere',
				0.1*length,
				0.1*length,
				0.1*length
			)
			fks.append(icon)
			ctrls.append(icon)
			
	Utils.ResetStaticKinematicState(fks)
	Utils.GroupSetup(parent.Model, fks, 'IK_Rig')
	
	XSI.SetNeutralPose(ctrls, 'siSRT', '')
	Utils.GroupSetup(parent.Model, ctrls, 'Anim_Controls')
