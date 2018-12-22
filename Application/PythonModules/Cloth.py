# -------------------------------------------------------------------
# Cloth
# -------------------------------------------------------------------
from win32com.client import constants as siConstants
from Globals import xsi
from Globals import XSIUIToolKit
import Utils as uti
import ICETree as ice


# --------------------------------------------
# Collider Class
# --------------------------------------------
class Collider:
	def __init__(self, obj, collider, extra=False):
		self.object = obj
		self.collider = collider
		self.pntcls = None
		self.polycls = None
		self.extra = extra
		
		if not extra:
			self.pntclsname = "Collide_"+collider.Name
			self.polyclsname = "Collide_"+obj.Name
		else:
			self.pntclsname = "Collide_Extra"
			self.polyclsname = "Collide_"+obj.Name
		
	def GetPntCls(self, prop):
		self.pntcls = self.object.ActivePrimitive.Geometry.Clusters.Find(self.pntclsname)
		if not self.pntcls:
			self.pntcls = uti.CreateCompleteButNotAlwaysCluster(self.object,
																siConstants.siVertexCluster,
																self.pntclsname)
		
	def GetPolyCls(self, prop):
		self.polycls = self.collider.ActivePrimitive.Geometry.Clusters(self.polyclsname)
		if not self.polycls:
			if self.extra:
				self.polycls = uti.CreateAlwaysCompleteCluster(self.collider,
															   siConstants.siPolygonCluster,
															   self.polyclsname)
			else:
				self.polycls = uti.CreateCompleteButNotAlwaysCluster(self.collider,
																	 siConstants.siPolygonCluster,
																	 self.polyclsname)
		
	def Rig(self):
		clothop = GetClothOperator(self.object)
		if not clothop:
			XSIUIToolKit.MsgBox("No Syflex Operator on "+str(self.object)+"\n Fuck You!",
								siConstants.siMsgCritical, "ClothBuilder")
			return
		
		collideop = xsi.ApplyOperator("syCollide", clothop.FullName +";"+
													 self.pntcls.FullName +";"+
													 self.collider.ActivePrimitive.FullName +";"+
													 self.collider.Kinematics.FullName+";"+
													 self.polycls.FullName +";"+
													 self.object.Kinematics.FullName)
		collideop.Parameters("Reset").Value = True
		
		collideop.Parameters("ExtEnvelope").Value = 0.05
		collideop.Parameters("IntEnvelope").Value = -0.05
		collideop.Parameters("Friction").Value = 0.2
		collideop.Parameters("Damp").Value = 0.01
		
		# if this collider isn't a cloth object
		# nor extra collider object we add pre roll setup
		if not self.collider.Properties("Cloth_Rigger") and not self.extra:
			#Create Init ICE Tree
			trees = self.collider.ActivePrimitive.ICETrees
			if not trees.Find("InitCollide"):
				tree = ice.CreateIceTree(self.collider, "InitCollide",1)
				compound = xsi.AddICECompoundNode("ClothInitTarget", str(tree))
				xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")
			
			#Create PreRoll ICE Tree
			if not trees.Find("PreRollCollide"):
				tree = ice.CreateIceTree(self.collider, "PreRollCollide",3)
				compound = xsi.AddICECompoundNode("ClothTargetPreRoll", str(tree))
				xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")


# --------------------------------------------
# Nail Class
# --------------------------------------------
class Nail:
	def __init__(self, obj, nail, pntcls=None, id=0):
		self.object = obj
		self.nail = nail
		self.pntcls = pntcls
		self.id = id
		
		self.pntclsname = "NailPoints"+str(id)+"Cls"
		
	def GetPntCls(self):
		if self.pntcls == None:
			self.pntcls = uti.CreateCompleteButNotAlwaysCluster(self.object,
																siConstants.siVertexCluster,
																self.pntclsname)
			
		return self.pntcls


# --------------------------------------------
# Get Cloth Operator
# --------------------------------------------
def GetClothOperator(obj):
	opstack = obj.ActivePrimitive.ConstructionHistory
	for op in opstack:
		if op.Type == "syCloth":
			return op

	return None

# --------------------------------------------
# Is Syflex
# --------------------------------------------
def IsSyflex(inObj):
	hist = inObj.ActivePrimitive.ConstructionHistory
	for h in hist:
		if h.Type == "syCloth":
			return h
	return None


# --------------------------------------------
# Find Syflex Meshes
# --------------------------------------------
def FindSyflexMeshes(model=None):
	syflex  = xsi.FindObjects(None, "{513A555C-13F2-4D77-82E7-C35D14B789C1}")
	meshes = []
	for s in syflex:
		m = s.OutputPorts(0).Target2.Parent
		if not model:
			meshes.Add(m)
		else:
			if model.Name == m.Model.Name:
				meshes.append(m)
	return meshes


# --------------------------------------------
# Find Collider Meshes
# --------------------------------------------
def FindColliderMeshes(cloth):
	if not IsSyflex(cloth):
		return None
	syflex  = cloth.ActivePrimitive.ConstructionHistory.Find("syCloth")
	colliders = []
	for s in syflex.NestedObjects:
		if s.Type == "syCollide":
			m = s.InputPorts(2).Target2.Parent3DObject
			colliders.append(m)
	return colliders


# --------------------------------------------
# Find Pattern Meshes
# --------------------------------------------
def FindPatternMeshes(model):	
	objects = model.FindChildren("", "", siConstants.si3DObjectFamily, True)
	patterns = []
	for o in objects:
		if o.Properties("Cloth_Rigger"):
			patterns.append(o)
			
	return patterns
