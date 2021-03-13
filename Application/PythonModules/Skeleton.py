# -------------------------------------------------------------------
# Skeleton
# -------------------------------------------------------------------
from win32com.client import constants
from Globals import XSI
from Globals import XSIMath
from Constants import *
import Element
import Utils
import ICETree


# -------------------------------------------------------------------
# Skeleton Element Class
# -------------------------------------------------------------------
class IRSkeleton(Element.IRElement):
	# ---------------------------------------------------------------
	# Constructor
	# ---------------------------------------------------------------
	def __init__(self, crv, parent, name, mode=ID_CHAIN, side=MIDDLE, divisions=1, mirror=False, suffix='_Crv'):
		super(IRSkeleton, self).__init__(crv, parent, name, side, mirror, suffix)

		self.type = ID_SKELETON
		self.mode = mode
		self.list = 'SkeletonList'
		self.chooser = 'SkeletonChooser'
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

		self.root = self.model.FindChild('Guides', constants.siNullPrimType)

	# ---------------------------------------------------------------
	# Override Pick Position
	# ---------------------------------------------------------------
	def PickPosition(self):
		self.position = Utils.PickMultiPosition()

		if len(self.position) < 2:
			XSI.LogMessage('[Create Element] You need to pick at least two points !', constants.siWarning)
			return False
		self.rotation = Utils.GetOrientationFromPickPosition(self.position)
		return True

	# ---------------------------------------------------------------
	# Create Skeleton Element
	# ---------------------------------------------------------------
	def CreateGuide(self):
		if self.CheckElementExist():
			XSI.LogMessage('[CreateSkeletonElement] {}_Crv already exist !'.format(self.fullname), constants.siWarning)
			return False

		if self.selfsymmetrize:
			self.crv = Utils.BuildCurveOnSymmetrizedPositions(self.position)
		else:
			self.crv = Utils.BuildCurveOnPositions(self.position)
		self.datacls = Utils.CreateAlwaysCompleteCluster(self.crv, constants.siVertexCluster, 'ElementData')
		self.widthmap = Utils.GetWeightMap(self.crv, 'WidthMap', 0.01, 0, 1.0, self.datacls)
		self.depthmap = Utils.GetWeightMap(self.crv, 'DepthMap', 0.01, 0, 1.0, self.datacls)
		self.collidemap = Utils.GetWeightMap(self.crv, 'CollideMap', 1.0, 0.0, 1.0, self.datacls)
		self.stickmap = Utils.GetWeightMap(self.crv, 'StickMap', 1.0, 0.0, 1.0, self.datacls)
		self.offsetshape = self.datacls.AddProperty('SimpleDeformShape', 0, 'Offset')

		# freeze curve
		XSI.FreezeObj(self.crv)

		# create self-symmetrize node
		if self.selfsymmetrize:
			tree = ICETree.CreateICETree(self.crv, 'SelfSymmetrize', 2)
			node = XSI.AddICECompoundNode('IRSelfSymmetrizeSkeletonDatas', str(tree))
			XSI.AddPortToICENode('{}.port1'.format(tree), 'siNodePortDataInsertionLocationAfter')
			XSI.ConnectICENodes('{}.port2'.format(tree), '{}.Execute'.format(node))

		self.root.AddChild(self.crv)
		Utils.SetWireColor(self.crv, COLOR_SKELETON_R, COLOR_SKELETON_G, COLOR_SKELETON_B)
		self.crv.Name = self.fullname+self.suffix

		# add rig element property
		self.prop = self.crv.AddProperty('RigElement')
		self.prop.Parameters('ElementType').Value = self.mode
		self.prop.Parameters('ElementName').Value = self.name
		self.prop.Parameters('Divisions').Value = self.divisions
		self.prop.Parameters('Side').Value = self.side
		# self.prop.Parameters("ControlType").Value = self.controltype

		Utils.GroupSetup(self.model, [self.crv], "Guide_Curves")

		return True

	# ---------------------------------------------------------------
	# Get Existing Skeleton Element
	# ---------------------------------------------------------------
	def GetFromGuide(self, crv):
		self.crv = crv
		self.model = self.crv.Model
		self.selfSymmetrize = False

		# add rig element property
		self.prop = self.crv.Properties('RigElement')
		self.mode = self.prop.Parameters('ElementType').Value
		self.fullname = self.prop.Parameters('ElementName').Value
		
		self.divisions = self.prop.Parameters('Divisions').Value
		self.side = self.prop.Parameters('Side').Value
		self.suffix = '_Crv'
		
		# add control element property
		# self.controltype = self.prop.Parameters("ControlType").Value

		self.position = []
		self.rotation = []
		for point in self.crv.ActivePrimitive.Geometry.Points:
			self.position.append(point.Position)

	# ---------------------------------------------------------------
	# Symmetrize Skeleton
	# ---------------------------------------------------------------
	def Symmetrize(self):
		if self.side == MIDDLE:
			XSI.LogMessage('[BuildRigElement] : Can\'t symmetrize middle elements !')
			return None

		sym_element = IRSkeleton(
			None,
			self.GetSymParent(self.parent),
			self.name,
			self.mode,
			self.GetOppositeSide(),
			self.divisions,
			self.selfsymmetrize,
			self.suffix
		)

		sym_element.position = self.position
		sym_element.rotation = self.rotation
		sym_element.symElement = self
		self.symElement = sym_element

		sym_element.CreateGuide()

		# add symmetrize element data ICE Tree
		tree = ICETree.CreateICETree(sym_element.crv, 'SymmetyrizeDatas', 2)
		node = XSI.AddICECompoundNode('IRSymetrizeSkeletonDatas', str(tree))

		XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(node))
		name = ICETree.ReplaceModelNameByThisModel(self.crv, self.model)
		XSI.SetValue('{}.Reference'.format(node), name, '')

		return sym_element

	# ---------------------------------------------------------------
	# Remove Element	
	# ---------------------------------------------------------------
	def RemoveFromRig(self):
		XSI.LogMessage('[RigSkeleton] Remove skeleton element not implemented !')

	# ---------------------------------------------------------------
	# Get Skin Copies	
	# ---------------------------------------------------------------
	def GetSkinCopies(self):
		self.skincopies = []
		builder = self.GetBuilder()
		geometries = builder.Parameters('GeometryList').Value
		split = geometries.split('|')
		split.pop()
		
		for token in split:
			copy = self.model.FindChild('{}_Copy'.format(token))
			if copy:
				self.skincopies.append(copy)
			else:
				XSI.LogMessage('[RigSkeleton] Can\'t find object skin "{}"'.format(token))


# -------------------------------------------------------------------
# Create Skeleton Curve
# -------------------------------------------------------------------
def CreateSkeletonCurve(curves):
	if curves.Count > 1:
		XSI.SelectObj(curves)
		op = XSI.MergeCurve(curves.GetAsText(), '')
		crv = op.Parent3DObject
	else:
		crv = XSI.Duplicate(curves(0))(0)
		
	crv.Name = 'Skeleton_Crv'
	return crv


# -------------------------------------------------------------------
# Create Fitter ICE Tree
# -------------------------------------------------------------------
def CreateFitter(ppg, obj, mesh):
	tree = ICETree.CreateICETree(obj, 'Fitter', 2)
	node = XSI.AddICECompoundNode('IRFitSkeleton', str(tree))
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(node))

	if obj and mesh:
		name = ICETree.ReplaceModelNameByThisModel(mesh, obj.Model)
		XSI.SetValue('{}.Reference'.format(node), name, '')
	else:
		XSI.LogMessage('No geometry set, Fitter may be invalid', constants.siWarningMsg)
	return tree


# -------------------------------------------------------------------
# Create Skeleton Cloud
# -------------------------------------------------------------------
def CreateSkeletonCloud(model):
	cloud = model.FindChild('ICE_Skeleton')
	if cloud:
		XSI.DeleteObj(cloud)
	
	cloud = model.AddPrimitive('PointCloud', 'ICE_Skeleton')
	tree = ICETree.CreateICETree(cloud, 'Emit', 0)

	emit = XSI.AddICECompoundNode('IRBuildSkeleton', tree)
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(emit))
	
	tree = ICETree.CreateICETree(cloud, 'Deform', 2)
	deform = XSI.AddICECompoundNode('IRDeformSkeleton', tree)
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(deform))
	
	return cloud


# -------------------------------------------------------------------
# Delete Skeleton Cloud
# -------------------------------------------------------------------
def DeleteSkeletonCloud(model):
	cloud = model.FindChild('ICE_Skeleton')
	if cloud:
		XSI.DeleteObj(cloud)


# -------------------------------------------------------------------
# Create Envelope Nulls From Skeleton Cloud
# -------------------------------------------------------------------
def CreateEnvelopeNullsFromSkeletonCloud(model):
	cloud = model.FindChild('ICE_Skeleton')
	grp = Utils.GroupSetup(model, None, 'Envelope')
	
	root = model.FindChild('Envelopes', constants.siNullPrimType)
	if root:
		XSI.SelectObj(root, 'BRANCH')
		XSI.DeleteObj()
		
	root = model.AddNull('Envelopes')
	idx = 0
	attrs = cloud.ActivePrimitive.ICEAttributes
	pos = attrs('PointPosition').DataArray
	scl = attrs('Scale').DataArray
	ori = attrs('Orientation').DataArray

	# we have to display this attribute for accessing it
	emit = cloud.ActivePrimitive.ICETrees(1)
	node = emit.Nodes(0)
	XSI.LogMessage(node)
	XSI.SetValue('{}.Show_Name'.format(node), True, '')
	skeleton_name = cloud.ActivePrimitive.GetICEAttributeFromName('SkeletonName').DataArray
	XSI.SetValue('{}.Show_Name'.format(node), False, '')

	group = Utils.GroupSetup(model, None, 'Envelope')

	t = XSIMath.CreateTransform()
	last_name = ''
	sub_index = 1
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
		current_name = skeleton_name[idx]
		if not skeleton_name:
			XSI.LogMessage('You need to display SkeletonName ICE Attribute !', constants.siWarning)
			break
		else:
			if not current_name == last_name:
				last_name = current_name
				sub_index = 1
				
			n = root.AddNull('{}{}_Env'.format(current_name, sub_index))
			n.ActivePrimitive.Primary_Icon = 4
			group.AddMember(n)
			n.Kinematics.Global.Transform = t
			idx += 1
			sub_index += 1
	
	return grp
