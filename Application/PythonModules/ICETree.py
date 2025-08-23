from Globals import XSI


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


def ReplaceModelNameByThisModel(obj, model=None):
	fullname = obj.FullName
	if not model:
		model = obj.Model

	if model.FullName == model.FullName:
		return fullname.replace(model.Name, "this_model", 1)
	else:
		return fullname


def GetICETreeByName(obj, name):
	tree = obj.ActivePrimitive.ICETrees.Find(name)
	return tree


def ReplaceObjectNameByThis(obj):
	fullname = obj.FullName
	parent_obj = obj.Parent3DObject
	return fullname.replace(parent_obj.FullName, "this", 1)


def GetObjectFromReferenceValue(node):
	target_name = node.Parameters("Reference").Value
	model = node.Parent3DObject.Model
	target_name = target_name.replace("this_model.", "")
	target_name = target_name.replace(model.Name+".", "")
	target = model.FindChild(target_name)
	
	return target


def CreateICETree(cloud, name="ICETree", construction_history=0):
	tree = XSI.ApplyOp("ICETree", cloud, "siNode", "", "", construction_history)
	ICETree = XSI.Dictionary.GetObject(tree)
	ICETree.Name = name
	return ICETree


def CreateSimulatedICETree(cloud, name="ICETree"):
	tree = XSI.CreateSimulatedICETree(cloud)
	ICETree = XSI.Dictionary.GetObject(tree)
	ICETree.Name = name
	return ICETree

