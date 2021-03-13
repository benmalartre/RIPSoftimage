# -------------------------------------------------------------------
# ICETree
# -------------------------------------------------------------------
from Globals import XSI

# -------------------------------------------------------------------
# Merged Rig Cloud
# -------------------------------------------------------------------
def MergeRigCloud(model, clouds, name):
	cloud = model.AddPrimitive("PointCloud", name)
	tree = XSI.ApplyOp("ICETree", cloud, "siNode", "", "", 2)
	idx = 1
	for c in clouds:
		merge = XSI.AddICECompoundNode("MergeRigPointCloud", str(tree))
		XSI.AddPortToICENode(str(tree) + ".port" + str(idx), "siNodePortDataInsertionLocationAfter")
		XSI.ConnectICENodes(str(tree) + ".port" + str(idx), str(merge) + ".execute")
		get = XSI.AddICENode("GetDataNode", str(tree))
		XSI.SetValue(str(get) + ".reference", ReplaceModelNameByThisModel(c), "")
		XSI.ConnectICENodes(str(merge) + ".Cloud", str(get) + ".outname")
		idx += 1


# -------------------------------------------------------------------
# Replace Model Name by this_model
# -------------------------------------------------------------------
def ReplaceModelNameByThisModel(obj, inmodel=None):
	if inmodel == None:
		inmodel = XSI.ActiveSceneRoot
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
	tree = XSI.ApplyOp("ICETree", cloud, "siNode", "", "", constructionhistory)
	ICETree = XSI.Dictionary.GetObject(tree)
	ICETree.Name = name
	return ICETree

def CreateSimulatedICETree(cloud, name="ICETree"):
	tree = XSI.CreateSimulatedICETree(cloud);
	ICETree = XSI.Dictionary.GetObject(tree)
	ICETree.Name = name
	return ICETree

