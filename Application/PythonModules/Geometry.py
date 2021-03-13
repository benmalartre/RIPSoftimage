# -------------------------------------------------------------------
# Geometry
# -------------------------------------------------------------------
from win32com.client import constants
from Globals import XSI
from Globals import XSIFactory
import Element
import Skeleton
import Utils
import ICETree


# -------------------------------------------------------------------
# Geometry Element Class
# -------------------------------------------------------------------
class IRGeometry(Element.IRElement):
	def __init__(self, prop, parent, name):
		super(IRGeometry, self).__init__(prop, parent, name)
		self.builder = prop
		
	def AddToRig(self):
		sel = XSI.Selection
		self.list = 'GeometryList'
		self.chooser = 'GeometryChooser'
		
		sList = self.builder.Parameters(self.list).Value
		
		if sel.Count > 0:
			for s in sel:
				if s.Type == "polymsh" or s.Type == "crvlist":
					sList = self.UpdateGeometryList(s)
				else:
					XSI.LogMessage("[RigBuilder] Accepts only POLYMESH or CRVLIST ---> " + str(s) + " : Skipped!!")
					
		else:
			objs = Utils.PickMultiElement(constants.siPolyMeshFilter, 'Pick Geometries To Deform')
			for o in objs:
				sList = self.UpdateGeometryList(o)
		
		ui_items = Element.BuildListFromString(sList)
		layout = self.builder.PPGLayout
		cnt = layout.Count
		for i in range(cnt):
			if layout.Item(i).Name == self.chooser:
				layout.Item(i).UIItems = ui_items
		
	def RemoveFromRig(self):
		self.list = 'GeometryList'
		self.chooser = 'GeometryChooser'
		
		ID = self.builder.Parameters(self.chooser).Value
		geoname = self.builder.Parameters(self.chooser).value
		
		sList = self.RemoveFromList(geoname)
		
		aUIItems = Element.BuildListFromString(sList)
		layout = self.builder.PPGLayout
		cnt = layout.Count
		for i in range(cnt):
			if layout.Item(i).Name == self.chooser:
				layout.Item(i).UIItems = aUIItems 
		
	def UpdateGeometryList(self, geo):
		sList = self.builder.Parameters(self.list).Value
		if not self.ExistInList(geo.Name):
			sList += geo.Name+'|'
			self.builder.Parameters(self.list).Value = sList
		return sList
			
	def RemoveFromList(self, name):
		sList = self.builder.Parameters(self.list).Value
		sList = sList.replace(name + '|', '')
		self.builder.Parameters(self.list).Value = sList
		return sList


# -------------------------------------------------------------------
# Bind Skin To Skeleton
# -------------------------------------------------------------------
def BindSkinToSkeleton(model, skin):
	# prepare mesh
	Utils.GetWeightMap(skin, 'SmoothWeightMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'SearchMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'StretchMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'BulgeMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'RaycastMap', 0, 0, 1)
	Utils.GetWeightMap(skin, 'MuscleMap', 0, 0, 1)
	
	# get weights
	tree = ICETree.CreateICETree(skin, 'GetWeights', 0)
	compound = XSI.AddICECompoundNode('IRGetSkeletonWeights', str(tree))
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.execute'.format(compound))
	
	# no deform
	'''
	tree = tre.CreateICETree(skin,"Deform",2)
	deform = xsi.AddICECompoundNode("ICE Envelope Skeleton", str(tree))
	#deform = xsi.AddICECompoundNode("DeformBySkeleton2", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(deform)+".execute")
	'''

# -------------------------------------------------------------------
# Create Envelope Duplicate
# -------------------------------------------------------------------
def CreateEnvelopeDuplicate(model, skin):
	# check ICE skeleton
	skeleton = model.FindChild('ICE_Skeleton')
	
	# check envelope group
	env_grp = model.Groups('Envelope')

	if not skeleton:
		XSI.LogMessage('[CreateEnvelopeDuplicate] No ICE Skeleton or no Envelope Group!', constants.siWarning)
		return
	
	# check nb deformers
	num_points = skeleton.ActivePrimitive.Geometry.Points.Count
	if not env_grp or not num_points == env_grp.Members.Count:
		env_grp = Skeleton.CreateEnvelopeNullsFromSkeletonCloud(model)
		
	# duplicate mesh
	dup = Utils.GetMeshCopy(skin)
	skin.Parent3DObject.AddChild(dup)
	skin.Properties('Visibility').Parameters('ViewVis').Value = False
	
	XSI.ApplyFlexEnv('{};{}'.format(dup.FullName, env_grp.Members.GetAsText()), '', 2)

	# transfer weights
	tree = ICETree.CreateICETree(dup, 'TransferWeights', 0)
	compound = XSI.AddICECompoundNode('TransferWeightsFromICESkeleton', str(tree))
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.execute'.format(compound))
	name = ICETree.ReplaceModelNameByThisModel(skin, model)
	XSI.SetValue('{}.Reference'.format(compound), name)
	
	# deform
	tree = ICETree.CreateICETree(dup, 'Deform', 2)
	deform = XSI.AddICECompoundNode('ICE Envelope Skeleton', str(tree))
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.execute'.format(deform))
	'''
	#prepare mesh
	uti.CopyWeightMap(skin,dup,"SmoothWeightMap")
	uti.CopyWeightMap(skin,dup,"SearchMap")
	uti.CopyWeightMap(skin,dup,"StretchMap")
	uti.CopyWeightMap(skin,dup,"BulgeMap")
	uti.CopyWeightMap(skin,dup,"RaycastMap")
	uti.CopyWeightMap(skin,dup,"MuscleMap")
	'''


# -------------------------------------------------------------------
# Prepare Skin
# -------------------------------------------------------------------
def PrepareSkin(model, skin):
	# check for weight maps
	Utils.GetWeightMap(skin, 'SmoothWeightMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'SearchMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'BulgeMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'StretchMap', 1, 0, 1)

	# create one complete but not always cluster 
	# plus one weight map per rig element
	elements = Element.CollectGuideElements(model)
	for element in elements:
		crv = element.Parent3DObject
		cluster_name = '{}_Cls'.format(element.ElementName.Value)
		cluster = crv.ActivePrimitive.Geometry.Clusters(cluster_name)
		if not cluster:
			cluster = Utils.CreateCompleteButNotAlwaysCluster(skin, constants.siVertexCluster, cluster_name)
			Utils.GetWeightMap(skin, 'WeightMap', 1, 0, 1, cluster)


# -------------------------------------------------------------------
# Set SetOneElementMap
# -------------------------------------------------------------------
def SetOneElementMap(crv, obj, name):
	weight_map = Utils.GetWeightMap(obj, name)
	tree = obj.ActivePrimitive.ICETrees.Find('SetOneElement')
	if not tree:
		tree = ICETree.CreateIceTree(obj, 'SetOneElement', 1)
		
	setter = XSI.AddICECompoundNode('SetOneElementMap', str(tree))
	idx = tree.InputPorts.Count
	XSI.AddPortToICENode('{}.port{}'.format(tree, idx), 'siNodePortDataInsertionLocationAfter')
	XSI.ConnectICENodes('{}.port{}'.format(tree, idx), '{}.execute'.format(setter))
	XSI.SetValue('{}.reference'.format(setter), 'this.cls.WeightMapCls.{}'.format(name), '')
	getter = XSI.AddICENode('GetDataNode', str(tree))
	XSI.SetValue('{}.reference'.format(getter), ICETree.ReplaceModelNameByThisModel(crv), '')
	XSI.ConnectICENodes('{}.Guide_Curve'.format(setter), '{}.outname'.format(getter))
	return weight_map


# -------------------------------------------------------------------
# Gather Skin Datas On Itself
# -------------------------------------------------------------------
def GatherSkinDataOnSelf(skin):
	all_clusters = []
	model = skin.Model
	
	elements = CollectSkeletonElements(model)
	for element in elements:
		# check cluster and weight map existence
		cluster_name = '{}_Cls'.format(element.ElementName.Value)
		cluster = skin.ActivePrimitive.Geometry.Clusters(cluster_name)
		if not cluster:
			cluster = Utils.CreateCompleteButNotAlwaysCluster(skin, constants.siVertexCluster, cluster_name)
		all_clusters.append(cluster)
		wm = Utils.GetWeightMap(skin, 'WeightMap', 1, 0, 1, cluster)
	
	# create build array and set data nodes	
	tree = ICETree.CreateRigIceTree(skin, 'GatherDatas', 1)
	setter = XSI.AddICECompoundNode('Set Data', tree)
	XSI.SetValue('{}.Reference'.format(setter), 'Self.__ElementWeights', )
	build = XSI.AddICENode('BuildArrayNode', tree)
	XSI.ConnectICENodes('{}.value'.format(setter), '{}.array'.format(build))
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.execute'.format(setter))

	idx = 1
	# connect each element map
	for cluster in all_clusters:
		getter = XSI.AddICECompoundNode('GetRigElementWeight', tree)
		if idx > 1:
			XSI.AddPortToICENode(
				'{}.value{}'.format(build, idx - 1),
				'siNodePortDataInsertionLocationAfter'
			)
		XSI.SetValue('{}.reference'.format(getter), 'Self.cls.{}'.format(cluster.Name), "")
		XSI.ConnectICENodes('{}.value{}'.format(build, idx), '{}.Weight'.format(getter))
		idx += 1
	
	# check for weight maps
	Utils.GetWeightMap(skin, 'SmoothWeightMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'SearchMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'BulgeMap', 1, 0, 1)
	Utils.GetWeightMap(skin, 'StretchMap', 1, 0, 1)

	# get smooth weight map
	getter = XSI.AddICENode('GetDataNode', tree)
	XSI.SetValue('{}.reference'.format(getter), 'Self.cls.WeightMapCls.SmoothWeightMap.Weights', '')
	XSI.AddPortToICENode('{}.Value'.format(setter), 'siNodePortDataInsertionLocationAfter')
	XSI.SetValue('{}.Reference1'.format(setter), 'Self.__SmoothWeights', '')
	XSI.ConnectICENodes('{}.value1'.format(setter), '{}.value'.format(getter))
	
	# get search map
	getter = XSI.AddICENode('GetDataNode', tree)
	XSI.SetValue('{}.reference'.format(getter), 'Self.cls.WeightMapCls.SearchMap.Weights', '')
	XSI.AddPortToICENode('{}.Value1'.format(setter), 'siNodePortDataInsertionLocationAfter')
	XSI.SetValue('{}.Reference2'.format(setter), 'Self.__PerPointSearch', '')
	XSI.ConnectICENodes('{}.value2'.format(setter), '{}.value'.format(getter))
	
	# get stretch map
	getter = XSI.AddICENode('GetDataNode', tree)
	XSI.SetValue('{}.reference'.format(getter), 'Self.cls.WeightMapCls.StretchMap.Weights', '')
	XSI.AddPortToICENode('{}.Value2'.format(setter), 'siNodePortDataInsertionLocationAfter')
	XSI.SetValue('{}.Reference3'.format(setter), 'Self.__PerPointStretch', '')
	XSI.ConnectICENodes('{}.value3'.format(setter), '{}.value'.format(getter))
	
	# get bulge map
	getter = XSI.AddICENode('GetDataNode', tree)
	XSI.SetValue('{}.reference'.format(getter), 'Self.cls.WeightMapCls.BulgeMap.Weights', '')
	XSI.AddPortToICENode('{}.Value3'.format(setter), 'siNodePortDataInsertionLocationAfter')
	XSI.SetValue('{}.Reference4'.format(setter), 'Self.__PerPointBulge', '')
	XSI.ConnectICENodes('{}.value4'.format(setter), '{}.value'.format(getter))
	
	# show affected points
	affected = XSI.AddICECompoundNode('ShowRigAffectedPoints', tree)
	XSI.AddPortToICENode('{}.port1'.format(tree), 'siNodePortDataInsertionLocationAfter')
	XSI.ConnectICENodes('{}.port2'.format(tree), '{}.Execute'.format(affected))


# -------------------------------------------------------------------
# Collect Skeleton Elements
# -------------------------------------------------------------------
def CollectSkeletonElements(model):
	prop = model.Properties('RigBuilder')
	if not prop:
		return
	
	element_names = prop.Parameters('SkeletonList').Value.split('|')
	skeleton_elements = XSIFactory.CreateActiveXObject('XSI.Collection')

	for element in element_names:
		if element:
			crv = model.FindChild('{}_Crv'.format(element))
			if crv:
				elem_prop = crv.Properties("RigElement")

				if elem_prop:
					skeleton_elements.Add(elem_prop)
				else:
					XSI.LogMessage(str(crv) + " does not have RigElement Property ---> Skipped!")
		
	return skeleton_elements
