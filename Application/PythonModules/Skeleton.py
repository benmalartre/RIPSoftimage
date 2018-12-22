# -------------------------------------------------------------------
# Skeleton
# -------------------------------------------------------------------
from win32com.client import constants as siConstants
from Globals import xsi
from Globals import XSIMath
from Constants import *
import Element as ele
import Utils as uti
import ICETree as tre


# -------------------------------------------------------------------
# Skeleton Element Class
# -------------------------------------------------------------------
class IRSkeleton(ele.IRElement):
	# ---------------------------------------------------------------
	# Constructor
	# ---------------------------------------------------------------
	def __init__(self, crv, parent, name, mode=ID_CHAIN, side=MIDDLE, divisions=1, selfsymmetrize=False, suffix="_Crv"):
		super(IRSkeleton, self).__init__(crv, parent, name, side, selfsymmetrize, suffix)

		self.type = ID_SKELETON
		self.mode = mode
		self.list = "SkeletonList"
		self.chooser = "SkeletonChooser"
		self.divisions = divisions
		if self.crv:
			self.prop = self.crv.Properties(PROP_SKELETON)

		self.datacls = None
		self.widthmap = None
		self.depthmap = None
		self.twistmap = None
		self.collidemap = None
		self.stickmap = None
		self.offsetshape = None

		self.root = self.model.FindChild("Guides", siConstants.siNullPrimType)

	# ---------------------------------------------------------------
	# Override Pick Position
	# ---------------------------------------------------------------
	def PickPosition(self):
		self.position = uti.PickMultiPosition()

		if len(self.position) < 2:
			xsi.LogMessage("[Create Element] : You need to pick at least TWO points ---> aborted...!",
						   siConstants.siWarning)
			return False
		self.rotation = uti.GetOrientationFromPickPosition(self.position)
		return True

	# ---------------------------------------------------------------
	# Create Skeleton Element
	# ---------------------------------------------------------------
	def CreateGuide(self):
		if self.CheckElementExist():
			xsi.LogMessage("[CreateSkeletonElement] : "+self.fullname+"_Crv already exist ---> aborted...!", siConstants.siWarning)
			return False

		if self.selfsymmetrize:
			self.crv = uti.BuildCurveOnSymmetrizedPositions(self.position)
		else:
			self.crv = uti.BuildCurveOnPositions(self.position)
		self.datacls = uti.CreateAlwaysCompleteCluster(self.crv, siConstants.siVertexCluster, "ElementData")
		self.widthmap = uti.GetWeightMap(self.crv, "WidthMap", 0.01, 0, 1.0, self.datacls)
		self.depthmap = uti.GetWeightMap(self.crv, "DepthMap", 0.01, 0, 1.0, self.datacls)
		self.collidemap = uti.GetWeightMap(self.crv, "CollideMap", 1.0, 0.0, 1.0, self.datacls)
		self.stickmap = uti.GetWeightMap(self.crv, "StickMap", 1.0, 0.0, 1.0, self.datacls)
		self.offsetshape = self.datacls.AddProperty("SimpleDeformShape", 0, "Offset")

		# freeze curve
		xsi.FreezeObj(self.crv)

		# create self-symmetrize node
		if self.selfsymmetrize:
			t = tre.CreateICETree(self.crv, "SelfSymmetrize", 2)
			n = xsi.AddICECompoundNode("IRSelfSymmetrizeSkeletonDatas", str(t))
			xsi.AddPortToICENode(str(t)+".port1", "siNodePortDataInsertionLocationAfter")
			xsi.ConnectICENodes(str(t)+".port2", str(n)+".Execute")

		self.root.AddChild(self.crv)
		uti.SetWireColor(self.crv, COLOR_SKELETON_R, COLOR_SKELETON_G, COLOR_SKELETON_B)
		self.crv.Name = self.fullname+self.suffix

		# add rig element property
		self.prop = self.crv.AddProperty("RigElement")
		self.prop.Parameters("ElementType").Value = self.mode
		self.prop.Parameters("ElementName").Value = self.name
		self.prop.Parameters("Divisions").Value = self.divisions
		self.prop.Parameters("Side").Value = self.side
		# self.prop.Parameters("ControlType").Value = self.controltype

		uti.GroupSetup(self.model, [self.crv], "Guide_Curves")

		return True

	# ---------------------------------------------------------------
	# Get Existing Skeleton Element
	# ---------------------------------------------------------------
	def GetFromGuide(self, crv):
		self.crv = crv
		self.model = self.crv.Model
		self.selfsymmetrize = False

		points = []
		
		# add rig element property
		self.prop = self.crv.Properties("RigElement")
		self.mode = self.prop.Parameters("ElementType").Value
		self.fullname = self.prop.Parameters("ElementName").Value
		
		self.divisions = self.prop.Parameters("Divisions").Value
		self.side = self.prop.Parameters("Side").Value
		self.suffix = "_Crv"
		
		# add control element property
		# self.controltype = self.prop.Parameters("ControlType").Value

		self.position = []
		self.rotation = []
		for gp in self.crv.ActivePrimitive.Geometry.Points:
			self.position.append(gp.Position)

	# ---------------------------------------------------------------
	# Symmetrize Skeleton
	# ---------------------------------------------------------------
	def Symmetrize(self):
		if self.side == MIDDLE:
			xsi.LogMessage("[BuildRigElement] : Can't symmetrize middle elements ---> aborted...!")
			return None

		symmelement = IRSkeleton(None,
									 self.GetSymParent(self.parent),
									 self.name,
									 self.mode,
									 self.GetOppositeSide(),
									 self.divisions,
									 self.selfsymmetrize,
									 self.suffix)

		symmelement.position = self.position
		symmelement.rotation = self.rotation
		symmelement.symmelement = self
		self.symmelement = symmelement

		symmelement.CreateGuide()

		# add symmetrize element data ICE Tree
		t = tre.CreateICETree(symmelement.crv, "SymmetyrizeDatas", 2)
		n = xsi.AddICECompoundNode("IRSymetrizeSkeletonDatas", str(t))

		xsi.ConnectICENodes(str(t) + ".port1", str(n) + ".Execute")
		name = tre.ReplaceModelNameByThisModel(self.crv, self.model)
		xsi.SetValue(str(n) + ".Reference", name, "")

		return symmelement

	# ---------------------------------------------------------------
	# Remove Element	
	# ---------------------------------------------------------------
	def RemoveFromRig(self):
		xsi.LogMessage("[RigSkeleton] : Remove Skeleton Element Called...")

	# ---------------------------------------------------------------
	# Get Skin Copies	
	# ---------------------------------------------------------------
	def GetSkinCopies(self):
		self.skincopies = []
		builder = self.GetBuilder()
		geometries = builder.Parameters("GeometryList").Value
		split = geometries.split("|")
		split.pop()
		
		for s in split:
			copy = self.model.FindChild(s+"_Copy")
			if copy:
				self.skincopies.append(copy)
			else:
				xsi.LogMessage("[RigSkeleton] : Can't find object skin : "+s)


# -------------------------------------------------------------------
# Create Skeleton Curve
# -------------------------------------------------------------------
def CreateSkeletonCurve(curves):
	if curves.Count > 1:
		xsi.SelectObj(curves)
		op = xsi.MergeCurve(curves.GetAsText(), "")
		crv = op.Parent3DObject
	else:
		crv = xsi.Duplicate(curves(0))(0)
		
	crv.Name = "Skeleton_Crv"
	return crv


# -------------------------------------------------------------------
# Create Fitter ICE Tree
# -------------------------------------------------------------------
def CreateFitter(ppg, obj, mesh):
	t = tre.CreateICETree(obj, "Fitter", 2)
	n = xsi.AddICECompoundNode("IRFitSkeleton", str(t))
	xsi.ConnectICENodes(str(t)+".port1", str(n)+".Execute")

	if obj and mesh:
		name = tre.ReplaceModelNameByThisModel(mesh, obj.Model)
		xsi.SetValue(str(n)+".Reference", name, "")
	else:
		xsi.LogMessage("No Geometry Set, Fitter may be invalid", siConstants.siWarningMsg)
	return t


# -------------------------------------------------------------------
# Create Skeleton Cloud
# -------------------------------------------------------------------
def CreateSkeletonCloud(model):
	cloud = model.FindChild("ICE_Skeleton")
	if cloud:
		xsi.DeleteObj(cloud)
	
	cloud = model.AddPrimitive("PointCloud", "ICE_Skeleton")
	tree = tre.CreateICETree(cloud, "Emit", 0)

	emit = xsi.AddICECompoundNode("IRBuildSkeleton", tree)
	xsi.ConnectICENodes(str(tree)+".port1", str(emit)+".Execute")
	
	tree = tre.CreateICETree(cloud, "Deform", 2)
	deform = xsi.AddICECompoundNode("IRDeformSkeleton", tree)
	xsi.ConnectICENodes(str(tree)+".port1", str(deform)+".Execute")
	
	return cloud


# -------------------------------------------------------------------
# Delete Skeleton Cloud
# -------------------------------------------------------------------
def DeleteSkeletonCloud(model):
	cloud = model.FindChild("ICE_Skeleton")
	if cloud:
		xsi.DeleteObj(cloud)


# -------------------------------------------------------------------
# Create Envelope Nulls From Skeleton Cloud
# -------------------------------------------------------------------
def CreateEnvelopeNullsFromSkeletonCloud(model):
	cloud = model.FindChild("ICE_Skeleton")
	grp = uti.GroupSetup(model, None, "Envelope")
	
	root = model.FindChild("Envelopes", siConstants.siNullPrimType)
	if root:
		xsi.SelectObj(root, "BRANCH")
		xsi.DeleteObj()
		
	root = model.AddNull("Envelopes")
	idx = 0
	attrs = cloud.ActivePrimitive.ICEAttributes
	pos = attrs("PointPosition").DataArray
	scl = attrs("Scale").DataArray
	ori = attrs("Orientation").DataArray

	# we have to display this attribute for accessing it
	emit = cloud.ActivePrimitive.ICETrees(1)
	node = emit.Nodes(0)
	xsi.LogMessage(node)
	xsi.SetValue(str(node)+".Show_Name", True, "")
	sname = cloud.ActivePrimitive.GetICEAttributeFromName("SkeletonName").DataArray
	xsi.SetValue(str(node)+".Show_Name", False, "")

	group = uti.GroupSetup(model, None, "Envelope")

	t = XSIMath.CreateTransform()
	lastname = ""
	subidx = 1
	for idx in xrange(cloud.ActivePrimitive.Geometry.Points.Count):
		p = pos[idx]
		o = ori[idx]
		s = scl[idx]
		t.SetTranslation(p)
		t.SetRotation(o)
		t.SetScaling(s)
		
		'''
		n = root.AddNull("Buffer"+str(id))
		tr = tree.CreateICETree(n,"Kinematics")
		node = xsi.AddICECompoundNode("BindTransformToPoint", str(tr))
		xsi.ConnectICENodes(str(tr)+".port1", str(node)+".Execute")
		xsi.SetValue(str(node)+".ID", id, "")
		'''
		currentname = sname[idx]
		if not sname:
			xsi.LogMessage("You need to display Skeleton Name ICE Attribute!!!", siConstants.siWarning)
			break
		else:
			if not currentname == lastname:
				lastname = currentname
				subidx = 1
				
			n = root.AddNull(currentname+str(subidx)+"_Env")
			n.ActivePrimitive.Primary_Icon = 4
			group.AddMember(n)
			n.Kinematics.Global.Transform = t
			idx += 1
			subidx += 1
	
	return grp
