# -------------------------------------------------------------------
# Control
# -------------------------------------------------------------------
from win32com.client import constants
from Globals import xsi
from Globals import XSIMath
from Globals import XSIFactory
from Constants import *
import Element as ele
import Utils as uti
import ICETree as tre
import Icon as ico


# -------------------------------------------------------------------
# Control Element Class
# -------------------------------------------------------------------
class IRControl(ele.IRElement):
	# ---------------------------------------------------------------
	# Constructor
	# ---------------------------------------------------------------
	def __init__(self, crv, parent, name, icon=0, mode=CTRL_FK, side=MIDDLE, suffix="_XX"):
		super(IRControl, self).__init__(crv, parent, name, side, False, suffix)

		self.type = ID_CONTROL
		self.mode = mode
		self.list = "ControlList"
		self.chooser = "ControlChooser"
		self.size = 1.0
		self.icon = RIG_CONTROL_ICON[icon]
		if self.crv:
			self.prop = self.crv.Properties(PROP_CONTROL)

		self.root = self.model.FindChild("Controls", constants.siNullPrimType)

	# ---------------------------------------------------------------
	# Override Pick Position
	# ---------------------------------------------------------------
	def PickPosition(self):
		self.position = uti.PickPosition()
		self.rotation = uti.GetOrientationFromPickPosition(self.position)
		return True

	# ---------------------------------------------------------------
	# Create Control Element
	# ---------------------------------------------------------------
	def CreateGuide(self):
		builder = self.GetBuilder()
		if self.CheckElementExist():
			xsi.LogMessage("[CreateControlElement] : " + self.fullname+"_Ctrl already exist ---> aborted...!")
			return False

		t = XSIMath.CreateTransform()
		t.SetTranslation(self.position)

		self.crv = RIG_CONTROL_ICON[builder.Parameters("ControlIconType").Value]
		self.GetControlColor()
		color = [self.colorrik, self.colorgik, self.colorbik]
		self.size = builder.Parameters("GlobalSize").Value
		self.crv = ico.IRIcon(self.parent, self.fullname+"_Ctrl", t, color, self.icon, self.size, self.size, self.size)

		print self.model
		print self.crv

		uti.GroupSetup(self.model, [self.crv], "Control_Curves")
		return True

	# ---------------------------------------------------------------
	# Get Existing Control Element
	# ---------------------------------------------------------------
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

	# ---------------------------------------------------------------
	# Symmetrize Control
	# ---------------------------------------------------------------
	def Symmetrize(self):
		if self.side == MIDDLE:
			xsi.LogMessage("[CreateControlElement] : Can't symmetrize middle elements ---> aborted...!")
			return None
		
		axis = self.builder.Parameters("SymmetryAxis").Value
		
		self.symparent = self.model.FindChild(self.ctrl.Parent.Name.replace(self.prefix, self.symprefix))
		if not self.symparent:
			self.symparent = self.ctrl.Parent
			
		color = [self.colorrik, self.colorgik, self.colorbik]
		self.symctrl = ico.IRIcon(self.symparent,
								  self.symfullname+"_Ctrl",
								  XSIMath.CreateTransform(),
								  color,
								  self.icon,
								  self.size,
								  self.size,
								  self.size)
		uti.SimpleSymmetry(self.symctrl, self.ctrl, axis)
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

	# ---------------------------------------------------------------
	# Add to Rig
	# ---------------------------------------------------------------
	def AddToRig(self):
		xsi.LogMessage("[RigControl] : Add to Rig called...")

	# ---------------------------------------------------------------
	# Remove Element	
	# ---------------------------------------------------------------
	def RemoveFromRig(self):
		xsi.LogMessage("[RigControl] : Remove Control Element Called...")

	# ---------------------------------------------------------------
	# Get Control Color	
	# ---------------------------------------------------------------
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

	# ---------------------------------------------------------------
	# Set Control Color	
	# ---------------------------------------------------------------
	def SetControlColor(self):
		if self.type == CTRL_FK:
			uti.SetWireColor(self.ctrl, self.colorrfk, self.colorgfk, self.colorbfk)
		else:
			uti.SetWireColor(self.ctrl, self.colorrik, self.colorgik, self.colorbik)
			

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
		p1 = uti.GetAveragePosition(pos)
		p2 = pnts[last].Position
	
	dir1 = XSIMath.CreateVector3()
	dir1.Sub(p0, p1)
	dir2 = XSIMath.CreateVector3()
	dir2.Sub(p1, p2)
	
	l = dir1.Length() + dir2.Length()
	
	plane = XSIMath.CreateVector3()
	plane.Cross(dir1, dir2)
	plane.NormalizeInPlace()
	
	offset = XSIMath.CreateVector3()
	offset.Normalize(dir1)
	offset.ScaleInPlace(-l*0.5)
	offset.AddInPlace(p1)
	
	l1 = dir1.Length()
	l2 = dir2.Length()
	
	t = XSIMath.CreateTransform()
	t.SetTranslation(p0)
	
	root = ico.IRIcon(parent, name+"_Root_Ctrl", t, [r, g, b], "cube", 0.1*l, 0.1*l, 0.1*l)
	ik = uti.MakeRigNull(parent, 1, name+"_IK")
	
	t.SetTranslation(p1)
	ik.Kinematics.Global.Transform = t
	
	tree = tre.CreateICETree(ik, "IK", 2)
	iknode = xsi.AddICECompoundNode("IK2BonesKinematics", tree)
	xsi.ConnectICENodes(str(tree)+".port1", str(iknode)+".Execute")
	
	# set ik values
	xsi.SetValue(str(iknode)+".Reference", "this_model."+name+"_Root_Ctrl", "")
	xsi.SetValue(str(iknode)+".Reference1", "this_model."+name+"_Eff_Ctrl", "")
	xsi.SetValue(str(iknode)+".Reference2", "this_model."+name+"_UpV_Ctrl", "")
	xsi.SetValue(str(iknode)+".LengthA", l1, "")
	xsi.SetValue(str(iknode)+".LengthB", l2, "")

	knee = ico.IRIcon(ik, name+"_Knee_Ctrl", t, [r, g, b], "sphere", 0.1*l, 0.1*l, 0.1*l)
	
	t.SetTranslation(offset)
	upv = ico.IRIcon(root, name+"_UpV_Ctrl", t, [r, g, b], "pyramid", 0.05*l, 0.05*l, 0.05*l)
	
	t.SetTranslation(p2)
	eff = ico.IRIcon(parent, name+"_Eff_Ctrl", t, [r, g, b], "square", l, l, l)
	
	uti.ResetStaticKinematicState([root, knee, eff])
	uti.GroupSetup(parent.Model, [root, knee, eff], "IK_Rig")
	xsi.SetNeutralPose([root, knee, eff, upv], "siSRT", "")


# -------------------------------------------------------------------
# Build FK RiG
# -------------------------------------------------------------------
def BuildFKRig(parent, crv, ppg, side=MIDDLE, simple=False):
	prefix = "M_"
	if side == LEFT:
		prefix = "L_"
	elif side == RIGHT:
		prefix = "R_"
			
	r = ppg.Parameters(prefix+"ColorR_FK").Value
	g = ppg.Parameters(prefix+"ColorG_FK").Value
	b = ppg.Parameters(prefix+"ColorB_FK").Value
			
	model = parent.Model
	builder = model.Properties("RigBuilder")
	if not builder:
		xsi.LogMessage("[RigControl] : Invalid Parent Object ---> Aborted!")
		return False
		
	elemppg = crv.Properties("RigElement")
	if not elemppg:
		xsi.LogMessage("[RigControl] : Guide Curve is NOT a Rig Element ---> Aborted!")
		return False
		
	ctrls = XSIFactory.CreateActiveXObject("XSI.Collection")
	
	basename = crv.Name.replace("_Crv", "")
	vertices = crv.ActivePrimitive.Geometry.Points
	
	upv_x = elemppg.Parameters("UpVectorX").Value
	upv_y = elemppg.Parameters("UpVectorY").Value
	upv_z = elemppg.Parameters("UpVectorZ").Value
	
	up = XSIMath.CreateVector3(upv_x, upv_y, upv_z)
	dir = XSIMath.CreateVector3()
	for idx, vertex in enumerate(vertices):
		t = XSIMath.CreateTransform()
		p0 = vertex.Position
		t.SetTranslation(p0)
		
		if idx < vertices.Count-1:
			p1 = vertices[idx+1].Position
			dir.Sub(p1, p0)
			dir.NormalizeInPlace() 
			rot = uti.GetRotationFromTwoVectors(dir, up)
			t.SetRotation(rot)
			fk = ico.simple(parent, basename+"_FK"+str(idx+1)+"_Ctrl", [r, g, b], t, p1)
		else:
			fk = ico.simple(parent, basename+"_FK"+str(idx+1)+"_Ctrl", [r, g, b], t, p0)
		parent = fk
		ctrls.Add(fk) 
		
	uti.ResetStaticKinematicState(ctrls)
	xsi.SetNeutralPose(ctrls, "siSRT", "")
	uti.GroupSetup(model, ctrls, inGroupName="FK_Rig")

	return True


# -------------------------------------------------------------------
# Build SRT RiG (Points on Curve)
# -------------------------------------------------------------------
def BuildSRTRig(parent, crv, ppg, side=MIDDLE, simple=False):
	prefix = "M_"
	if side == LEFT:
		prefix = "L_"
	elif side == RIGHT:
		prefix = "R_"
			
	r = ppg.Parameters(prefix+"ColorR_FK").Value
	g = ppg.Parameters(prefix+"ColorG_FK").Value
	b = ppg.Parameters(prefix+"ColorB_FK").Value
			
	model = parent.Model
	builder = model.Properties("RigBuilder")
	if not builder:
		xsi.LogMessage("[RigControl] : Invalid Parent Object ---> Aborted!")
		return False
		
	elemppg = crv.Properties("RigElement")
	if not elemppg:
		xsi.LogMessage("[RigControl] : Guide Curve is NOT a Rig Element ---> Aborted!")
		return False
		
	ctrls = XSIFactory.CreateActiveXObject("XSI.Collection")
	
	basename = crv.Name.replace("_Crv", "")
	vertices = crv.ActivePrimitive.Geometry.Points
	for idx, vertex in enumerate(vertices):
		t = XSIMath.CreateTransform()
		p0 = vertex.Position
		t.SetTranslation(p0)
		srt = ico.IRIcon(parent, basename+str(idx)+"_Ctrl", t, [r, g, b], "sphere", 0.1, 0.1, 0.1)
		ctrls.Add(srt)
		
	uti.ResetStaticKinematicState(ctrls)
	xsi.SetNeutralPose(ctrls, "siSRT", "")
	uti.GroupSetup(model, ctrls, inGroupName="FK_Rig")


# -------------------------------------------------------------------
# Build FootRoll RiG
# -------------------------------------------------------------------
def BuildFootRollRig(parent, crv, ppg, side=MIDDLE, simple=False):
	prefix = "M_"
	if side == LEFT:
		prefix = "L_"
	elif side == RIGHT:
		prefix = "R_"
		
	basename = crv.Name.replace("_Crv", "")
	
	r = ppg.Parameters(prefix+"ColorR_FK").Value
	g = ppg.Parameters(prefix+"ColorG_FK").Value
	b = ppg.Parameters(prefix+"ColorB_FK").Value

	axis = XSIMath.CreateVector3()
	d1 = XSIMath.CreateVector3()
	d2 = XSIMath.CreateVector3()
	
	pnts = crv.ActivePrimitive.Geometry.Points
	pos = []
	for p in pnts:
		pos.append(p.Position)
		
	last = pnts.Count-1
	d1.Sub(pos[1], pos[0])
	d2.Sub(pos[last], pos[0])
	
	d1.NormalizeInPlace()
	d2.NormalizeInPlace()
	
	axis.Cross(d1, d2)
	
	# build root control
	minZ = pos[0].Z
	for i, p in enumerate(pos):
		if i == 0:
			continue
		if p.Z < minZ:
			minZ = p.Z
			
	average = uti.GetAveragePosition(pos)
	average.Z = minZ
	
	l = uti.GetCurveLength(crv)
	t = XSIMath.CreateTransform()
	t.SetTranslation(average)
	main = ico.IRIcon(parent, basename+"Main_Ctrl", t, [r, g, b], "circle", 0.5*l, 0.5*l, 0.5*l)
	parent = main
	parents = []
	
	ctrls = []
	# build reverse structure
	for i in range(0, last):
		t.SetTranslation(pos[last-i])
		parent = ico.IRIcon(parent, prefix+basename+str(i)+"_Ctrl", t, [r, g, b], "bendedarrow2", 0.2*l, 0.2*l, 0.2*l)
		parents.append(parent)
		ctrls.append(parent)
			
	# build final control structure
	fks = []
	for i in range(0, last+1):
		t.SetTranslation(pos[last-i])
		if i == 0:
			icon = ico.IRIcon(main, prefix+basename+str(i)+"_Ctrl", t, [r, g, b], "sphere", 0.1*l, 0.1*l, 0.1*l)
			fks.append(icon)
			ctrls.append(icon)
		else:
			icon = ico.IRIcon(parents[i-1], prefix+basename+str(i)+"_Ctrl", t, [r, g, b], "sphere", 0.1*l, 0.1*l, 0.1*l)
			fks.append(icon)
			ctrls.append(icon)
			
	uti.ResetStaticKinematicState(fks)
	uti.GroupSetup(parent.Model, fks, "IK_Rig")
	
	xsi.SetNeutralPose(ctrls, "siSRT", "")
	uti.GroupSetup(parent.Model, ctrls, "Anim_Ctrls")