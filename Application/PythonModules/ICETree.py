# -------------------------------------------------------------------
# ICETree
# -------------------------------------------------------------------
from Globals import xsi

# -------------------------------------------------------------------
# Merged Rig Cloud
# -------------------------------------------------------------------
def MergeRigCloud(model, clouds, name):
	cloud = model.AddPrimitive("PointCloud", name)
	tree = xsi.ApplyOp("ICETree", cloud, "siNode", "", "", 2)
	idx = 1
	for c in clouds:
		merge = xsi.AddICECompoundNode("MergeRigPointCloud", str(tree))
		xsi.AddPortToICENode(str(tree)+".port"+str(idx), "siNodePortDataInsertionLocationAfter")
		xsi.ConnectICENodes(str(tree)+".port"+str(idx), str(merge)+".execute")
		get = xsi.AddICENode("GetDataNode", str(tree))
		xsi.SetValue(str(get)+".reference", ReplaceModelNameByThisModel(c), "")
		xsi.ConnectICENodes(str(merge)+".Cloud", str(get)+".outname")
		idx += 1


# -------------------------------------------------------------------
# Replace Model Name by this_model
# -------------------------------------------------------------------
def ReplaceModelNameByThisModel(obj, inmodel=None):
	if inmodel == None:
		inmodel = xsi.ActiveSceneRoot
	fullname = obj.FullName
	model = obj.Model
	if model.FullName == inmodel.FullName:
		return fullname.replace(model.Name, "this_model", 1)
	else:
		return fullname


# -------------------------------------------------------------------
# Get ICE Tree By Name
# -------------------------------------------------------------------
def GetICETreeByName(obj, name):
	tree = obj.ActivePrimitive.ICETrees.Find(name)
	return tree


# -------------------------------------------------------------------
# Replace Object Name by this
# -------------------------------------------------------------------
def ReplaceObjectNameByThis(obj):
	fullname = obj.FullName
	parentobj = obj.Parent3DObject
	return fullname.replace(parentobj.FullName, "this", 1)


# -------------------------------------------------------------------
# Get Object From Reference Value
# -------------------------------------------------------------------
def GetObjectFromReferenceValue(node):
	targetname = node.Parameters("Reference").Value
	model = node.Parent3DObject.Model
	targetname = targetname.replace("this_model.", "")
	targetname = targetname.replace(model.Name+".", "")
	target = model.FindChild(targetname)
	
	return target


# -------------------------------------------------------------------
# Create Empty ICETree
# -------------------------------------------------------------------
def CreateICETree(cloud, name="ICETree", constructionhistory=0):
	tree = xsi.ApplyOp("ICETree", cloud, "siNode", "", "", constructionhistory)
	ICETree = xsi.Dictionary.GetObject(tree)
	ICETree.Name = name
	return ICETree

def CreateSimulatedICETree(cloud, name="ICETree"):
	tree = xsi.CreateSimulatedICETree(cloud);
	ICETree = xsi.Dictionary.GetObject(tree)
	ICETree.Name = name
	return ICETree

