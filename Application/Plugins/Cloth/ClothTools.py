# ---------------------------------------
# ClothToolsPlugin
# ---------------------------------------
from win32com.client import constants
from Globals import xsi
from Globals import XSIUIToolKit
import Utils as uti
import Property as pro
import Cloth as cc
import ICETree as ice
import XML as xml
import time


# ---------------------------------------
# Register
# ---------------------------------------
def XSILoadPlugin( in_reg ):
	in_reg.Author = "benmalartre"
	in_reg.Name = "ClothTools"
	in_reg.Major = 1
	in_reg.Minor = 0
	
	in_reg.RegisterProperty("Cloth_Rigger")
	in_reg.RegisterCommand("Cloth_Rig", "Cloth_Rig")
	in_reg.RegisterCommand("Cloth_Clean", "Cloth_Clean")
	
	in_reg.RegisterMenu(constants.siMenuTbCreateClothID, "ClothTools", False, False)


# ---------------------------------------
# ClothTools Menu
# ---------------------------------------
def ClothTools_Init( in_ctxt ):
	oMenu = in_ctxt.Source
	oItem = oMenu.AddItem( "Cloth", constants.siMenuItemSection)
	oItem.SetBackgroundColor (178, 191, 194	)
	oMenu.AddCallBackItem("Cloth Builder", "Cloth_GetBuilder")
	oMenu.AddCallBackItem("Cloth Control", "Cloth_GetControl")
	oMenu.AddCallBackItem("Help Page", "Cloth_GetHelpPage")
	
	
def Cloth_GetBuilder(in_ctxt):
	mesh = xsi.Selection(0)
	if not mesh or not mesh.type=="polymsh":
		XSIUIToolKit.MsgBox("Select PolygonMesh and try again...", constants.siMsgOkOnly, "ClothTools")
		return
	prop = mesh.Properties("Cloth_Rigger")
	if not prop:
		prop = mesh.AddProperty("Cloth_Rigger")
	
	xsi.InspectObj(prop, "", "", constants.siLock)
	
	
def Cloth_GetControl(in_ctxt):
	model = xsi.Selection(0)
	if not model or not model.type=="#model":
		XSIUIToolkit.MsgBox("Select Model and try again...", constants.siMsgOkOnly, "ClothTools")
		return
		
	prop = model.Properties("ClothControl")
	if not prop:
		prop = model.AddProperty("Cloth_Control", False, "ClothControl")
	
	xsi.InspectObj(prop, "", "", constants.siLock)

def Cloth_GetHelpPage(in_ctxt):
	xsi.LogMessage("Cloth_GetHelpPage called, but NOT implemented!!")

#---------------------------------------
# Cloth_Rigger
#---------------------------------------
def Cloth_Rigger_Define( in_ctxt ):
	prop = in_ctxt.Source
	prop.AddParameter3("SetClothStiffness",constants.siBool,False,False,True,False,False)
	prop.AddParameter3("StiffnessDriver",constants.siString)
	prop.AddParameter3("ColliderMeshes",constants.siString)
	prop.AddParameter3("ColliderMeshesList",constants.siInt4,0,0,100,False,False)
	prop.AddParameter3("SelfCollide",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("CreateTargetMesh",constants.siBool,False,False,False,False,False)
	prop.AddParameter3("TargetMesh",constants.siString)
	prop.AddParameter3("PresetFile",constants.siString,"")
	prop.AddParameter3("PresetList",constants.siInt4,0,0,100,False,False)
	prop.AddParameter3("PresetMethod",constants.siInt4,0,0,3,False,False)
	prop.AddParameter3("ClothLogo",constants.siString)
	prop.AddParameter3("OutputMesh",constants.siString)
	
	return True
	
def Cloth_Rigger_OnInit( ):
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
	
def Cloth_Rigger_RebuildLayout(prop):
	layout = prop.PPGLayout
	layout.Clear()
	Cloth_Rigger_AddBuilderTab(prop)
	Cloth_Rigger_AddDriverTab(prop)
	Cloth_Rigger_AddTargetTab(prop)
	Cloth_Rigger_AddColliderTab(prop)
	Cloth_Rigger_AddPresetTab(prop)
	
def Cloth_Rigger_AddBuilderTab(prop):
	layout = prop.PPGLayout
	
	layout.AddTab("Builder")
	pro.GetLogo(prop)
	layout.AddGroup("Output Mesh")
	layout.AddRow()
	layout.AddItem("OutputMesh","Mesh")
	layout.AddButton("PickOutputMesh","Pick")
	layout.EndRow()
	layout.EndGroup()
	layout.AddGroup("Tools")
	item = layout.AddButton("BuildCloth","BUILD/REBUILD")
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,30)
	item = layout.AddButton("CleanCloth","CLEAN + STORE PRESET")
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,30)
	item = layout.AddButton("InspectCloth","INSPECT")
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,30)
	layout.EndGroup()
	
def Cloth_Rigger_AddDriverTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Driver")
	pro.GetLogo(prop)
	layout.AddGroup("Driver Object")
	layout.AddStaticText("the driver object is used for setting cloth stiffness\naccording to the character velocity\nIn general we connect it to the Local SRT controller\nFor an hair object, we connect it to the head bone")
	layout.AddGroup()
	aStiffness = ["Don't Correct Stiffness",0,"Select Driver Object",1]
	item = layout.AddEnumControl("SetClothStiffness",aStiffness,"",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,True)
	
	if prop.Parameters("SetClothStiffness").Value:
		layout.AddRow()
		item = layout.AddItem("StiffnessDriver")
		item.SetAttribute(constants.siUINoLabel,True)
		layout.AddButton("PickStiffnessDriver","Pick")
		layout.EndRow()
	layout.EndGroup()
	layout.EndGroup()

def Cloth_Rigger_AddWeightMapTab(prop):
	obj = prop.Parent3DObject
	layout = prop.PPGLayout
	weightmaps_list = pro.BuildListWeightMaps(obj, prop, "FurMaps")
	
	layout.AddGroup("WeightMaps")
	if len(weightmaps_list) == 0:
		layout.AddStaticText("This object contains no weightmap")
	else:
		item = layout.AddEnumControl("WeightMapChooser",weightmaps_list,"List",constants.siControlListBox)
		item.SetAttribute(constants.siUINoLabel,True)
def Cloth_Rigger_AddTargetTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Target")
	pro.GetLogo(prop)
	layout.AddGroup("Target Mesh")
	layout.AddStaticText("the target mesh is used for 3 things:\n-Init Shape\n-Restore Shape\n-Driving Shape")
	layout.AddGroup()
	aMeshTargetOptions = ["Duplicate Mesh",0,"Use Existing Mesh",1]
	item = layout.AddEnumControl("CreateTargetMesh",aMeshTargetOptions,"",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,True)
	
	if prop.Parameters("CreateTargetMesh").Value:
		layout.AddRow()
		item = layout.AddItem("TargetMesh")
		layout.AddButton("PickTargetMesh","Pick")
		layout.EndRow()
		
	else:
		btn = layout.AddButton("DuplicateTargetMesh","Duplicate")
		btn.SetAttribute(constants.siUICX,288)
		btn.SetAttribute(constants.siUICY,33)
	layout.EndGroup()
	layout.EndGroup()
	
	if prop.Parameters("CreateTargetMesh").Value:
		layout.AddGroup("Rigid Mimic Cluster")
		layout.AddRow()
		item = layout.AddButton("CreateRigidMimicCluster","Create From Sel")
		item.SetAttribute(constants.siUICX,109)
		item.SetAttribute(constants.siUICY,30)
		item = layout.AddButton("AddPointsRigidMimicCluster","Add Sel")
		item.SetAttribute(constants.siUICX,59)
		item.SetAttribute(constants.siUICY,30)
		item = layout.AddButton("RemovePointsFromRigidMimicCluster","Remove Sel")
		item.SetAttribute(constants.siUICX,79)
		item.SetAttribute(constants.siUICY,30)
		item = layout.AddButton("ShowRigidMimicCluster","Show")
		item.SetAttribute(constants.siUICX,49)
		item.SetAttribute(constants.siUICY,30)
		layout.EndRow()
		layout.EndGroup()
		
		layout.AddGroup("Soft Mimic Cluster")
		layout.AddRow()
		item = layout.AddButton("CreateSoftMimicCluster","Create From Sel")
		item.SetAttribute(constants.siUICX,109)
		item.SetAttribute(constants.siUICY,30)
		item = layout.AddButton("AddPointsSoftMimicCluster","Add Sel")
		item.SetAttribute(constants.siUICX,59)
		item.SetAttribute(constants.siUICY,30)
		item = layout.AddButton("RemovePointsFromSoftMimicCluster","Remove Sel")
		item.SetAttribute(constants.siUICX,79)
		item.SetAttribute(constants.siUICY,30)
		item = layout.AddButton("ShowSoftMimicCluster","Show")
		item.SetAttribute(constants.siUICX,49)
		item.SetAttribute(constants.siUICY,30)
		layout.EndRow()
		layout.EndGroup()

def Cloth_Rigger_AddColliderTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Collider")
	pro.GetLogo(prop)
	pro.AddObjectList(prop,"ColliderMeshes")
	
	colliders = prop.Parameters("ColliderMeshes").Value
	
	if not colliders:
		layout.AddStaticText("No Collider For this Syflex object")
		layout.AddGroup("Self Collide")
		layout.AddItem("SelfCollide","Apply")
		layout.EndGroup()
		return

	layout.AddGroup("Cloth Point Cluster")
	layout.AddRow()
	item = layout.AddButton("CreateCollidePointCluster","Create From Sel")
	item.SetAttribute(constants.siUICX,109)
	item.SetAttribute(constants.siUICY,30)
	item = layout.AddButton("AddPointsToCollidePointCluster","Add Sel")
	item.SetAttribute(constants.siUICX,59)
	item.SetAttribute(constants.siUICY,30)
	item = layout.AddButton("RemovePointsFromCollidePointCluster","Remove Sel")
	item.SetAttribute(constants.siUICX,79)
	item.SetAttribute(constants.siUICY,30)
	item = layout.AddButton("ShowCollidePointCluster","Show")
	item.SetAttribute(constants.siUICX,49)
	item.SetAttribute(constants.siUICY,30)
	layout.EndRow()
	layout.EndGroup()
	layout.AddGroup("Collider Polygon Cluster")
	layout.AddRow()
	item = layout.AddButton("CreateColliderPolygonCluster","Create From Sel")
	item.SetAttribute(constants.siUICX,109)
	item.SetAttribute(constants.siUICY,30)
	item = layout.AddButton("AddPolygonsToColliderPolygonCluster","Add Sel")
	item.SetAttribute(constants.siUICX,59)
	item.SetAttribute(constants.siUICY,30)
	item = layout.AddButton("RemovePolygonsFromColliderPolygonCluster","Remove Sel")
	item.SetAttribute(constants.siUICX,79)
	item.SetAttribute(constants.siUICY,30)
	item = layout.AddButton("ShowColliderPolygonCluster","Show")
	item.SetAttribute(constants.siUICX,49)
	item.SetAttribute(constants.siUICY,30)
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("Self Collide")
	layout.AddItem("SelfCollide","Apply Self Collide")
	layout.EndGroup()
	


def Cloth_Rigger_AddPresetTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Presets")
	pro.GetLogo(prop)
	layout.AddGroup("Method")
	aItems = ["AutoSaved",0,"Presets",1,"Save/Load",2]
	item = layout.AddEnumControl("PresetMethod",aItems,"Method",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,True)
	layout.EndGroup()
	
	method = prop.Parameters("PresetMethod").Value
	
	if method == 0:
		layout.AddGroup("AutoSaved")
		path = ""
		layout.AddStaticText("Prest File Saved as : \n"+path)
		layout.EndGroup()
		
	elif  method == 1:
		layout.AddGroup("Presets")
		aPresets = []
		
		layout.AddRow()
		item = layout.AddEnumControl("PresetList",aPresets,"Presets",constants.siControlListBox)
		item.SetAttribute(constants.siUINoLabel,True)
		button = layout.AddButton("ApplyPreset","Apply")
		button.SetAttribute(constants.siUICX,80)
		button.SetAttribute(constants.siUICY,120)
		layout.EndRow()
		layout.EndGroup()
		
	elif method == 2:
		layout.AddGroup("Preset")
		item = layout.AddEnumControl("PresetFile",[],"Preset",constants.siControlFilePath)
		item.SetAttribute(constants.siUIFileFilter,"XML files (*.xml)|*.xml:*.xml|All Files (*.*)|*.*||")
		item.SetAttribute(constants.siUINoLabel,True)
		item.SetAttribute(constants.siUIFileMustExist,True)
	
		#layout.AddSpacer()
		layout.AddRow()
		btn = layout.AddButton("SavePreset","Save Preset");
		btn.SetAttribute(constants.siUICX,149)
		btn.SetAttribute(constants.siUICY,30)
		btn = layout.AddButton("LoadPreset","Load Preset");
		btn.SetAttribute(constants.siUICX,149)
		btn.SetAttribute(constants.siUICY,30)
		layout.EndRow()
		
		layout.EndGroup()
	
#---------------------------------------
# Parameters Change
#---------------------------------------
def Cloth_Rigger_CreateTargetMesh_OnChanged():
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
		
def Cloth_Rigger_RigType_OnChanged():
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()

def Cloth_Rigger_PresetMethod_OnChanged():
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
	
def Cloth_Rigger_ColliderMeshesList_OnChanged():
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()

def Cloth_Rigger_SetClothStiffness_OnChanged():
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
	
#---------------------------------------
# Buttons
#---------------------------------------
def Cloth_Rigger_PickOutputMesh_OnClicked():
	pick = uti.PickElement(constants.siObjectFilter,"Pick Output Mesh")
	if pick:
		PPG.OutputMesh.Value = pick.Name
		
def Cloth_Rigger_DuplicateTargetMesh_OnClicked():
	prop = PPG.Inspected(0)
	target = Cloth_GetTargetMesh(prop)
	
def Cloth_Rigger_AddColliderMeshes_OnClicked():
	prop = PPG.Inspected(0)
	pro.AddObjectToList(prop,"ColliderMeshes","Pick Collider Mesh")
	pro.AddColliderObject(prop)

	Cloth_Rigger_RebuildLayout(prop)
	PPG.Refresh()
	
	#prop.Parameters("ColliderMeshesList").Value = len(prop.PPGLayout.Item("ColliderMeshesList").UIItems)/2

def Cloth_Rigger_RemoveColliderMeshes_OnClicked():
	prop = PPG.Inspected(0)
	pro.RemoveColliderObject(prop)
	pro.RemoveObjectFromList(PPG.Inspected(0),"ColliderMeshes")
	
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()

def Cloth_Rigger_AddStaticMeshes_OnClicked():
	pro.AddObjectToList(PPG.Inspected(0),"StaticMeshes","Pick Static Mesh")
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()

def Cloth_Rigger_RemoveStaticMeshes_OnClicked():
	pro.RemoveObjectFromList(PPG.Inspected(0),"StaticMeshes")
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
	
def Cloth_Rigger_AddDynamicMeshes_OnClicked():
	pro.AddObjectToList(PPG.Inspected(0),"DynamicMeshes","Pick Dynamic Mesh")
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()

def Cloth_Rigger_RemoveDynamicMeshes_OnClicked():
	pro.RemoveObjectFromList(PPG.Inspected(0),"DynamicMeshes")
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
	
def Cloth_Rigger_PickTargetMesh_OnClicked():
	pick = uti.PickElement(constants.siPolyMeshFilter,"Pick Target Mesh")
	if pick:
		PPG.TargetMesh.Value = pick.Name

def Cloth_Rigger_PickStiffnessDriver_OnClicked():
	pick = uti.PickElement(constants.siObjectFilter,"Pick Stiffnes Driver")
	if pick:
		PPG.StiffnessDriver.Value = pick.Name
		
def Cloth_Rigger_BuildCloth_OnClicked():
	for p in PPG.Inspected:
		xsi.Cloth_Rig(p)
	
def Cloth_Rigger_CleanCloth_OnClicked():
	for p in PPG.Inspected:
		xsi.Cloth_Clean(p)
	
def Cloth_Rigger_CreateCollidePointCluster_OnClicked():
	items = PPG.PPGLayout.Item("ColliderMeshesList").UIItems
	value = PPG.ColliderMeshesList.Value
	collider = items[value*2]
	
	clsname = "Collide_"+collider
	clstype = constants.siVertexCluster
	target = prop.Parent3DObject
		
	uti.CreateClusterFromSelection(target,clsname,clstype)
	
def Cloth_Rigger_CreateColliderPolygonCluster_OnClicked():
	prop = PPG.Inspected(0)
	obj = prop.Parent3DObject
	clsname = "Collide_"+obj.Name
	clstype = constants.siPolygonCluster
	target = pro.GetObjectFromSelectedUIItem(prop,"ColliderMeshes")
		
	uti.CreateClusterFromSelection(target,clsname,clstype)
	
def Cloth_Rigger_CreateRigidMimicCluster_OnClicked():
	prop = PPG.Inspected(0)
	clsname = "RigidMimicCls"
		
	uti.CreateClusterFromSelection(clsname)

def Cloth_Rigger_CreateSoftMimicCluster_OnClicked():
	prop = PPG.Inspected(0)
	target = prop.Parent3DObject
	clsname = "SoftMimicCls"
		
	uti.CreateClusterFromSelection(target,clsname)
		
def Cloth_Rigger_SelectColliderPolygonCluster_OnClicked():
	item_id = pro.GetIDFromSelectedUIItem(PPG.Inspected(0),"ColliderMeshes")
	target = pro.GetObjectFromSelectedUIItem(PPG.Inspected(0),"ColliderMeshes")
	cls = xsi.OpenTransientExplorer( target, constants.siSEFilterPropertiesAndPrimitives, 3 )(0)
	if cls and cls.Type == "poly":
		PPG.Inspected(0).Parameters("ColliderPolygonCluster"+str(item_id)).Value = cls.Name
	else:
		xsi.LogMessage("[Cloth_Rigger] Invalid Selection : Select a Polygon Cluster on Collider Geometry", constants.siWarning)

def Cloth_Rigger_SelectCollidePointCluster_OnClicked():
	item_id = pro.GetIDFromSelectedUIItem(PPG.Inspected(0),"ColliderMeshes")
	target = PPG.Inspected(0).Parent3DObject
	cls = xsi.OpenTransientExplorer( target, constants.siSEFilterPropertiesAndPrimitives, 3 )(0)
	if cls and cls.Type == "pnt":
		PPG.Inspected(0).Parameters("CollidePointCluster"+str(item_id)).Value = cls.Name
	else:
		xsi.LogMessage("[Cloth_Rigger] Invalid Selection : Select a Point Cluster on Cloth Geometry", constants.siWarning)
		
def Cloth_Rigger_SelectRigidMimicCluster_OnClicked():
	target = PPG.Inspected(0).Parent3DObject
	cls = xsi.OpenTransientExplorer( target, constants.siSEFilterPropertiesAndPrimitives, 3 )(0)
	if cls and cls.Type == "pnt":
		PPG.Inspected(0).Parameters("RigidMimicCluster").Value = cls.Name
		uti.GetWeightMap(target,"RigidMimicMap",1,0,1,cls)
	else:
		xsi.LogMessage("[Cloth_Rigger] Invalid Selection : Select a Point Cluster on Cloth Geometry", constants.siWarning)
		
def Cloth_Rigger_SelectSoftMimicCluster_OnClicked():
	target = PPG.Inspected(0).Parent3DObject
	cls = xsi.OpenTransientExplorer( target, constants.siSEFilterPropertiesAndPrimitives, 3 )(0)
	if cls and cls.Type == "pnt":
		PPG.Inspected(0).Parameters("SoftMimicCluster").Value = cls.Name
		uti.GetWeightMap(target,"SoftMimicMap",1,0,1,cls)
	else:
		xsi.LogMessage("[Cloth_Rigger] Invalid Selection : Select a Point Cluster on Cloth Geometry", constants.siWarning)
	
	
#---------------------------------------
# Rig Cloth Command
#---------------------------------------
def Cloth_Rig_Init(ctxt):
	cmd = ctxt.Source
	cmd.Description = "Build Cloth Rig From Given Cloth_Rigger PPG"

	args = cmd.Arguments
	args.Add("prop",constants.siArgumentInput)

def Cloth_Rig_Execute(prop):	
	obj = prop.Parent3DObject

	#if cloth op, we delete it, as well as ICE trees
	xsi.Cloth_Clean(prop)
		
	model = obj.Model
	
	clothop = None
	target = Cloth_GetTargetMesh(prop)
	
	clothop = Cloth_ApplySyflexOp(obj)
	
	init = Cloth_ApplyInitOp(obj,target)
	restore = Cloth_ApplyRestoreOp(obj, target)
			
	gravity = Cloth_GetGravity(clothop)
	wind = Cloth_GetWind(clothop)
	air = Cloth_GetAir(clothop)
	
	if prop.Parameters("SelfCollide").Value:
		selfcollide = Cloth_GetSelfCollide(obj)
	
	colliders = Cloth_GetColliderMeshes(prop)
	extra = Cloth_GetExtraCollider(prop)
	rigidmimic = Cloth_GetRigidMimic(prop,target)
	softmimic = Cloth_GetSoftMimic(prop,target)
	
	#if prop.Parameters("SetClothStiffness").Value:
	#	driver = xsi.Dictionary.GetObject(prop.Parameters("StiffnessDriver").Value)
	#	stiffness = Cloth_ApplyStiffnessOp(obj, driver)
	
	# lastly we read XML preset if exists
	#if cloth op, we store a preset
	Cloth_Rigger_ReadXML(prop)
	
def Cloth_Clean_Init(ctxt):
	cmd = ctxt.Source
	cmd.Description = "Clean Cloth Rig From Given Cloth_Rigger PPG"

	args = cmd.Arguments
	args.Add("prop",constants.siArgumentInput)
	
def Cloth_Clean_Execute(prop):

	#if cloth op, we store a preset
	Cloth_Rigger_WriteXML(prop)
	
	obj = prop.Parent3DObject
	trees = obj.ActivePrimitive.ICETrees
	
	syflexop = cc.GetClothOperator(obj)
	if not syflexop == None:
		xsi.DeleteObj(syflexop)
	
	if trees.Count>0:
		restore = obj.ActivePrimitive.ICETrees.Find("ClothRestore")
		if restore:
			xsi.DeleteObj(restore)
		
		correct = obj.ActivePrimitive.ICETrees.Find("ClothCorrectStiffness")
		if correct:
			xsi.DeleteObj(correct)
			
		init = obj.ActivePrimitive.ICETrees.Find("ClothInit")
		if init:
			xsi.DeleteObj(init)
	
	model = obj.Model
	target = model.FindChild(obj.Name+"_Target")
	if target:
		xsi.DeleteObj(target)

def Cloth_ApplyInitOp(obj,target):
	tree = ice.CreateICETree(obj,"ClothInit",1)
	compound = xsi.AddICECompoundNode("ClothInitShape", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".execute")
	xsi.SetValue(str(compound)+".Reference", ice.ReplaceModelNameByThisModel(target,obj.Model), "")
	
def Cloth_ApplyStiffnessOp(obj, target):
	tree = ice.CreateICETree(obj,"ClothStiffness",1)
	compound = xsi.AddICECompoundNode("ClothSetMimicStiffness", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".execute")
	
	if obj.Model.FullName == target.Model.FullName:
		xsi.SetValue(str(compound)+".Velocity_Driver_Reference", ice.ReplaceModelNameByThisModel(target), "")
	else:
		xsi.SetValue(str(compound)+".Velocity_Driver_Reference", target.FullName, "")
	
def Cloth_ApplyRestoreOp(obj, target):
	tree = ice.CreateICETree(obj,"ClothRestore",2)
	compound = xsi.AddICECompoundNode("ClothRestoreShape", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Result")
	xsi.SetValue(str(compound)+".Target_Mesh_Reference", ice.ReplaceModelNameByThisModel(target), "")

def Cloth_ApplySyflexOp(obj):
	syflexop = obj.ActivePrimitive.ConstructionHistory.Find( "syCloth" )
	if syflexop:
		xsi.DeleteObj(syflexop)
		
	clothcls = obj.ActivePrimitive.Geometry.Clusters("ClothCls")
	if not clothcls:
		clothcls = uti.CreateCompleteButNotAlwaysCluster(obj,constants.siVertexCluster,"ClothCls")
	
	massmap = uti.GetWeightMap(obj,"MassMap",1,0,1,clothcls)
	stiffmap = uti.GetWeightMap(obj,"StiffMap",1,0,1,clothcls)
	restoremap = uti.GetWeightMap(obj,"RestoreMap",0,0,1,clothcls)
	
	xsi.FreezeObj(massmap.FullName+","+stiffmap.FullName)
	
	#syflexop = xsi.ApplyOperator("syCloth",obj.FullName+";"+massmap.FullName+";"+stiffmap.FullName)
	syflexop = xsi.ApplyOp("syCloth",obj.FullName+";"+massmap.FullName+";"+stiffmap.FullName,3, 0, None, 2)(0)
	stf = xsi.ApplyOperator("syProperties",syflexop)
	startframe = uti.GetStartFrame()
	syflexop.Parameters("FirstFrame").Value = startframe
	xsi.SaveKey(syflexop.Parameters("Update"),1,1)
	
	stf.Parameters("Density").Value = 0.5
	stf.Parameters("StretchStiffness").Value = 1
	stf.Parameters("ShearStiffness").Value = 0.2
	stf.Parameters("BendStiffness").Value = 0.1
	stf.Parameters("StretchDamping").Value = 0.1
	stf.Parameters("ShearDamping").Value = 0.02
	stf.Parameters("BendDamping").Value = 0.01

	return syflexop

def Cloth_GetTargetMesh(prop):
	mesh = prop.Parent3DObject
	model = mesh.Model
	
	dup = prop.Parameters("CreateTargetMesh").Value
	target = None
	if not dup:
		target = Cloth_DuplicateMesh(mesh)
	else:
		try:
			target = model.FindChild(prop.Parameters("TargetMesh").Value)
			if not target:
				target = Cloth_DuplicateMesh(mesh)
		except:
			target = Cloth_DuplicateMesh(mesh)
		'''
		if not uti.CheckGeometryMatch(mesh,target):
			XSIUIToolkit.MsgBox("Target Mesh Invalid: we generate a new one",constants.siMsgExclamation,"Cloth_Tools")
			target = Cloth_DuplicateMesh(mesh)
		'''
		
	#Transfer Envelope from Output mesh if not exists	
	if not target.ActivePrimitive.ConstructionHistory.Find( "envelopop" ):
		out = Cloth_GetOutputMesh(prop)
		if out:
			envelope = out.ActivePrimitive.ConstructionHistory.Find( "envelopop" )
			if envelope:
				env = xsi.Dictionary.GetObject(envelope)
				weights = env.PortAt(4,0,0).Target2
				xsi.ApplyGenOp("Gator", "", target.FullName+";"+out.FullName, 3, "siPersistentOperation", "siKeepGenOpInputs", "")
				xsi.TransferClusterPropertiesAcrossGenOp(target.FullName+".polymsh.TransferAttributes", target.FullName, str(weights), "Envelope_Weights", "")
				xsi.FreezeModeling(target)
	
	# Deform OutputMesh
	Cloth_DeformOutputMesh(prop)
		
	#Create Init ICE Tree
	trees = target.ActivePrimitive.ICETrees
	if not trees.Find("InitTarget"):
		tree = ice.CreateICETree(target,"InitTarget",1)
		compound = xsi.AddICECompoundNode("ClothInitTarget", str(tree))
		xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")
	
	#Create PreRoll ICE Tree
	if not trees.Find("PreRollTarget"):
		tree = ice.CreateICETree(target,"PreRollTarget",3)
		compound = xsi.AddICECompoundNode("ClothTargetPreRoll", str(tree))
		xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")
	
	# Update Cloth Rigger
	prop.Parameters("CreateTargetMesh").Value = 1
	prop.Parameters("TargetMesh").Value = target.Name
	uti.SetVisibility([target],False,False)
	
	Cloth_Rigger_RebuildLayout(prop)
	try:
		PPG.Refresh()
	except:
		pass
	
	return target
	
def Cloth_GetOutputMesh(prop):
	outname = prop.OutputMesh.Value
	if outname == "":
		return None
		
	obj = prop.Parent3DObject
	model = obj.Model
	out = model.FindChild(outname)
	if out:
		return out
	return None
	
	
def Cloth_DeformOutputMesh(prop):
	out = Cloth_GetOutputMesh(prop)
	cloth = prop.Parent3DObject
	
	if not out:
		xsi.LogMessage("[Cloth_Rigger]There is NO Output Mesh set for "+cloth.FullName, constants.siWarning)
		return
	
	# delete cage if already exists
	cage = out.ActivePrimitive.ConstructionHistory.Find( "cagedeformop" )
		
	if cage:
		xsi.DeleteObj(cage)
	
	# delete all cagedeform cluster
	cls = out.ActivePrimitive.Geometry.Clusters
	for c in cls:
		if not c.Name.find("CageWeightCls")==-1:
			xsi.DeleteObj(c)
	
	cage = xsi.ApplyDeformByCage(out.FullName+";"+cloth.FullName, constants.siConstructionModeAnimation)(0)
	cage.Strength = 10
	cage.Falloff = 0.2
	
	# Deactivate EnvelopeOp
	env = out.ActivePrimitive.ConstructionHistory.Find("envelopop")
	if env:
		env.Mute = True
	
def Cloth_DuplicateMesh(mesh):
	data = mesh.ActivePrimitive.Geometry.Get2()
	target = xsi.ActiveSceneRoot.AddPolygonMesh (data[0], data[1])
	target.Name = mesh.Name+"_Target"
	mesh.Parent3DObject.AddChild(target)
	return target
	
def Cloth_GetColliderMeshes(prop):
	obj = prop.Parent3DObject
	model = obj.Model
	colliders = []
	v = prop.Parameters("ColliderMeshes").Value
	if not v:
		return colliders
		
	split = v.split("|")
	split.pop()
	
	_id = 1
	_col = None
	for s in split:
		_col = model.FindChild(s)
			
		if not _col:
			xsi.LogMessage("[Cloth_Rigger] Collider named "+s+" not found...\nRemove it from list!!",constants.siWarning)
			continue

		collider = cc.Collider(obj,_col,False)
		collider.GetPntCls(prop)
		collider.GetPolyCls(prop)
		colliders.append(collider)
		collider.Rig()
		_id += 1
				
	return colliders
	
def Cloth_GetExtraCollider(prop):
	obj = prop.Parent3DObject
	parent = obj.Parent3DObject
	
	model = obj.Model
	_col = model.FindChild("ExtraCollider")
	if not _col:
		_id = pro.GetNbObjectsInList(prop,"ColliderMeshes")
		
		col = parent.AddPrimitive("EmptyPolygonMesh","ExtraCollider")
		tree = ice.CreateICETree(col,"ClothExtraCollider",0)
		compound = xsi.AddICECompoundNode("ClothExtraCollider", str(tree))
		xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".execute")
		
	extra = cc.Collider(obj,_col,True)
	extra.GetPntCls(prop)
	extra.GetPolyCls(prop)
	extra.Rig()
		
	return extra

def Cloth_GetRigidMimic(prop,target):
	Cloth_GetMimic(prop,target,"Rigid",True)
	
def Cloth_GetSoftMimic(prop,target):
	Cloth_GetMimic(prop,target,"Soft",False)
	
def Cloth_GetMimic(prop,target,prefix,empty=False):
	obj = prop.Parent3DObject
	syflexop = obj.ActivePrimitive.ConstructionHistory.Find( "syCloth" )
	
	mimiccls = obj.ActivePrimitive.Geometry.Clusters(prefix+"MimicCls")
	if not mimiccls:
		mimiccls = uti.CreateCompleteButNotAlwaysCluster(obj,constants.siVertexCluster,prefix+"MimicCls")
	
	mimicmap = uti.GetWeightMap(obj,prefix+"MimicMap",1,0,1,mimiccls)
	mimicop = xsi.ApplyOperator( "syMimic", syflexop.FullName + ";" + target.FullName + ";" + target.Kinematics.FullName + ";" + 
									mimiccls.FullName + ";" + obj.Kinematics.FullName + ";" + mimicmap.FullName )
									
	mimicop.Reset = 1
	mimicop.Active = False
	
	if prefix == "Soft":								
		mimicop.Distance = 0.001
		mimicop.Stiff = 0.1
		mimicop.Damp = 0.01
		xsi.SaveKey(mimicop.ModifMap,0,True)
		
	else:
		mimicop.Distance = 0
		mimicop.Damp = 0.01
		mimicop.Stiff = 1	

def Cloth_GetGravity(clothop):
	gravity = xsi.ApplyOperator( "syGravity", clothop )
	gravity.Parameters("Gy").Value = -0.1
	
	model = clothop.Parent3DObject.Model
	icon = model.FindChild("Cloth_Gravity")
	if not icon:
		icon = xsi.GravityIcon()
		icon.Name = "Cloth_Gravity"
		clothop.Parent3DObject.Parent3DObject.AddChild(icon)
		
	xsi.ApplySyGravityIconOp(gravity, icon)

def Cloth_GetWind(clothop):
	wind = xsi.ApplyOperator( "syWind", clothop )
	model = clothop.Parent3DObject.Model
	icon = model.FindChild("Cloth_Wind")
	if not icon:
		icon = xsi.WindIcon()
		icon.Name = "Cloth_Wind"
		clothop.Parent3DObject.Parent3DObject.AddChild(icon)
		
	wind.Parameters("Intensity").Value = 0.1
	wind.Parameters("Active").Value =False
	
	xsi.ApplySyWindIconOp(wind, icon)
	
def Cloth_GetAir(clothop):
	air = xsi.ApplyOperator( "syAir", clothop )
	air.Parameters("Damp").Value = 0.05
	
def Cloth_GetSelfCollide(obj):
	syflexop = obj.ActivePrimitive.ConstructionHistory.Find( "syCloth" )
	
	cls = obj.ActivePrimitive.Geometry.Clusters("SelfCollideCls")
	if not cls:
		cls = uti.CreateCompleteButNotAlwaysCluster(obj,constants.siPolygonCluster,"SelfCollideCls")

	selfcollideop = xsi.ApplyOperator( "sySelfCollide", syflexop.FullName + ";" + obj.FullName + ";" + cls.FullName )
	selfcollideop.Reset = 1;
	selfcollideop.Envelope = 0.05;


# Preset
#----------------------------------------------------		
def Cloth_GetPresetList():
	plugins = Application.Plugins
	aPresets = []

	for p in plugins:
		if p.Name == "ClothToolsPlugin":
			path = p.OriginPath
			presetpath = path.replace("Application\\Plugins\\Cloth\\","Data\\ClothPresets")
			XSIUtils.EnsureFolderExists(presetpath)
			regex = ".*\.xml$"
			files = xsi.FindFilesInFolder(presetpath,regex,True,False)
			
			i = 0
			for f in files:
				aPresets.append(f)
				aPresets.append(i)
				i+=1
			return aPresets

def Cloth_PresetMethod_OnChanged():
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()

def Cloth_Rigger_WriteXML(prop,filename=None):
	if not filename:
		filename = Cloth_Rigger_GetXMLFile(prop)

	cloth = prop.Parent3DObject
	clothop = cloth.ActivePrimitive.ConstructionHistory.Find("syCloth")
	if clothop:
		xml.WriteSyflexXMLPreset(filename,clothop)

def Cloth_Rigger_ReadXML(prop,filename=None):
	if not filename:
		filename = Cloth_Rigger_GetXMLFile(prop)

	cloth = prop.Parent3DObject
	clothop = cloth.ActivePrimitive.ConstructionHistory.Find("syCloth")
	if clothop:
		xml.ReadSyflexXMLPreset(filename,clothop)

def Cloth_Rigger_GetXMLFile(prop):
	if prop.Parameters("PresetFile").Value == "":
		filename = Cloth_Rigger_GetAutoSavePresetPath(prop)
		return filename
	else:
		filename = prop.Parameters("PresetFile").Value
		if not xml.CheckFileExists(filename):
			prop.Parameters("PresetFile").Value = ""
			Cloth_Rigger_GetXMLFile(prop)
		else:
			return filename
			
def Cloth_Rigger_GetAutoSavePresetPath(PPG):
	path = xsi.ActiveProject2.Path
	obj = PPG.Parent3DObject
	presetfolder = XSIUtils.BuildPath(path,"ClothPresets")
	XSIUtils.EnsureFolderExists(presetfolder,False)
	presetpath = XSIUtils.BuildPath(presetfolder,obj.Name+".xml")
	return presetpath
	
def Cloth_Rigger_SavePreset_OnClicked():
	path = PPG.PresetFile.Value
	if path:
		Cloth_Rigger_WriteXML(PPG.Inspected(0),path)

def Cloth_Rigger_LoadPreset_OnClicked():
	path = PPG.PresetFile.Value
	if path:
		Cloth_Rigger_ReadXML(PPG.Inspected(0),path)
