import sys
from win32com.client import constants
from Globals import XSI
from Globals import XSIMath
from Constants import *
import ICETree
import Utils

ELEMENT_SKELETON = 0
ELEMENT_MUSCLE = 1
ELEMENT_CONTROL = 2


class IRElement(object):
	""" Base class for a rig element
	:param X3DObject crv: guide curve
	:param X3DObject parent: parent 3d object
	:param str name: name of the element
	:param int side: side of the element
	:param bool mirror: mirror element (self-symmetry)
	:param str suffix: suffix
	"""
	def __init__(self, crv, parent, name, side=MIDDLE, mirror=False, suffix=""):
		# element
		self.crv = crv
		self.name = name
		self.type = None
		self.parent = parent
		self.side = side
		self.divisions = 1
		self.size = 1
		if self.crv:
			self.model = self.crv.Model
		else:
			self.model = parent.Model

		self.position = None
		self.rotation = None

		# ppg
		self.prop = None
		self.list = ""
		self.chooser = ""
		self.root = None

		# prefix
		self.prefix = Utils.GetPrefixFromSide(side)
		self.suffix = suffix

		# symmetry
		self.mirror = mirror
		self.sibling = None

		# full name
		self.fullname = self.prefix + self.name

	def CreateGuide(self):
		raise NotImplementedError('Subclass must implement abstract method')
		
	def Symmetrize(self):
		raise NotImplementedError('Subclass must implement abstract method')

	def RemoveFromRig(self):
		raise NotImplementedError('Subclass must implement abstract method')

	def ConnectSymmetry(self, elem):
		raise NotImplementedError('Subclass must implement abstract method')

	def GetSymFullName(self):
		if self.side == MIDDLE:
			return self.fullname
		elif self.side == LEFT:
			return self.fullname.replace('L_', 'R_', 1)
		elif self.side == RIGHT:
			return self.fullname.replace('R_', 'L_', 1)
		else:
			return self.fullname

	def GetSymName(self, name):
		if self.side == MIDDLE:
			return name
		elif self.side == LEFT:
			return name.replace('L_', 'R_', 1)
		elif self.side == RIGHT:
			return name.replace('R_', 'L_', 1)
		else:
			return name

	def GetOppositeSide(self):
		if self.side == MIDDLE:
			return MIDDLE
		elif self.side == LEFT:
			return RIGHT
		elif self.side == RIGHT:
			return LEFT
		else:
			return MIDDLE

	def GetSymParent(self, ref):
		parent = self.model.FindChild(self.GetSymName(ref.Name))
		if parent:
			return parent
		else:
			return ref.Parent

	def GetBuilder(self):
		return self.model.Properties('RigBuilder')

	def CheckElementExist(self):
		child = self.model.FindChild(self.fullname)
		if not child:
			return False
		return True
		
	def ExistInList(self, name):
		builder = self.GetBuilder()
		return any([(token == name) for token in builder.Parameters(self.list).Value.split('|')])

	def PickPosition(self):
		self.position = Utils.PickMultiPosition()
		self.rotation = Utils.GetOrientationFromPickPosition(self.position)
		return True

	def UpdateBuilder(self):
		builder = self.GetBuilder()
		elements = builder.Parameters(self.list).Value
		if not self.ExistInList(self.fullname):
			elements += self.fullname+'|'
		
		builder.Parameters(self.list).Value = elements
		items = BuildListFromString(elements)
		layout = builder.PPGLayout
		for i in xrange(layout.Count):
			if layout.Item(i).Name == self.chooser:
				layout.Item(i).UIItems = items
		
	def Inspect(self):
		if self.prop:
			XSI.InspectObj(self.prop, None, None, constants.siFollow)
		else:
			XSI.LogMessage('[RigElement] Can\'t find associated property !')


def CollectGuideElements(model):
	""" Collect guide element property page under model
	:param Model model: SDK Model object
	:return list: SDK PPG list
	"""
	return [prop for prop in XSI.FindObjects(None, '{76332571-D242-11d0-B69C-00AA003B3EA6}')
			if prop.Type == 'RigElement' and prop.Parent3DObject.Model.IsEqualTo(model)]


def CheckRigElementList(prop, _type):
	""" Check element list and remove invalid items
	:param PPG prop: SDK PPG object
	:param int _type: rig element type
	"""
	model = prop.Parent3DObject
	prefix = GetRigElementTypePrefix(_type)
	
	ext = '_Crv'
	if prefix == 'Muscle':
		ext = '_Crv'
	elif prefix == 'Geometry':
		ext = ''
	
	valid = []
	tokens = prop.Parameters('{}List'.format(prefix)).Value.split('|')
	for token in tokens:
		if token and model.FindChild('{}{}'.format(token, ext)):
			valid.append(token)

	prop.Parameters('{}List'.format(prefix)).Value = '|'.join(valid)


def GetRigElementTypePrefix(_type):
	""" Get rig element type prefix
	:param int _type: rig element type
	:return str: prefix for rig element based on type
	"""
	if _type == ID_SKELETON:
		return 'Skeleton'
	elif _type == ID_MUSCLE:
		return 'Muscle'
	elif _type == ID_GEOMETRY:
		return 'Geometry'
	elif _type == ID_CONTROL:
		return 'Control'
	return ''


def BuildListFromString(string):
	""" Build ppg items list from string
	:param str string: all elements name in a string separated by '|'
	:return list: ppg items list
	"""
	tokens = string.split('|')
	return [tokens[i/2] if not i % 2 else tokens[(i-1)/2] for i in xrange(2 * len(tokens))]


def ReorderCurveGroup(model, _type):
	""" Reorder curve group
	:param Model model: SDK Model object
	:param int _type: rig element type
	"""
	if _type not in [ID_SKELETON, ID_MUSCLE]:
		return

	if _type == ID_SKELETON:
		emit_ref = 'this_model.Guide_Curves'
		prop_name = 'RigElement'
		grp_name = 'Guide_Curves'

	elif _type == ID_MUSCLE:
		emit_ref = 'this_model.Muscle_Curves'
		prop_name = 'MuscleElement'
		grp_name = 'Muscle_Curves'

	# get all guide curves
	guides = [g for g in model.FindChildren2('*_Crv', '', constants.siCurveFamily, True) if g.Properties(prop_name)]

	if guides and len(guides) > 1:
		# get guide curve group
		grp = model.Groups(grp_name)

		cloud = model.AddPrimitive('PointCloud', 'ICE_Correct')
		tree = ICETree.CreateICETree(cloud, 'Emit', 0)

		emit = XSI.AddICECompoundNode('IRCorrectEmit', tree)
		XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(emit))
		XSI.SetValue('{}.Reference'.format(emit), emit_ref, None)

		# get point cloud data
		cloud_geom = cloud.ActivePrimitive.Geometry
		positions_attr = cloud_geom.GetICEAttributeFromName('PointPosition')
		positions = positions_attr.DataArray
		indices_attr = cloud_geom.GetICEAttributeFromName('ShowGuideID')
		indices = indices_attr.DataArray
		sorted_guides = [g for g in guides]

		for guide in guides:
			geom = guide.ActivePrimitive.Geometry
			closest_indices = [FindClosestIndex(p.Position, positions, indices) for p in geom.Points]
			r = max(set(closest_indices), key=closest_indices.count)
			sorted_guides[r] = guide

		if grp:
			XSI.DeleteObj(grp)

		Utils.GroupSetup(model, sorted_guides, grp_name)
		XSI.DeleteObj(cloud)


def FindClosestIndex(pnt, positions, indices):
	""" Find closest index
	:param Vector3 pnt: SDK Vector3 object
	:param Vector3Array positions: SDK PointPositionArray
	:param list indices: indices list
	:return int: closest vertex index
	"""
	delta = XSIMath.CreateVector3()
	closest_distance = sys.float_info.max
	closest_idx = 0
	for idx, p2 in enumerate(positions):
		t = XSIMath.CreateVector3(pnt.X, pnt.Y, pnt.Z)
		delta.Sub(t, p2)
		distance = delta.Length()
		if distance < closest_distance:
			closest_distance = distance
			closest_idx = idx

	return indices[closest_idx]

