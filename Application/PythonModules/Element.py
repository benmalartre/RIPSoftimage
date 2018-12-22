# -------------------------------------------------------------------
# Element
# -------------------------------------------------------------------
import sys
from win32com.client import constants as siConstants
from Globals import xsi
from Globals import XSIMath
from Globals import XSIFactory
from Constants import *
import ICETree
import Element
import Utils

ELEMENT_SKELETON = 0
ELEMENT_MUSCLE = 1
ELEMENT_CONTROL = 2

# ---------------------------------------------------
# Base RigElement Class
# ---------------------------------------------------
class IRElement(object):
	def __init__(self, crv, parent, name, side=MIDDLE, selfsymmetrize=False, suffix=""):
		# element
		self.curvetype = 0
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

		# symmetry
		self.selfsymmetrize = selfsymmetrize
		self.symmelement = None

		self.position = None
		self.rotation = None

		self.list = ""
		self.chooser = ""
		self.root = None

		# arrays to store guide elements
		self.position = []

		# prefix
		self.prefix = "M_"
		if self.side == LEFT:
			self.prefix = "L_"
		elif self.side == RIGHT:
			self.prefix = "R_"

		self.suffix = suffix

		# ppg
		self.prop = None

		# full name
		self.fullname = self.prefix + self.name

	def CreateGuide(self):
		raise NotImplementedError("Subclass must implement abstract method")
		
	def Symmetrize(self):
		raise NotImplementedError("Subclass must implement abstract method")

	def RemoveFromRig(self):
		raise NotImplementedError("Subclass must implement abstract method")

	def ConnectSymmetry(self, elem):
		raise NotImplementedError("Subclass must implement abstract method")

	def GetSymFullName(self):
		if self.side == MIDDLE:
			return self.fullname
		elif self.side == LEFT:
			return self.fullname.replace("L_", "R_", 1)
		elif self.side == RIGHT:
			return self.fullname.replace("R_", "L_", 1)
		else:
			return self.fullname

	def GetSymName(self, name):
		if self.side == MIDDLE:
			return name
		elif self.side == LEFT:
			return name.replace("L_", "R_", 1)
		elif self.side == RIGHT:
			return name.replace("R_", "L_", 1)
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
		return self.model.Properties("RigBuilder")

	def CheckElementExist(self):
		child = self.model.FindChild(self.fullname)
		if not child:
			return False
		return True
		
	def ExistInList(self, name):
		builder = self.GetBuilder()
		sList = builder.Parameters(self.list).value
		split = sList.split("|")
		split.pop()
		return any([s == name for s in split])

	def PickPosition(self):
		self.position = Utils.PickMultiPosition()
		self.rotation = Utils.GetOrientationFromPickPosition(self.position)
		return True

	def UpdateBuilder(self):
		builder = self.GetBuilder()
		elements = builder.Parameters(self.list).Value
		if not self.ExistInList(self.fullname):
			elements += self.fullname+"|"
		
		builder.Parameters(self.list).Value = elements
		items = BuildListFromString(elements)
		layout = builder.PPGLayout
		cnt = layout.Count
		for c in range(cnt):
			if layout.Item(c).Name == self.chooser:
				layout.Item(c).UIItems = items
		
	def Inspect(self):
		if self.prop:
			xsi.InspectObj(self.prop, None, None, siConstants.siFollow)
		else:
			xsi.LogMessage("[RigElement] Can't find associated property --> Inspection aborted...")


# ---------------------------------------------------
# Collect Guide Elements
# ---------------------------------------------------
def CollectGuideElements(model):
	xsi.LogMessage("Collect Guide Element")
	elems = XSIFactory.CreateActiveXObject("XSI.Collection")
	props = xsi.FindObjects(None, "{76332571-D242-11d0-B69C-00AA003B3EA6}")
	for p in props:
		if p.Type == "RigElement":
			if p.Parent3DObject.Model.IsEqualTo(model):
				elems.Add(p)
				
	return elems


# ---------------------------------------------------
# Check Element List
# ---------------------------------------------------
def CheckRigElementList(prop, elem_type):
	model = prop.Parent3DObject
	prefix = GetRigElementTypePrefix(elem_type)
	
	ext = "_Crv"
	if prefix == "Muscle":
		ext = "_Crv"
	elif prefix == "Geometry":
		ext = ""
	
	outlist = ""
	items = prop.Parameters(prefix+"List").Value
	split = items.split("|")
	split.pop()
	
	for s in split:
		if model.FindChild(s+ext):
			outlist += s+"|"
		else:
			xsi.LogMessage("[RigElement] "+s+ext+" was not found under "+str(model)+" : Removed from list!!")

	prop.Parameters(prefix+"List").Value = outlist


# ---------------------------------------------------
# Get Element Type Prefix
# ---------------------------------------------------
def GetRigElementTypePrefix(elem_type):
	if elem_type == ID_SKELETON:
		return "Skeleton"
	elif elem_type == ID_MUSCLE:
		return "Muscle"
	elif elem_type == ID_GEOMETRY:
		return "Geometry"
	elif elem_type == ID_CONTROL:
		return "Control"
	return ""


# ---------------------------------------------------
# Build UIItems List from String
# ---------------------------------------------------
def BuildListFromString(in_string):
	in_array = in_string.split("|")
	out_array = []
	for a in range(len(in_array)-1):
		out_array.append(str(in_array[a]))
		out_array.append(str(in_array[a]))
	return out_array


# ---------------------------------------------------
# Reorder Curve Group
# ---------------------------------------------------
def ReorderCurveGroup(model, crv_type):
	if crv_type not in [ID_SKELETON, ID_MUSCLE]:
		return

	if crv_type == ID_SKELETON:
		emit_ref = "this_model.Guide_Curves"
		prop_name = "RigElement"
		grp_name = "Guide_Curves"

	elif crv_type == ID_MUSCLE:
		emit_ref = "this_model.Muscle_Curves"
		prop_name = "MuscleElement"
		grp_name = "Muscle_Curves"

	# get all guide curves
	guides = [g for g in model.FindChildren2("*_Crv", "", siConstants.siCurveFamily, True) if
			  g.Properties(prop_name)]

	if guides and len(guides) > 1:
		# get guide curve group
		grp = model.Groups(grp_name)

		cloud = model.AddPrimitive("PointCloud", "ICE_Correct")
		tree = ICETree.CreateICETree(cloud, "Emit", 0)

		emit = xsi.AddICECompoundNode("IRCorrectEmit", tree)
		xsi.ConnectICENodes(str(tree) + ".port1", str(emit) + ".Execute")
		xsi.SetValue(str(emit) + ".Reference", emit_ref, None)

		# get point cloud data
		cloud_geom = cloud.ActivePrimitive.Geometry
		positions_attr = cloud_geom.GetICEAttributeFromName("PointPosition")
		positions = positions_attr.DataArray
		indices_attr = cloud_geom.GetICEAttributeFromName("ShowGuideID")
		indices = indices_attr.DataArray


		sorted_guides = [g for g in guides]

		for guide in guides:
			geom = guide.ActivePrimitive.Geometry
			closests = [FindClosestIndex(p.Position, positions, indices) for p in geom.Points]
			r = Element.FindRedundant(closests)
			sorted_guides[r] = guide

		if grp:
			xsi.DeleteObj(grp)

		Utils.GroupSetup(model, sorted_guides, grp_name)
		xsi.DeleteObj(cloud)


# ---------------------------------------------------
# Find Closest Index
# ---------------------------------------------------
def FindClosestIndex(pnt, positions, indices):
	delta = XSIMath.CreateVector3()
	closestDistance = sys.float_info.max
	closestIdx = 0
	for idx, p2 in enumerate(positions):
		t = XSIMath.CreateVector3(pnt.X, pnt.Y, pnt.Z)
		delta.Sub(t, p2)
		distance = delta.Length()
		if distance < closestDistance:
			closestDistance = distance
			closestIdx = idx

	return indices[closestIdx]


# ---------------------------------------------------
# Find Redundant Index (best match)
# ---------------------------------------------------
def FindRedundant(lst):
	return max(set(lst), key=lst.count)
