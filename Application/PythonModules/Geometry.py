# -------------------------------------------------------------------
# Geometry
# -------------------------------------------------------------------
from win32com.client import constants as siConstants
from Globals import xsi
from Globals import XSIFactory
import Element as ele
import Skeleton as ske
import Utils as uti
import ICETree as tre


# -------------------------------------------------------------------
# Geometry Element Class
# -------------------------------------------------------------------
class IRGeometry(ele.IRElement):
	def __init__(self, prop, parent):
		super(IRGeometry, self).__init__(prop, parent)

	def AddToRig(self):
		sel = xsi.Selection
		self.list = "GeometryList"
		self.chooser = "GeometryChooser"
		
		sList = self.builder.Parameters(self.list).Value
		
		if sel.Count>0:
			for s in sel:
				if s.Type == "polymsh" or s.Type == "crvlist":
					sList = self.UpdateGeometryList(s)
				else:
					xsi.LogMessage("[RigBuilder] Accepts only POLYMESH or CRVLIST ---> "+str(s)+" : Skipped!!")
					
		else:
			objs = uti.PickMultiElement(siConstants.siPolyMeshFilter, "Pick Geometries To Deform")
			for o in objs:
				sList = self.UpdateGeometryList(o)
		
		aUIItems = ele.BuildListFromString(sList)
		layout = self.builder.PPGLayout
		cnt = layout.Count
		for i in range(cnt):
			if layout.Item(i).Name == self.chooser:
				layout.Item(i).UIItems = aUIItems 
		
	def RemoveFromRig(self):
		self.list = "GeometryList"
		self.chooser = "GeometryChooser"
		
		ID = self.builder.Parameters(self.chooser).Value
		geoname = self.builder.Parameters(self.chooser).value
		
		sList = self.RemoveFromList(geoname)
		
		aUIItems = ele.BuildListFromString(sList)
		layout = self.builder.PPGLayout
		cnt = layout.Count
		for i in range(cnt):
			if layout.Item(i).Name == self.chooser:
				layout.Item(i).UIItems = aUIItems 
		
	def UpdateGeometryList(self,geo):
		sList = self.builder.Parameters(self.list).Value
		if not self.ExistInList(geo.Name):
			sList+=geo.Name+"|"
			self.builder.Parameters(self.list).Value = sList
		return sList
			
	def RemoveFromList(self,geoname):
		sList = self.builder.Parameters(self.list).Value
		sList = sList.replace(geoname+"|","")
		self.builder.Parameters(self.list).Value = sList
		return sList


# -------------------------------------------------------------------
# Bind Skin To Skeleton
# -------------------------------------------------------------------
def BindSkinToSkeleton(model, skin):
	
	#prepare mesh
	uti.GetWeightMap(skin, "SmoothWeightMap", 1, 0, 1)
	uti.GetWeightMap(skin, "SearchMap", 1, 0, 1)
	uti.GetWeightMap(skin, "StretchMap", 1, 0, 1)
	uti.GetWeightMap(skin, "BulgeMap", 1, 0, 1)
	uti.GetWeightMap(skin, "RaycastMap", 0, 0, 1)
	uti.GetWeightMap(skin, "MuscleMap", 0, 0, 1)
	
	# get weights
	tree = tre.CreateICETree(skin, "GetWeights", 0)
	weights = xsi.AddICECompoundNode("IRGetSkeletonWeights", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(weights)+".execute")
	
	# NO deform
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
	skel = model.FindChild("ICE_Skeleton")
	
	# check envelope group
	env_grp = model.Groups("Envelope")

	if not skel:
		xsi.LogMessage("[CreateEnvelopeDuplicate] No ICE Skeleton or no Envelope Group!", siConstants.siWarning)
		return
	
	# check nb deformers
	nbp = skel.ActivePrimitive.Geometry.Points.Count
	if not env_grp or not nbp == env_grp.Members.Count:
		env_grp = ske.CreateEnvelopeNullsFromSkeletonCloud(model)
		
	# duplicate mesh
	dup = uti.GetMeshCopy(skin)
	skin.Parent3DObject.AddChild(dup)
	skin.Properties("Visibility").Parameters("ViewVis").Value = False
	
	xsi.ApplyFlexEnv(dup.FullName+";"+env_grp.Members.GetAsText(),"",2)

	# transfer weights
	tree = tre.CreateICETree(dup,"TransfertWeights",0)
	weights = xsi.AddICECompoundNode("TransferWeightsFromICESkeleton", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(weights)+".execute")
	name = tre.ReplaceModelNameByThisModel(skin,model)
	xsi.SetValue(str(weights)+".Reference",name)
	
	# deform
	tree = tre.CreateICETree(dup,"Deform",2)
	deform = xsi.AddICECompoundNode("ICE Envelope Skeleton", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(deform)+".execute")
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
def PrepareSkin(model,skin):
	#check for weight maps
	smoothweightmap = uti.GetWeightMap(skin, "SmoothWeightMap", 1, 0, 1)
	searchmap = uti.GetWeightMap(skin, "SearchMap", 1, 0, 1)
	bulgemap = uti.GetWeightMap(skin, "BulgeMap", 1, 0, 1)
	stretchmap = uti.GetWeightMap(skin, "StretchMap", 1, 0, 1)

	# create one complete but not always cluster 
	# plus one weightmap per rig element	
	elems = ele.CollectGuideElements(model)
	for e in elems:
		crv = e.Parent3DObject
		clsname = e.ElementName.Value+"_Cls"
		cls = crv.ActivePrimitive.Geometry.Clusters(clsname)
		if not cls:
			cls = uti.CreateCompleteButNotAlwaysCluster(skin, siConstants.siVertexCluster, clsname)
			wm = uti.GetWeightMap(skin, "WeightMap", 1, 0, 1, cls)


# -------------------------------------------------------------------
# Set SetOneElementMap
# -------------------------------------------------------------------
def SetOneElementMap(crv, inObj, name):
	wMap = uti.GetWeightMap(inObj, name)
	tree = inObj.ActivePrimitive.ICETrees.Find("SetOneElement")
	if not tree:
		tree = tre.CreateIceTree(inObj, "SetOneElement", 1)
		
	setter = xsi.AddICECompoundNode("SetOneElementMap", str(tree))
	idx = tree.InputPorts.Count
	xsi.AddPortToICENode(str(tree)+".port"+str(idx), "siNodePortDataInsertionLocationAfter")
	xsi.ConnectICENodes(str(tree)+".port"+str(idx), str(setter)+".execute")
	xsi.SetValue(str(setter)+".reference", "this.cls.WeightMapCls."+name, "")
	getter = xsi.AddICENode("GetDataNode", str(tree))
	xsi.SetValue(str(getter)+".reference", tre.ReplaceModelNameByThisModel(crv), "")
	xsi.ConnectICENodes(str(setter)+".Guide_Curve", str(getter)+".outname")
	return wMap


# -------------------------------------------------------------------
# Gather Skin Datas On Another Geometry
# -------------------------------------------------------------------
def GatherSkinDataOnOther(skin, copy):
	elementmaps = []
	model = copy.Model
	
	elems = CollectSkeletonElements(model)
	for e in elems:
		wMap =uti.GetWeightMap(copy,e.ElementName.Value+"Map")
		elementmaps.append(wMap)
	
	# create build array and set data nodes	
	tree = tre.CreateRigIceTree(skin,"GatherDatas",1)
	setter = xsi.AddICECompoundNode("Set Data", tree)
	xsi.SetValue(str(setter)+".Reference", "Self.__ElementWeights", "")
	build = xsi.AddICENode("BuildArrayNode", tree)
	xsi.ConnectICENodes(str(setter)+".value", str(build)+".array")
	xsi.ConnectICENodes(str(tree)+".port1", str(setter)+".execute")

	# create get closest location
	closest = xsi.AddICENode("GetClosestLocationNode", tree)
	get1 = xsi.AddICENode("GetDataNode", tree)
	get2 = xsi.AddICENode("GetDataNode", tree)
	xsi.SetValue(str(get1)+".Reference", copy.FullName, "")
	xsi.SetValue(str(get2)+".Reference", "Self.PointPosition", "")
	xsi.ConnectICENodes(str(closest)+".geometry", str(get1)+".value")
	xsi.ConnectICENodes(str(closest)+".position", str(get2)+".value")
	
	# create point index to location
	idtoloc = xsi.AddICENode("PointIndexToLocationNode", tree)
	elem = xsi.AddICENode("GetElementIndicesNode", tree)
	xsi.ConnectICENodes(str(idtoloc)+".geometry", str(get1)+".value")
	xsi.ConnectICENodes(str(elem)+".value", str(get2)+".value")
	xsi.ConnectICENodes(str(idtoloc)+".index", str(elem)+".indices")
	
	# create switch
	switch = xsi.AddICENode("IfNode", tree)
	xsi.ConnectICENodes(str(switch)+".iftrue", str(closest)+".location")
	xsi.ConnectICENodes(str(switch)+".iffalse", str(idtoloc)+".result")
	
	idx = 1
	# connect each element map
	for m in elementmaps:
		get = xsi.AddICENode("GetDataNode", tree)
		if idx > 1:
			xsi.AddPortToICENode(str(build)+".value"+str(idx-1), "siNodePortDataInsertionLocationAfter")
		xsi.ConnectICENodes(str(get)+".source", str(switch)+".result")
		xsi.SetValue(str(get)+".reference", "cls.WeightMapCls."+m.Name+".Weights", "")
		xsi.ConnectICENodes(str(build)+".value"+str(idx), str(get)+".value")
		idx += 1
	
	id = 1
	# get side map
	get = xsi.AddICENode("GetDataNode", tree)
	xsi.AddPortToICENode(str(tree)+".port"+str(id), "siNodePortDataInsertionLocationAfter")
	xsi.ConnectICENodes(str(get)+".source", str(switch)+".result" )
	xsi.SetValue(str(get)+".reference", "cls.WeightMapCls.SideMap.Weights", "")
	xsi.AddPortToICENode(str(set)+".Value", "siNodePortDataInsertionLocationAfter")
	xsi.SetValue(str(set)+".Reference1", "Self.__PerPointSide", "")
	xsi.ConnectICENodes(str(set)+".value1", str(get)+".value")
	
	# get smooth weight map
	get = xsi.AddICENode("GetDataNode", tree)
	xsi.AddPortToICENode(str(tree)+".port"+str(id), "siNodePortDataInsertionLocationAfter")
	xsi.ConnectICENodes(str(get)+".source", str(switch)+".result" )
	xsi.SetValue(str(get)+".reference", "cls.WeightMapCls.SmoothWeightMap.Weights", "")
	xsi.AddPortToICENode(str(set)+".Value1", "siNodePortDataInsertionLocationAfter")
	xsi.SetValue(str(set)+".Reference2", "Self.__SmoothWeights", "")
	xsi.ConnectICENodes(str(set)+".value2", str(get)+".value")
	
	# get search map
	get = xsi.AddICENode("GetDataNode", tree)
	xsi.AddPortToICENode(str(tree)+".port"+str(id), "siNodePortDataInsertionLocationAfter")
	xsi.ConnectICENodes(str(get)+".source", str(switch)+".result" )
	xsi.SetValue(str(get)+".reference", "cls.WeightMapCls.SearchMap.Weights", "")
	xsi.AddPortToICENode(str(set)+".Value2", "siNodePortDataInsertionLocationAfter")
	xsi.SetValue(str(set)+".Reference3", "Self.__PerPointSearch", "")
	xsi.ConnectICENodes(str(set)+".value3", str(get)+".value")


# -------------------------------------------------------------------
# Gather Skin Datas On Itself
# -------------------------------------------------------------------
def GatherSkinDataOnSelf(skin):
	cls = []
	model = skin.Model
	
	elems = CollectSkeletonElements(model)
	for e in elems:
		# check cluster and weightmap existence
		clsname = e.ElementName.Value+"_Cls"
		c = skin.ActivePrimitive.Geometry.Clusters(clsname)
		if not c:
			c = uti.CreateCompleteButNotAlwaysCluster(skin, siConstants.siVertexCluster,clsname)
		cls.append(c)
		wm = uti.GetWeightMap(skin,"WeightMap",1,0,1,c)
	
	# create build array and set data nodes	
	tree = tre.CreateRigIceTree(skin,"GatherDatas",1)
	setter = xsi.AddICECompoundNode("Set Data", tree)
	xsi.SetValue(str(setter)+".Reference", "Self.__ElementWeights", "")
	build = xsi.AddICENode("BuildArrayNode", tree)
	xsi.ConnectICENodes(str(setter)+".value", str(build)+".array")
	xsi.ConnectICENodes(str(tree)+".port1", str(setter)+".execute")

	idx = 1
	# connect each element map
	for c in cls:
		getter = xsi.AddICECompoundNode("GetRigElementWeight", tree)
		if idx > 1:
			xsi.AddPortToICENode(str(build)+".value"+str(idx-1), "siNodePortDataInsertionLocationAfter")
		xsi.SetValue(str(getter)+".reference", "Self.cls."+c.Name, "")
		xsi.ConnectICENodes(str(build)+".value"+str(idx), str(getter)+".Weight")
		idx += 1
	
	#check for weight maps
	smoothweightmap = uti.GetWeightMap(skin, "SmoothWeightMap", 1, 0, 1)
	searchmap = uti.GetWeightMap(skin, "SearchMap", 1, 0, 1)
	bulgemap = uti.GetWeightMap(skin, "BulgeMap", 1, 0, 1)
	stretchmap = uti.GetWeightMap(skin, "StretchMap", 1, 0, 1)
	
	idx=1
	# get smooth weight map
	getter = xsi.AddICENode("GetDataNode", tree)
	xsi.SetValue(str(getter)+".reference", "Self.cls.WeightMapCls.SmoothWeightMap.Weights", "")
	xsi.AddPortToICENode(str(setter)+".Value", "siNodePortDataInsertionLocationAfter")
	xsi.SetValue(str(setter)+".Reference1", "Self.__SmoothWeights", "")
	xsi.ConnectICENodes(str(setter)+".value1", str(getter)+".value")
	
	# get search map
	getter = xsi.AddICENode("GetDataNode", tree)
	xsi.SetValue(str(getter)+".reference", "Self.cls.WeightMapCls.SearchMap.Weights", "")
	xsi.AddPortToICENode(str(setter)+".Value1", "siNodePortDataInsertionLocationAfter")
	xsi.SetValue(str(setter)+".Reference2", "Self.__PerPointSearch", "")
	xsi.ConnectICENodes(str(setter)+".value2", str(getter)+".value")
	
	# get stretch map
	getter = xsi.AddICENode("GetDataNode", tree)
	xsi.SetValue(str(getter)+".reference", "Self.cls.WeightMapCls.StretchMap.Weights", "")
	xsi.AddPortToICENode(str(setter)+".Value2", "siNodePortDataInsertionLocationAfter")
	xsi.SetValue(str(setter)+".Reference3", "Self.__PerPointStretch", "")
	xsi.ConnectICENodes(str(setter)+".value3", str(getter)+".value")
	
	# get bulge map
	getter = xsi.AddICENode("GetDataNode", tree)
	xsi.SetValue(str(getter)+".reference", "Self.cls.WeightMapCls.BulgeMap.Weights", "")
	xsi.AddPortToICENode(str(setter)+".Value3", "siNodePortDataInsertionLocationAfter")
	xsi.SetValue(str(setter)+".Reference4", "Self.__PerPointBulge", "")
	xsi.ConnectICENodes(str(setter)+".value4", str(getter)+".value")
	
	# show affected points
	affected = xsi.AddICECompoundNode("ShowRigAffectedPoints", tree)
	xsi.AddPortToICENode(str(tree)+".port1", "siNodePortDataInsertionLocationAfter")
	xsi.ConnectICENodes(str(tree)+".port2", str(affected)+".Execute")


# -------------------------------------------------------------------
# Collect Skeleton Elements
# -------------------------------------------------------------------
def CollectSkeletonElements(model):
	prop = model.Properties("RigBuilder")
	if not prop:
		return
	
	sList = prop.Parameters("SkeletonList").Value
	elemsname = sList.split("|")
	elemsname.pop()
	
	elems = XSIFactory.CreateActiveXObject("XSI.Collection")
	#props = xsi.FindObjects(None,"{76332571-D242-11d0-B69C-00AA003B3EA6}")
	
	for e in elemsname:
		crv = model.FindChild(e+"_Crv")
		p = crv.Properties("RigElement")
		
		if p:
			elems.Add(p)
		else:
			xsi.LogMessage(str(crv)+" does not have RigElement Property ---> Skipped!")
		
	return elems
