# -------------------------------------------------------------------
# Muscle
# -------------------------------------------------------------------
from Globals import XSI
from win32com.client import constants as siConstants
from Constants import *
import Element as ele
import Utils as uti
import ICETree as tre


# -------------------------------------------------------------------
# Muscle Element Class
# -------------------------------------------------------------------
class IRMuscle(ele.IRElement):
	def __init__(self, crv, parent, name, side=MIDDLE, mirror=False, suffix="_Crv"):
		super(IRMuscle, self).__init__(crv, parent, name, side, mirror, suffix)

		self.type = ID_SKELETON
		self.list = "MuscleList"
		self.chooser = "MuscleChooser"
		if self.crv:
			self.prop = self.crv.Properties(PROP_MUSCLE)

		self.datacls = None
		self.widthmap = None
		self.depthmap = None
		self.twistmap = None
		self.collidemap = None
		self.stickmap = None
		self.offsetshape = None
		self.nbp = 0

		self.root = self.model.FindChild("Muscles")

	# ---------------------------------------------------------------
	# Override Pick Position
	# ---------------------------------------------------------------
	def PickPosition(self):
		self.position = uti.PickMultiPosition(-1, False)
		if len(self.position) < 2:
			XSI.LogMessage("[Create Element] : You need to pick at least TWO points ---> aborted...!",
                           siConstants.siWarning)
			return False
			self.rotation = uti.GetOrientationFromPickPosition(self.position)
		return True

	def CreateGuide(self):
		if not self.root:
			self.root = uti.AddNull(self.model, 0, 1, "Muscles", 0, 0, 0)
		
		if self.CheckElementExist():
			XSI.LogMessage("[BuildRigMuscle] : " + self.fullname + self.suffix + " already exist ---> skipped...!")
			return False

		self.nbp = len(self.position) + 2
		self.crv = uti.BuildCurveOnPositions(self.position)

		self.datacls = uti.CreateAlwaysCompleteCluster(self.crv, siConstants.siVertexCluster, "ElementData")
		self.widthmap = uti.GetWeightMap(self.crv, "WidthMap", 0.01, 0, 1, self.datacls)
		self.depthmap = uti.GetWeightMap(self.crv, "DepthMap", 0.01, 0, 1, self.datacls)
		self.twistmap = uti.GetWeightMap(self.crv, "TwistMap", 0.01, -1, 1, self.datacls)
		self.collidemap = uti.GetWeightMap(self.crv, "CollideMap", 1, 0, 1, self.datacls)
		self.stickmap = uti.GetWeightMap(self.crv, "StickMap", 1, 0, 1, self.datacls)
		self.offsetshape = self.datacls.AddProperty("SimpleDeformShape", 0, "Offset")

		self.crv.Name = self.fullname+self.suffix
		uti.SetWireColor(self.crv, COLOR_MUSCLE_R, COLOR_MUSCLE_G, COLOR_MUSCLE_B)
		self.root.AddChild(self.crv)

		self.prop = self.crv.AddProperty(PROP_MUSCLE)

		uti.GroupSetup(self.model, [self.crv], "Muscle_Curves")
		return True

	# ---------------------------------------------------------------
	# Get Existing Muscle Element
	# -------------------------------------------------------------------
	def GetFromGuide(self, crv):
		pass

	# ---------------------------------------------------------------
	# Symmetrize Muscle
	# ---------------------------------------------------------------
	def Symmetrize(self):
		if self.side == MIDDLE:
			XSI.LogMessage("[ICERig] : Can't symmetrize middle elements ---> aborted...!")
			return None

		sibling = IRMuscle(
			None,
			self.GetSymParent(self.parent),
			self.name,
			self.GetOppositeSide(),
			self.mirror,
			self.suffix
		)

		sibling.position = self.position
		sibling.rotation = self.rotation
		sibling.sibling = self
		self.sibling = sibling

		sibling.CreateGuide()

		# add symmetrize element data ICE Tree
		t = tre.CreateICETree(sibling.crv, "SymmetrizeMuscleGuide", 2)
		n = XSI.AddICECompoundNode("IRSymetrizeMuscleDatas", str(t))

		XSI.ConnectICENodes(str(t) + ".port1", str(n) + ".Execute")
		name = tre.ReplaceModelNameByThisModel(self.crv, self.model)
		XSI.SetValue(str(n) + ".Reference", name, "")

		return sibling
			
	def AddToRig(self):
		CreateMuscleCloud(self.model,False)
		#CreateMuscleMesh(self.model,False)


# -------------------------------------------------------------------
# Create Muscle Cloud
# -------------------------------------------------------------------
def CreateMuscleCloud(model, deform=False):
	cloud = model.FindChild("ICE_Muscles")
	if cloud:
		XSI.LogMessage("[CreateMuscleCloud] Muscle Cloud already exists : skipped....")
		return cloud
	
	cloud = model.AddPrimitive("PointCloud", "ICE_Muscles")
	tree = tre.CreateICETree(cloud, "Emit", 0)
	
	muscle = XSI.AddICECompoundNode("IRMuscleCloudInit", str(tree))
	XSI.ConnectICENodes(str(tree) + ".port1", str(muscle) + ".Execute")
	
	tree = tre.CreateICETree(cloud, "Deform", 2)
	
	bind = XSI.AddICECompoundNode("BindMusclesToSkeleton", str(tree))
	XSI.ConnectICENodes(str(tree) + ".port1", str(bind) + ".Execute")
	
	return cloud
	
def CreateMuscleMesh(model, deform=True):
	root = model.FindChild("Muscles")
	if not root:
		root = model.AddNull("Muscles")
		
	static = model.FindChild("Muscle_Static")
	if not static:
		static = root.AddPrimitive("EmptyPolygonMesh")
		static.Name = "Muscle_Static"
		tree = tre.CreateICETree(static, "Init", 0)
	
		build = XSI.AddICECompoundNode("IRBuildMuscles", str(tree))
		XSI.ConnectICENodes(str(tree) + ".port1", str(build) + ".Execute")
	
	deform = model.FindChild("Muscle_Deform")
	if not deform:
		deform = root.AddPrimitive("EmptyPolygonMesh")
		deform.Name = "Muscle_Deform"

	'''
	mesh = model.AddPolymesh("PointCloud","ICE_Muscles")
	tree = tre.CreateICETree(cloud,"Emit",0)
	
	muscle = xsi.AddICECompoundNode("MusclesFromNull", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1",str(muscle)+".Execute")
	
	tree = tre.CreateICETree(cloud,"Deform",2)
	
	bind = xsi.AddICECompoundNode("BindMusclesToSkeleton", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(bind)+".Execute")
	'''
	return None
