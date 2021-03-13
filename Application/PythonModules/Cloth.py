from win32com.client import constants
from Globals import XSI
from Globals import XSIUIToolKit
import Utils
import ICETree


class Collider:
	def __init__(self, obj, collider, extra=False):
		self.object = obj
		self.collider = collider
		self.pntCluster = None
		self.polyCluster = None
		self.extra = extra
		
		if not extra:
			self.pntClusterName = 'Collide_{}'.format(collider.Name)
			self.polyClusterName = 'Collide_{}'.format(obj.Name)
		else:
			self.pntClusterName = 'Collide_Extra'
			self.polyClusterName = 'Collide_{}'.format(obj.Name)
		
	def GetPntCluster(self):
		self.pntCluster = self.object.ActivePrimitive.Geometry.Clusters.Find(self.pntClusterName)
		if not self.pntCluster:
			self.pntCluster = Utils.CreateCompleteButNotAlwaysCluster(
				self.object,
				constants.siVertexCluster,
				self.pntClusterName
			)
		
	def GetPolyCluster(self):
		self.polyCluster = self.collider.ActivePrimitive.Geometry.Clusters(self.polyClusterName)
		if not self.polyCluster:
			if self.extra:
				self.polyCluster = Utils.CreateAlwaysCompleteCluster(
					self.collider,
					constants.siPolygonCluster,
					self.polyClusterName
				)
			else:
				self.polyCluster = Utils.CreateCompleteButNotAlwaysCluster(
					self.collider,
					constants.siPolygonCluster,
					self.polyClusterName)
		
	def Rig(self):
		cloth_op = GetClothOperator(self.object)
		if not cloth_op:
			XSIUIToolKit.MsgBox(
				'No Syflex Operator on "{}"+"'.format(self.object),
				constants.siMsgCritical, 'ClothBuilder')
			return
		
		collide_op = XSI.ApplyOperator('syCollide', ';'.join([
			cloth_op.FullName,
			self.pntCluster.FullName,
			self.collider.ActivePrimitive.FullName,
			self.collider.Kinematics.FullName,
			self.polyCluster.FullName,
			self.object.Kinematics.FullName
		]))

		collide_op.Parameters('Reset').Value = True
		
		collide_op.Parameters('ExtEnvelope').Value = 0.05
		collide_op.Parameters('IntEnvelope').Value = -0.05
		collide_op.Parameters('Friction').Value = 0.2
		collide_op.Parameters('Damp').Value = 0.01
		
		# if this collider isn't a cloth object
		# nor extra collider object we add pre roll setup
		if not self.collider.Properties('Cloth_Rigger') and not self.extra:
			# create init ICE tree
			trees = self.collider.ActivePrimitive.ICETrees
			if not trees.Find('InitCollide'):
				tree = ICETree.CreateIceTree(self.collider, 'InitCollide', 1)
				compound = XSI.AddICECompoundNode('ClothInitTarget', str(tree))
				XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(compound))
			
			# create pre-roll ICE tree
			if not trees.Find('PreRollCollide'):
				tree = ICETree.CreateIceTree(self.collider, 'PreRollCollide', 3)
				compound = XSI.AddICECompoundNode('ClothTargetPreRoll', str(tree))
				XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(compound))


# --------------------------------------------
# Nail Class
# --------------------------------------------
class Nail:
	def __init__(self, obj, nail, cluster=None, idx=0):
		self.object = obj
		self.nail = nail
		self.cluster = cluster
		self.id = idx
		self.name = 'NailPoints {} Cluster'.format(idx)
		
	def GetPntCls(self):
		if not self.cluster:
			self.cluster = Utils.CreateCompleteButNotAlwaysCluster(
				self.object,
				constants.siVertexCluster,
				self.name)
			
		return self.cluster


# --------------------------------------------
# Get Cloth Operator
# --------------------------------------------
def GetClothOperator(obj):
	op_stack = obj.ActivePrimitive.ConstructionHistory
	for op in op_stack:
		if op.Type == 'syCloth':
			return op

	return None


# --------------------------------------------
# Is Syflex
# --------------------------------------------
def IsSyflex(obj):
	hist = obj.ActivePrimitive.ConstructionHistory
	for h in hist:
		if h.Type == 'syCloth':
			return h
	return None


# --------------------------------------------
# Find Syflex Meshes
# --------------------------------------------
def FindSyflexMeshes(model=None):
	all_syflex = XSI.FindObjects(None, '{513A555C-13F2-4D77-82E7-C35D14B789C1}')
	if model:
		return [syflex.OutputPorts(0).Target2.Parent for syflex in all_syflex
				if (model.Name == syflex.OutputPorts(0).Target2.Parent.Model.Name)]
	else:
		return [syflex.OutputPorts(0).Target2.Parent for syflex in all_syflex]


# --------------------------------------------
# Find Collider Meshes
# --------------------------------------------
def FindColliderMeshes(cloth):
	if not IsSyflex(cloth):
		return None
	all_syflex = cloth.ActivePrimitive.ConstructionHistory.Find('syCloth')
	colliders = []
	for syflex in all_syflex.NestedObjects:
		if syflex.Type == 'syCollide':
			mesh = syflex.InputPorts(2).Target2.Parent3DObject
			colliders.append(mesh)
	return colliders


# --------------------------------------------
# Find Pattern Meshes
# --------------------------------------------
def FindPatternMeshes(model):	
	objects = model.FindChildren('', '', constants.si3DObjectFamily, True)
	return [obj for obj in objects if obj.Properties('Cloth_Rigger')]

