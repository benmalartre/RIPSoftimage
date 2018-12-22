
import win32com.client
from win32com.client import constants
from siutils import *

siuitk = disp('XSI.UIToolkit')

false = 0
true = 1
null = 0

# Helpers
#------------------------------------------------------------
def CreateICETree(obj,name="ICETree",constructionhistory=0):
	tree = Application.ApplyOp("ICETree", obj, "siNode", "", "", constructionhistory)
	ICETree = Application.Dictionary.GetObject(tree)
	ICETree.Name = name
	return ICETree
	
def CreateWeightMap(cluster,name,value=0,inMin=0,inMax=1):

	for wm in cluster.LocalProperties.Filter("wtmap"):
		if wm.Name == name:
			return wm
			
	# create and set weight map 
	wm = disp(cluster.AddProperty("Weight Map Property",False,name))
	
	wm.Parameters("wmin").Value = inMin*0.01
	wm.Parameters("wmax").Value = inMax*0.01

	elems_tuple = wm.Elements.Array
	elems = [value for i in range(len(elems_tuple[0])) for j in range(len(elems_tuple))]
	wm.Elements.Array = elems
	
	Application.FreezeObj(wm)
	
	return wm

# Register
#------------------------------------------------------------
def XSILoadPlugin( in_reg ):
	in_reg.Author = "benmalartre"
	in_reg.Name = "FuraxPlugin"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterCommand("FuraxSetupEmitter","FuraxSetupEmitter")
	in_reg.RegisterCommand("FuraxSetupCage","FuraxSetupCage")
	in_reg.RegisterCommand("FuraxCreateGuide","FuraxCreateGuide")
	in_reg.RegisterCommand("FuraxCreateRender","FuraxCreateRender")

	return True

def XSIUnloadPlugin( in_reg ):
	strPluginName = in_reg.Name
	Application.LogMessage(str(strPluginName) + str(" has been unloaded."),constants.siVerbose)
	return True

# Implementation
#------------------------------------------------------------
def FuraxSetupEmitter_Init( in_ctxt ):
	oCmd = in_ctxt.Source
	oCmd.Description = ""
	oCmd.ReturnValue = True

	return True

def FuraxSetupEmitter_Execute():
	emitter = Application.Selection(0)
	if not emitter or not emitter.Type == "polymsh":
		Application.LogMessage("You need to select emitter polymesh")
		return False
		
	cluster =  emitter.ActivePrimitive.Geometry.Clusters("Hair")
	if not cluster:
		cluster = emitter.ActivePrimitive.Geometry.AddCluster(C.siVertexCluster,"Hair")
		
	CreateWeightMap(cluster,"Density_Map",1,0,1)
	CreateWeightMap(cluster,"Size_Map",0.5,0,1)
	CreateWeightMap(cluster,"Frizz_Map",1,0,1)
	CreateWeightMap(cluster,"Cut_Map",0.5,0,1)
	CreateWeightMap(cluster,"Clump_Map",0.5,0,1)
	CreateWeightMap(cluster,"Clone_Map",0.5,0,1)
	CreateWeightMap(cluster,"Spread_Map",0.5,0,1)
	CreateWeightMap(cluster,"SpreadRoot_Map",0.5,0,1)
	CreateWeightMap(cluster,"SpreadTip_Map",0.5,0,1)
	return true

def FuraxSetupCage_Init( in_ctxt ):
	oCmd = in_ctxt.Source
	oCmd.Description = ""
	oCmd.ReturnValue = True

	return True

def FuraxSetupCage_Execute():
	cage = Application.Selection(0)
	if not cage or not cage.Type == "polymsh":
		Application.LogMessage("You need to select cage polymesh")
		return False
		
	geom = cage.ActivePrimitive.Geometry
	found = False
	for cls in geom.Clusters:
		Application.LogMessage(cls.Name)
		if cls.Name == "RootVertices" and cls.Type == "pnt":
			found = True
			
	if not found:
		siuitk.MsgBox( "Can't find RootVertices Cluster\n Create it and try again", C.siMsgCritical, "Furax" )
		return False
		
	init = CreateICETree(cage,"FuraxInit",1)
	node = Application.AddICECompoundNode("Furax_Mesh_Init", init)
	Application.ConnectICENodes(str(init)+".port1", str(node)+".Execute")

	set = CreateICETree(cage,"FuraxSet",3)
	node = Application.AddICECompoundNode("Furax_Mesh_Set", set)
	Application.ConnectICENodes(str(set)+".port1", str(node)+".Execute")
	
def FuraxCreateGuide_Init( in_ctxt ):
	oCmd = in_ctxt.Source
	oCmd.Description = ""
	oCmd.ReturnValue = True

	oArgs = oCmd.Arguments
	oArgs.Add("cage",C.siArgumentInput)
	
	return True

def FuraxCreateGuide_Execute(cage):
	if not cage:
		rtn = Application.PickElement(C.siPolyMeshFilter, "Pick Cage Object", "Pick Cage Object" )
		cage = rtn.Value( "PickedElement" )
	
	found = False
	for t in cage.ActivePrimitive.ICETrees:
		if t.Name == "FuraxInit":
			found = True
			
	if not found:
		siuitk.MsgBox( "Cage object NOT valid\n Setup Cage Object First!!!", C.siMsgCritical, "Furax" )
		return False
		
	guide = Application.GetPrim("PointCloud", "Guide", Application.ActiveSceneRoot)
	tree = CreateICETree(guide,"FuraxGuide",0)
	emit = Application.AddICECompoundNode("Furax_Guide", tree)
	get = Application.AddICENode("GetDataNode", tree)
	Application.SetValue(str(get)+".reference", cage.FullName, "")
	Application.ConnectICENodes(str(emit)+".Guide_Mesh", str(get)+".outname")
	Application.ConnectICENodes(str(tree)+".port1", str(emit)+".execute")

def FuraxCreateRender_Init( in_ctxt ):
	oCmd = in_ctxt.Source
	oCmd.Description = ""
	oCmd.ReturnValue = True

	oArgs = oCmd.Arguments
	oArgs.Add("emitter_static",C.siArgumentInput)
	oArgs.Add("emitter_deform",C.siArgumentInput)
	oArgs.Add("guide_cloud",C.siArgumentInput)
	return True

def FuraxCreateRender_Execute(emitter_static,emitter_deform,guide_cloud):
	if not emitter_static:
		rtn = Application.PickElement(C.siPolyMeshFilter, "Pick Static Emitter", "Pick Static Emitter" )
		emitter_static = rtn.Value( "PickedElement" )
	if not emitter_deform:
		rtn = Application.PickElement(C.siPolyMeshFilter, "Pick Deform Emitter", "Pick Deform Emitter" )
		emitter_deform = rtn.Value( "PickedElement" )
	if not guide_cloud:
		rtn = Application.PickElement(C.siPointCloudFilter, "Pick Static Emitter", "Pick Static Emitter" )
		guide_cloud = rtn.Value( "PickedElement" )
	
	if not emitter_static or not emitter_deform or not guide_cloud:
		siuitk.MsgBox( "Invalid Inputs, Can't create Render Cloud!!!", C.siMsgCritical, "Furax" )
		return False
		
	found = False
	for t in guide_cloud.ActivePrimitive.ICETrees:
		if t.Name == "FuraxGuide":
			found = True
			
	if not found:
		siuitk.MsgBox( "Guide Cloud object NOT valid\n Create Guide First!!!", C.siMsgCritical, "Furax" )
		return False
		
	render = Application.GetPrim("PointCloud", "Render", Application.ActiveSceneRoot)
	tree = CreateICETree(render,"FuraxRender",0)
	emit = Application.AddICECompoundNode("Furax_Render", tree)
	get1 = Application.AddICENode("GetDataNode", tree)
	Application.SetValue(str(get1)+".reference", emitter_static.FullName, "")
	get2 = Application.AddICENode("GetDataNode", tree)
	Application.SetValue(str(get2)+".reference", emitter_deform.FullName, "")
	get3 = Application.AddICENode("GetDataNode", tree)
	Application.SetValue(str(get3)+".reference", guide_cloud.FullName, "")
	
	Application.ConnectICENodes(str(emit)+".Skull_Mesh_Static", str(get1)+".outname")
	Application.ConnectICENodes(str(emit)+".Skull_Mesh_Deform", str(get2)+".outname")
	Application.ConnectICENodes(str(emit)+".Guide_Cloud", str(get3)+".outname")
	Application.ConnectICENodes(str(tree)+".port1", str(emit)+".execute")
