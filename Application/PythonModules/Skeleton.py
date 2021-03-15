from win32com.client import constants
from Globals import XSI
from Globals import XSIMath
from Constants import *
import Element
import Utils
import ICETree


class IRSkeleton(Element.IRElement):
	""" IRSkeleton class for a skeleton element
	:param X3DObject crv: guide curve
	:param X3DObject parent: parent 3d object
	:param str name: name of the element
	:param int mode: skeleton mode (chain, spine or tail)
	:param int side: side of the element
	:param int divisions: segment num elements
	:param bool mirror: mirror element (self-symmetry)
	:param str suffix: suffix
	"""
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

		self.cluster = None
		self.width = None
		self.depth = None
		self.twist = None
		self.collide = None
		self.stick = None
		self.offset = None

		self.root = self.model.FindChild('Guides', constants.siNullPrimType)

	def PickPosition(self):
		self.position = Utils.PickMultiPosition()

		if len(self.position) < 2:
			XSI.LogMessage(
				'[Create Element] You need to pick at least two points !',
				constants.siWarning
			)
			return False
		self.rotation = Utils.GetOrientationFromPickPosition(self.position)
		return True

	def CreateGuide(self):
		if self.CheckElementExist():
			XSI.LogMessage('[CreateSkeletonElement] {}_Crv already exist !'.format(self.fullname), constants.siWarning)
			return False

		if self.mirror:
			self.crv = Utils.BuildCurveOnSymmetrizedPositions(self.position)
		else:
			self.crv = Utils.BuildCurveOnPositions(self.position)
		self.cluster = Utils.CreateAlwaysCompleteCluster(self.crv, constants.siVertexCluster, 'ElementData')
		Utils.GetWeightMap(self.crv, 'WidthMap', 0.01, 0, 1.0, self.cluster)
		Utils.GetWeightMap(self.crv, 'DepthMap', 0.01, 0, 1.0, self.cluster)
		Utils.GetWeightMap(self.crv, 'CollideMap', 1.0, 0.0, 1.0, self.cluster)
		Utils.GetWeightMap(self.crv, 'StickMap', 1.0, 0.0, 1.0, self.cluster)
		self.cluster.AddProperty('SimpleDeformShape', 0, 'Offset')

		# freeze curve
		XSI.FreezeObj(self.crv)

		# create self-symmetrize node
		if self.mirror:
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

		Utils.GroupSetup(self.model, [self.crv], 'Guide_Curves')

		return True

	def GetFromGuide(self, crv):
		self.crv = crv
		self.model = self.crv.Model
		self.mirror = False

		self.prop = self.crv.Properties('RigElement')
		self.mode = self.prop.Parameters('ElementType').Value
		self.fullname = self.prop.Parameters('ElementName').Value
		
		self.divisions = self.prop.Parameters('Divisions').Value
		self.side = self.prop.Parameters('Side').Value
		self.suffix = '_Crv'

		self.position = []
		self.rotation = []
		for point in self.crv.ActivePrimitive.Geometry.Points:
			self.position.append(point.Position)

	def Symmetrize(self):
		if self.side == MIDDLE:
			XSI.LogMessage('[BuildRigElement] : Can\'t symmetrize middle elements !')
			return None

		sibling = IRSkeleton(
			None,
			self.GetSymParent(self.parent),
			self.name,
			self.mode,
			self.GetOppositeSide(),
			self.divisions,
			self.mirror,
			self.suffix
		)

		sibling.position = self.position
		sibling.rotation = self.rotation
		sibling.sibling = self
		self.sibling = sibling

		sibling.CreateGuide()

		# add symmetrize element data ICE Tree
		tree = ICETree.CreateICETree(sibling.crv, 'SymmetyrizeDatas', 2)
		node = XSI.AddICECompoundNode('IRSymetrizeSkeletonDatas', str(tree))

		XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(node))
		name = ICETree.ReplaceModelNameByThisModel(self.crv, self.model)
		XSI.SetValue('{}.Reference'.format(node), name, '')

		return sibling

	def RemoveFromRig(self):
		XSI.LogMessage('[RigSkeleton] Remove skeleton element not implemented !')

	def GetSkinCopies(self):
		skin_copies = []
		builder = self.GetBuilder()

		for geometry in builder.Parameters('GeometryList').Value.split('|'):
			copy = self.model.FindChild('{}_Copy'.format(geometry))
			if copy:
				self.skin_copies.append(copy)
			else:
				XSI.LogMessage('[RigSkeleton] Can\'t find object skin "{}"'.format(geometry))
		return skin_copies


def CreateSkeletonCurve(curves):
	if curves.Count > 1:
		XSI.SelectObj(curves)
		op = XSI.MergeCurve(curves.GetAsText(), '')
		crv = op.Parent3DObject
	else:
		crv = XSI.Duplicate(curves(0))(0)
		
	crv.Name = 'Skeleton_Crv'
	return crv


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


def DeleteSkeletonCloud(model):
	cloud = model.FindChild('ICE_Skeleton')
	if cloud:
		XSI.DeleteObj(cloud)


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
