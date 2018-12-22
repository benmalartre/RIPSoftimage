#---------------------------------------
# HairTools Plugin
#---------------------------------------
from win32com.client import constants
from Globals import xsi
from Globals import XSIUtils
import Utils as uti
import ICETree as ice
import Property as pro


#---------------------------------------
# Register
#---------------------------------------
def XSILoadPlugin( in_reg ):
	in_reg.Author = "benmalartre"
	in_reg.Name = "HairTools"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterCommand("Hair_MergeMeshes","Hair_MergeMeshes")
	in_reg.RegisterProperty("Hair_Groom")
	
	in_reg.RegisterCommand("Hair_ApplyDynamics","Hair_ApplyDynamics")
	in_reg.RegisterMenu(constants.siMenuTbICEParticlesCreateID,"HairTools",False,False)


#---------------------------------------
# HairTools Menu
#---------------------------------------
def HairTools_Init( in_ctxt ):
	oMenu = in_ctxt.Source
	oItem = oMenu.AddItem( "Hair", constants.siMenuItemSection)
	oItem.SetBackgroundColor (178, 191, 194	)
	oMenu.AddCallBackItem("Hair Groom","Hair_GetGroomProperty")
	oMenu.AddCallBackItem("Help Page","Hair_GetHelpPage")
	
def Hair_GetGroomProperty(in_ctxt):
	mesh = xsi.Selection(0)
	if not mesh or not mesh.type=="polymsh":
		XSIUIToolkit.MsgBox("Select PolygonMesh and try again...",constants.siMsgOkOnly,"HairTools")
		return
	prop = mesh.Properties("Hair_Groom")
	if not prop:
		prop = mesh.AddProperty("Hair_Groom")
	
	uti.GetWeightMap(mesh,"Frizz_Map",1,0,1)
	uti.GetWeightMap(mesh,"Spread_Map",1,0,1)
	xsi.InspectObj(prop,"","",constants.siLock)

def Hair_GetHelpPage(in_ctxt):
	path = pro.GetPluginPath("HairTools")
	spath = path.split("Application")[0]
	doc = XSIUtils.BuildPath(spath,"docs","Hair_documentation.html")
	xsi.OpenNetView(doc)
	
	
#---------------------------------------
# Hair Groom Property
#---------------------------------------
def Hair_Groom_Define(in_ctxt):
	prop = in_ctxt.Source
	prop.AddParameter3("UpVectorCurve",constants.siString,"")
	prop.AddParameter3("SkullMesh",constants.siString,"")
	prop.AddParameter3("SkullDeform",constants.siString,"")
	prop.AddParameter3("PresetFile",constants.siString,"")
	prop.AddParameter3("PresetList",constants.siInt4,0,0,100,False,False)
	prop.AddParameter3("PresetMethod",constants.siInt4,0,0,3,False,False)
	prop.AddParameter3("HairGuide",constants.siString,"")
	prop.AddParameter3("HairRender",constants.siString,"")
	prop.AddParameter3("HairMaterial",constants.siString)
	prop.AddParameter3("HairLogo",constants.siString)
	
	prop.AddParameter3("DynamicsType",constants.siInt4,1,0,2,False,False)
	prop.AddParameter3("ApplyDynamicsMap",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("CreateMeshTarget",constants.siBool,False,False,True,False,False)
	prop.AddParameter3("TargetMesh",constants.siString)
	prop.AddParameter3("HeadBone",constants.siString)
	prop.AddParameter3("ColliderMesh",constants.siString)
	prop.AddParameter3("CollideChooser",constants.siInt4,0,0,10,False,False)
	
	prop.AddParameter2("SkullMapChooser",constants.siInt4,0,0,100,0,100,constants.siClassifUnknown,constants.siPersistable)
	prop.AddParameter2("CageMapChooser",constants.siInt4,0,0,100,0,100,constants.siClassifUnknown,constants.siPersistable)
	
	
def Hair_Groom_OnInit():
	prop = PPG.Inspected(0)
	Hair_Groom_RebuildLayout(prop)
	PPG.Refresh()
	
	
def Hair_Groom_RebuildLayout(prop):
	layout = prop.PPGLayout
	layout.Clear()
	
	layout.AddTab("Groom")
	pro.GetLogo(prop)
	
	layout.AddGroup("Skull Mesh")
	layout.AddRow()
	item = layout.AddItem("SkullMesh")
	item.SetAttribute(constants.siUINoLabel,True)
	layout.AddButton("SelectSkullMesh","Select")
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("UpVector Curve")
	layout.AddRow()
	item = layout.AddItem("UpVectorCurve")
	item.SetAttribute(constants.siUINoLabel,True)
	layout.AddButton("SelectUpVectorCurve","Select")
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("Tip Polygons Cluster")
	layout.AddRow()
	item = layout.AddButton("CreateTipPolygonsCluster","Create From Sel")
	item.SetAttribute(constants.siUICX,109)
	item.SetAttribute(constants.siUICY,25)
	item = layout.AddButton("AddPolygonsToTipPolygonsCluster","Add Sel")
	item.SetAttribute(constants.siUICX,59)
	item.SetAttribute(constants.siUICY,25)
	item = layout.AddButton("RemovePolygonsFromTipPolygonsCluster","Remove Sel")
	item.SetAttribute(constants.siUICX,79)
	item.SetAttribute(constants.siUICY,25)
	item = layout.AddButton("ShowTipPolygonsCluster","Show")
	item.SetAttribute(constants.siUICX,49)
	item.SetAttribute(constants.siUICY,25)
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("Tools")
	item = layout.AddButton("BuildHair","Build/Rebuild")
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,25)
	item = layout.AddButton("DeleteHair","Clean/Save Preset")
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,25)
	item = layout.AddButton("InspectHairGuide","Inspect Guide")
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,25)
	item = layout.AddButton("InspectHairRender","Inspect Render")
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,25)
	item = layout.AddButton("HideHairObjects","Hide/Unhide Objects")
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,25)
	item = layout.AddButton("BuildDebugEmitTriangles","Build Debug Triangles")
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,25)
	

	layout.EndGroup()
	
	if prop.Parameters("SkullMapChooser") and prop.Parameters("CageMapChooser"):
		layout.AddTab("Maps")
		layout.AddGroup("Skull Maps")
		#layout.AddTab("WeightMaps")
		skull = pro.FindChildFromParameterValue(prop,"SkullMesh")
		skulllist = []
		if skull:
			skulllist = pro.BuildListWeightMaps(skull, prop, "WeightMapCls")

		if len(skulllist) == 0:
			layout.AddStaticText("No skull or Skull contains no weightmap")
		else:
			item = layout.AddEnumControl("SkullMapChooser",skulllist,"List",constants.siControlListBox)
			item.SetAttribute(constants.siUINoLabel,True)
			btn = layout.AddButton("PaintSkullMap","Paint in Object View")
			btn.SetAttribute(constants.siUICX,300)
		layout.EndGroup()
		
		layout.AddGroup("Cage Maps")
		#layout.AddTab("WeightMaps")
		cage = prop.Parent3DObject
		cagelist = pro.BuildListWeightMaps(cage, prop, "WeightMapCls")

		if len(cagelist) == 0:
			layout.AddStaticText("Cage contains no weightmap right now...")
		else:
			item = layout.AddEnumControl("CageMapChooser",cagelist,"List",constants.siControlListBox)
			item.SetAttribute(constants.siUINoLabel,True)
			btn = layout.AddButton("PaintCageMap","Paint in Object View")
			btn.SetAttribute(constants.siUICX,300)
		layout.EndGroup()
	
	layout.AddTab("Dynamics")
	pro.GetLogo(prop)
	layout.AddGroup("Input Objects")
	dynamictype = prop.Parameters("DynamicsType").Value
	if dynamictype == 0:
		layout.AddGroup("Head Bone")
		layout.AddRow()
		item = layout.AddItem("HeadBone")
		item.SetAttribute(constants.siUINoLabel,True)
		layout.AddButton("PickHeadBone","Pick")
		layout.EndRow()
		layout.EndGroup()
	layout.AddGroup("Target Mesh")
	aMeshTargetOptions = ["Duplicate Mesh",0,"Pick Existing Mesh",1]
	item = layout.AddEnumControl("CreateMeshTarget",aMeshTargetOptions,"",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,True)
	if prop.Parameters("CreateMeshTarget").Value:
		layout.AddRow()
		item = layout.AddItem("TargetMesh")
		#item.SetAttribute(constants.siUINoLabel,True)
		layout.AddButton("PickTargetMesh","Pick")
		layout.EndRow()
	layout.EndGroup()
	#layout.AddGroup("Collider Mesh")
	#layout.AddRow()
	pro.CheckParameterExist(prop,"CollideChooser",constants.siInt4,True)
	
	collidelist = Hair_Groom_GetCollideList(prop)

	layout.AddGroup("Colliders List")
	item = layout.AddEnumControl("CollideChooser",collidelist,"",constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel,True)
	item.setAttribute(constants.siUICY,50)
	layout.EndGroup()
	layout.AddRow()
	btn = layout.AddButton("AddCollider","ADD")
	btn.SetAttribute(constants.siUICY,25)
	btn.SetAttribute(constants.siUICX,100)
	btn = layout.AddButton("RemoveCollider","REMOVE")
	btn.SetAttribute(constants.siUICY,25)
	btn.SetAttribute(constants.siUICX,100)
	btn = layout.AddButton("ShowCollider","SHOW")
	btn.SetAttribute(constants.siUICY,25)
	btn.SetAttribute(constants.siUICX,100)
	layout.EndRow()
	
	#item = layout.AddItem("ColliderMesh")
	#item.SetAttribute(constants.siUINoLabel,True)
	#layout.AddButton("PickColliderMesh","Pick")
	#layout.EndRow()
	#layout.EndGroup()
	layout.EndGroup()
	
	layout.AddGroup("Dynamics Map")
	layout.AddItem("ApplyDynamicsMap","Apply ICETree on Dynamics Map")
	layout.EndGroup()
	
	aDynamicsType = ["ICE SoftBody",0,"ICE Syflex",1,"Regular Syflex(NOT implemented!!)",2]
	layout.AddGroup("Choose Dynamics Type")
	item = layout.AddEnumControl("DynamicsType",aDynamicsType,"Type",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,True)
	layout.EndGroup()
	
	layout.AddGroup()
	layout.AddRow()
	item = layout.AddButton("CreateHairDynamics","Create Hair Dynamics")
	item.SetAttribute(constants.siUICY,25)
	item.SetAttribute(constants.siUICX,300)
	layout.EndRow()
	
	layout.EndGroup()
	
	layout.AddTab("Presets")
	pro.GetLogo(prop)
	
	layout.AddGroup("Method")
	aItems = ["AutoSaved",0,"Presets",1,"Save/Load",2]
	item = layout.AddEnumControl("PresetMethod",aItems,"Method",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,True)
	layout.EndGroup()
	
	v = prop.Parameters("PresetMethod").Value
	
	if v == 0:
		layout.AddGroup("AutoSaved")
		path = Hair_Groom_GetAutoSavePresetPath(prop)
		layout.AddStaticText("Prest File Saved as : \n"+path)
		layout.EndGroup()
		
	elif  v == 1:
		layout.AddGroup("Presets")
		aPresets = Hair_Groom_GetPresetList()
		layout.AddRow()
		item = layout.AddEnumControl("PresetList",aPresets,"Presets",constants.siControlListBox)
		item.SetAttribute(constants.siUINoLabel,True)
		button = layout.AddButton("ApplyPreset","Apply")
		button.SetAttribute(constants.siUICX,80)
		button.SetAttribute(constants.siUICY,120)
		layout.EndRow()
		layout.EndGroup()
		
	elif v == 2:
		layout.AddGroup("Preset")
		item = layout.AddEnumControl("PresetFile",[],"Preset",constants.siControlFilePath)
		item.SetAttribute(constants.siUIFileFilter,"XML files (*.xml)|*.xml:*.xml|All Files (*.*)|*.*||")
		item.SetAttribute(constants.siUINoLabel,True)
		item.SetAttribute(constants.siUIFileMustExist,True)
	
		#layout.AddSpacer()
		layout.AddRow()
		btn = layout.AddButton("SaveHairPreset","Save Preset");
		btn.SetAttribute(constants.siUICX,149)
		btn.SetAttribute(constants.siUICY,60)
		btn = layout.AddButton("LoadHairPreset","Load Preset");
		btn.SetAttribute(constants.siUICX,149)
		btn.SetAttribute(constants.siUICY,60)
		layout.EndRow()
		
		layout.EndGroup()
		
def Hair_Groom_PaintMap(map):
	object = map.Parent3DObject
	object.Properties("Visibility").Parameters("ViewVis").Value = True
	xsi.SelectObj(map)
	view = xsi.OpenView("Object View",True)
	view.SetAttributeValue("lockstatus",True)
	xsi.PaintTool()
	
def Hair_Groom_GetPresetList():
	path = pro.GetPluginPath("HairToolsPlugin")
	
	if path:
		presetpath = path.replace("Application\\Plugins\\Hair\\","Data\\Presets\\Hair")
		XSIUtils.EnsureFolderExists(presetpath)
		xsi.LogMessage(presetpath)
		regex = ".*\.xml$"
		files = xsi.FindFilesInFolder(presetpath,regex,True,False)
		
		i = 0
		for f in files:
			aPresets.append(f)
			aPresets.append(i)
			i+=1
		return aPresets
	return None
	
def Hair_Groom_DynamicsType_OnChanged():
	prop = PPG.Inspected(0)
	Hair_Groom_RebuildLayout(prop)
	PPG.Refresh()
	
def Hair_Groom_SkullMapChooser_OnChanged():
	prop = PPG.Inspected(0)

	uiitems = prop.PPGLayout.Item("SkullMapChooser").UIItems
	value = prop.Parameters("SkullMapChooser").Value
	skull = pro.FindChildFromParameterValue(prop,"SkullMesh")
	if not skull:
		return
		
	cls = skull.ActivePrimitive.Geometry.Clusters("WeightMapCls")

	if cls:
		wm = cls.LocalProperties(uiitems[value*2])
		if wm:
			xsi.SelectObj(wm)
			
		else:
			xsi.LogMessage("[Hair_Groom] Can't find WeightMap "+wmname+"...",constants.siWarning)

def Hair_Groom_CageMapChooser_OnChanged():
	prop = PPG.Inspected(0)

	uiitems = prop.PPGLayout.Item("CageMapChooser").UIItems
	value = prop.Parameters("CageMapChooser").Value
	cls = prop.Parent3DObject.ActivePrimitive.Geometry.Clusters("WeightMapCls")

	if cls:
		wm = cls.LocalProperties(uiitems[value*2])
		if wm:
			xsi.SelectObj(wm)
			
		else:
			xsi.LogMessage("[Hair_Groom] Can't find WeightMap "+wmname+"...",constants.siWarning)

def Hair_Groom_BuildDebugEmitTriangles_OnClicked():
	prop = PPG.Inspected(0)
	guide = pro.FindChildFromParameterValue(PPG.Inspected(0),"HairGuide")
	if guide:
		debug = guide.Parent3DObject.AddPrimitive("EmptyPolygonMesh","DebugEmitTriangles")
		tree = ice.CreateIceTree(debug,"DebugTriangles",0)
		compound = xsi.AddICECompoundNode("HairDebugEmitTriangles", str(tree))
		xsi.ConnectICENodes(str(tree)+".Port1", str(compound)+".Execute")
		mat = debug.AddMaterial("Lambert",False,"DebugTrianglesMaterial")
		xsi.SetInstanceDataValue("", str(mat)+".CAV", "Color")
		xsi.DisconnectICENodePort(str(compound)+".Set_Data.Value")
		xsi.ConnectICENodes(str(compound)+".Set_Data.Value", str(compound)+".RandomValueNode.value")
		
	else:
		xsi.LogMessage("[Hair_Groom] Build Debug Emit Triangles, no Guide Cloud found, Sorry...", constants.siWarning)
		
	
def Hair_Groom_PaintCageMap_OnClicked():
	prop = PPG.Inspected(0)

	uiitems = prop.PPGLayout.Item("CageMapChooser").UIItems
	value = prop.Parameters("CageMapChooser").Value
	cls = prop.Parent3DObject.ActivePrimitive.Geometry.Clusters("WeightMapCls")

	if cls:
		wm = cls.LocalProperties(uiitems[value*2])
		if wm:
			Hair_Groom_PaintMap(wm)

def Hair_Groom_PaintSkullMap_OnClicked():
	prop = PPG.Inspected(0)

	uiitems = prop.PPGLayout.Item("CageMapChooser").UIItems
	value = prop.Parameters("CageMapChooser").Value
	skull = pro.FindChildFromParameterValue(prop,"SkullMesh")
	if not skull:
		return
	cls = skull.ActivePrimitive.Geometry.Clusters("WeightMapCls")

	if cls:
		wm = cls.LocalProperties(uiitems[value*2])
		if wm:
			Hair_Groom_PaintMap(wm)

def Hair_Groom_PresetMethod_OnChanged():
	Hair_Groom_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()

def Hair_Groom_SelectUpVectorCurve_OnClicked():
	pick = uti.PickElement(constants.siCurveFilter,"Pick UpVector Curve")
	model = pro.FindPPGModel(PPG.Inspected(0))
	if pick and pick.Model.Name == model.Name:
		PPG.UpVectorCurve.Value = pick.Name
	else:
		xsi.LogMessage("Ensure the UpVector Curve is under the same model as the Hair Cage",constants.siWarning)
		
def Hair_Groom_SelectSkullMesh_OnClicked():
	pick = uti.PickElement(constants.siPolyMeshFilter,"Pick Skull Mesh")
	model = pro.FindPPGModel(PPG.Inspected(0))
	if pick and pick.Model.Name == model.Name:
		PPG.SkullMesh.Value = pick.Name
	else:
		xsi.LogMessage("Ensure the Skull Object is under the same model as the Hair Cage",constants.siWarning)
		
def Hair_Groom_CreateTipPolygonsCluster_OnClicked():
	Hair_CreateTipPolygonsCluster()
	
def Hair_Groom_AddPolygonsToTipPolygonsCluster_OnClicked():
	Hair_Groom_WriteXML()
	Hair_AddPolygonsToTipPolygonsCluster(PPG.Inspected(0))
	Hair_Groom_ReadXML()
	
def Hair_Groom_RemovePolygonsFromTipPolygonsCluster_OnClicked():
	Hair_Groom_WriteXML()
	Hair_RemovePolygonsFromTipPolygonsCluster(PPG.Inspected(0))
	Hair_Groom_ReadXML()
	
def Hair_Groom_ShowTipPolygonsCluster_OnClicked():
	obj = PPG.Inspected(0).Parent3DObject
	geo = obj.ActivePrimitive.Geometry
	cls = geo.Clusters("TipPolygons")
	if cls:
		cls = xsi.Dictionary.GetObject(cls)
		xsi.SelectObj(cls)

	else:
		XSIUIToolkit.MsgBox("TipPolygons cluster doesn't exists on "+ obj.FullName ,constants.siMsgOkOnly,"HairTools")
	
def Hair_Groom_BuildHair_OnClicked():
	valid = Hair_EmitFromMesh(PPG.Inspected(0))

def Hair_Groom_DeleteHair_OnClicked():
	Hair_Groom_WriteXML()
	Hair_CleanUp(PPG.Inspected(0))

def Hair_Groom_InspectHairGuide_OnClicked():
	guide = pro.FindChildFromParameterValue(PPG.Inspected(0),"HairGuide")
	tree = guide.ActivePrimitive.ICETrees("Emit")
	node = tree.Nodes.Find("HairEmitGuideFromMesh2")
	xsi.InspectObj(node,"","",constants.siLock)
	
def Hair_Groom_InspectHairRender_OnClicked():
	render = pro.FindChildFromParameterValue(PPG.Inspected(0),"HairRender")
	tree = render.ActivePrimitive.ICETrees("Emit")
	node = tree.Nodes.Find("HairRenderEmit")
	xsi.InspectObj(node,"","",constants.siLock)
	
def Hair_Groom_HideHairObjects_OnClicked():
	prop = PPG.Inspected(0)
	obj = prop.Parent3DObject
	model = obj.Model
	grp = model.Groups("HairHidden")
	if grp:
		xsi.ToggleVisibility(grp)
	else:
		xsi.LogMessage("[Hair_Groom] There is no HairHidden Group under "+ model.FullName,constants.siError)
	
def Hair_Groom_WriteXML(filename=None):
	prop = PPG.Inspected(0)
	if not filename:
		filename = Hair_Groom_GetXMLFile(prop)

	nodes = Hair_GetCompoundNodes(prop)
	
	if nodes:	
		xml.WriteCompoundsXMLPreset(filename,nodes)

def Hair_Groom_ReadXML(filename=None):
	prop = PPG.Inspected(0)
	
	if not filename:
		filename = Hair_Groom_GetAutoSavePresetPath(prop)

	nodes = Hair_GetCompoundNodes(prop)
	
	if nodes:	
		xml.ReadCompoundsXMLPreset(filename,nodes)
		
def Hair_Groom_GetXMLFile(prop):
	if prop.Parameters("PresetFile").Value == "":
		filename = Hair_Groom_GetAutoSavePresetPath(prop)
		return filename
	else:
		filename = prop.Parameters("PresetFile").Value
		if not xml.CheckFileExists(filename):
			prop.Parameters("PresetFile").Value = ""
			Hair_Groom_GetXMLFile(prop)
		else:
			return filename
			
def Hair_Groom_GetAutoSavePresetPath(PPG):
	path = xsi.ActiveProject2.Path
	obj = PPG.Parent3DObject
	presetfolder = XSIUtils.BuildPath(path,"Presets","Hair")
	XSIUtils.EnsureFolderExists(presetfolder,false)
	presetpath = XSIUtils.BuildPath(presetfolder,obj.Name+".xml")
	return presetpath
	
def Hair_Groom_SaveHairPreset_OnClicked():
	path = PPG.PresetFile.Value
	if path:
		Hair_Groom_WriteXML(path)

def Hair_Groom_LoadHairPreset_OnClicked():
	path = PPG.PresetFile.Value
	if path:
		Hair_Groom_ReadXML(path)
		
def Hair_GetCompoundNodes(prop):
	nodes = []
		
	guide = pro.FindChildFromParameterValue(prop,"HairGuide")
	if not guide:
		xsi.LogMessage("[Hair_GetCompoundNodes] Cant't find corresponding Hair Guide Cloud!!")
	else:
		tree = guide.ActivePrimitive.ICETrees("Emit")
		if tree:
			node = tree.CompoundNodes.Find("HairEmitGuideFromMesh2")
			if node:
				nodes.append(node)
				
	render = pro.FindChildFromParameterValue(prop,"HairRender")
	if not render:
		xsi.LogMessage("[Hair_GetCompoundNodes] Cant't find corresponding Hair Render Cloud")
	
	else:
		tree = render.ActivePrimitive.ICETrees("Emit")
		if tree:
			node = tree.CompoundNodes.Find("HairRenderEmit")
			nodes.append(node)
			
	if len(nodes)>0:
		return nodes
	else:
		return None
	
def Hair_Groom_CreateMeshTarget_OnChanged():
	Hair_Groom_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
	
def Hair_Groom_PickTargetMesh_OnClicked():
	pick = uti.PickElement(constants.siPolyMeshFilter,"Pick Target Mesh")
	if pick:
		if not pick.Model.Name == PPG.Inspected(0).Parent3DObject.Model.Name:
			xsi.LogMessage("[Hair_Groom] Target Mesh have to be under same model as Hair Cage object!!", constants.siWarning)
			return 
		PPG.TargetMesh.Value = pick.Name
		
def Hair_Groom_PickHeadBone_OnClicked():
	pick = uti.PickElement(constants.siObjectFilter,"Pick Head Bone")
	if pick:
		if not pick.Model.Name == PPG.Inspected(0).Parent3DObject.Model.Name:
			xsi.LogMessage("[Hair_Groom] Head Bone have to be under same model as Hair Cage object!!", constants.siWarning)
			return 
			
		PPG.HeadBone.Value = pick.Name
		PPG.Refresh()

def Hair_Groom_PickColliderMesh_OnClicked():
	pick = uti.PickElement(constants.siPolyMeshFilter,"Pick Collider Mesh")
	if pick:
		if not pick.Model.Name == PPG.Inspected(0).Parent3DObject.Model.Name:
			xsi.LogMessage("[Hair_Groom] Collider have to be under same model as Hair Cage object!!", constants.siWarning)
			return 
		
		
		PPG.ColliderMesh.Value = pick.Name

#--------------------------------------------
# Collider Gestion
#--------------------------------------------
def Hair_Groom_GetCollideList(prop):
	model = prop.Parent3DObject.Model
	if not model.Groups("HairCollide"):
		uti.GroupSetup(model,None,"HairCollide")
		return []

	else:
		grp = model.Groups("HairCollide")
		members = grp.Members
		if members.Count == 0:
			xsi.LogMessage("[Hair_Control_GetCollideList] HairCollide Group is Empty!!", constants.siWarning)
			return []
		else:
			list = []
			id = 0
			for m in members:
				list.append(m.Name)
				list.append(id)
				id = id+1
			return list

def Hair_Groom_AddCollider_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject.Model
	pick = uti.PickElement(constants.siPolyMeshFilter,"Pick Polymesh Collider")

	if pick:
		uti.GroupSetup(model,[pick],"HairCollide")
		Hair_Groom_RebuildLayout(prop)
		PPG.Refresh()

def Hair_Groom_RemoveCollider_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject.Model
	grp = model.Groups("HairCollide")
	if not grp:
		return
		
	items = PPG.PPGLayout.Item("CollideChooser").UIItems
	value = prop.Parameters("CollideChooser").Value
	
	collider = model.FindChild2(items[value*2],constants.siPolyMeshType,constants.si3DObjectFamily,True)
	if collider:
		grp.RemoveMember(collider)
		Hair_Groom_RebuildLayout(prop)
		PPG.Refresh()
		
def Hair_Groom_CreateHairDynamics_OnClicked():
	prop = PPG.Inspected(0)
	mesh = prop.Parent3DObject
	model = mesh.Model
	parent = mesh.Parent3DObject
	
	#get dynamics type
	dynamictype = prop.Parameters("DynamicsType").Value
	if type == 2:
		xsi.LogMessage("[Hair_DynamicsProperty] Regular Syflex NOT yet implemented, you have to do it manually...", constants.siError)
		return
	
	#duplicate hair mesh to be used as dynamics target
	dup = prop.Parameters("CreateMeshTarget").Value
	target = None
	if not dup:
		data = mesh.ActivePrimitive.Geometry.Get2()
		target = xsi.ActiveSceneRoot.AddPolygonMesh (data[0], data[1])
		target.Name = mesh.Name+"_Target"
		parent.AddChild(target)
	else:
		target = None
		try:
			target = model.FindChild(prop.Parameters("TargetMesh").Value)
		except:
			xsi.LogMessage("[Hair_DynamicsProperty] Target Mesh doesn't exist : we create it...",constants.siInfo)
			data = mesh.ActivePrimitive.Geometry.Get2()
			target = xsi.ActiveSceneRoot.AddPolygonMesh (data[0], data[1])
			target.Name = mesh.Name+"_Target"
			parent.AddChild(target)
	
	
	#get head bone
	hn = prop.Parameters("HeadBone").Value
	headbone = None
	if hn:
		try:
			headbone = model.FindChild(hn)
		except:
			pass
		
	if not headbone and dynamictype==0:
		headbone = model.AddNull("Hair_Driver")
	
	#get collider object
	colliderName = prop.Parameters("ColliderMesh").Value
	
	collider = None
	if colliderName:
		collider = model.FindChild(colliderName)
	
	setmap = prop.Parameters("ApplyDynamicsMap").Value
		
	xsi.Hair_ApplyDynamics(mesh,target,collider,headbone,dynamictype,setmap)
#---------------------------------------
# Create Hair Dynamics Command 
#---------------------------------------
def Hair_ApplyDynamics_Init(in_ctxt):
	cmd = in_ctxt.Source
	cmd.Description = ""
	cmd.ReturnValue = true

	args = cmd.Arguments
	args.Add("mesh",constants.siArgumentInput)
	args.Add("target",constants.siArgumentInput)
	args.Add("collider",constants.siArgumentInput)
	args.Add("headbone",constants.siArgumentInput)
	args.Add("dynamictype",constants.siArgumentInput)
	args.Add("setmap",constants.siArgumentInput)
	
	return true

def Hair_ApplyDynamics_Execute(mesh,target,collider,headbone,dynamictype,setmap):
	model = mesh.Model
	parent = mesh.Parent3DObject

	#if no target mesh duplicate hair mesh to be used as dynamics target
	if not target:
		data = mesh.ActivePrimitive.Geometry.Get2()
		target = xsi.ActiveSceneRoot.AddPolygonMesh (data[0], data[1])
		target.Name = mesh.Name+"_Target"
		parent.AddChild(target)
			
	#get head boneif softbody dynamic type 
	if not headbone and dynamictype == 0:
		headbone = model.AddNull("Hair_Driver")
		
	#set dynamics weight map
	xsi.FreezeModeling(mesh)
	
	dynamicsmap = uti.GetWeightMap(mesh,"Dynamics_Map",1,0,1)
	blendmap = uti.GetWeightMap(mesh,"Blend_Map",1,0,1)
	
	# Set Dynamic Map
	if setmap:
		tree = ice.CreateIceTree(mesh,"SetDynamicsMap",0)
		compound = xsi.AddICECompoundNode("HairTipGradientMap", str(tree))
		xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")
		if type<2:
			xsi.LogMessage("Set Dynamic Map Value")
			xsi.SetValue(str(compound)+".Out_Start", 0, "")
			xsi.SetValue(str(compound)+".Out_End", 1, "")
		
		compound = xsi.AddICECompoundNode("HairSmoothDynamicMap", str(tree))
		xsi.AddPortToICENode(str(tree)+".port1", "siNodePortDataInsertionLocationAfter")
		xsi.ConnectICENodes(str(tree)+".port2", str(compound)+".Execute")
		
	#Init Dynamics ICETree
	tree = ice.CreateIceTree(mesh,"InitDynamics",2)
	compound = xsi.AddICECompoundNode("ClothInitShape", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")
	xsi.SetValue(str(compound)+".Reference", ice.ReplaceModelNameByThisModel(target, model), "")
	
	# create pin/mimic cluster if not exists
	pincls = mesh.ActivePrimitive.Geometry.Clusters("Pin_Cls") 
	mimiccls = mesh.ActivePrimitive.Geometry.Clusters("Mimic_Cls") 
	if not pincls or not mimiccls:
		sub = []
		add = []
		for p in mesh.ActivePrimitive.Geometry.Points:
			if p.IsBoundary:
				sub.append(p.Index)
			else:
				add.append(p.Index)
		
		if not pincls:
			pincls = mesh.ActivePrimitive.Geometry.AddCluster(constants.siVertexCluster,"Pin_Cls",sub)
		if not mimiccls:
			mimiccls = mesh.ActivePrimitive.Geometry.AddCluster(constants.siVertexCluster,"Mimic_Cls",add)
		
	else:
		xsi.LogMessage("Pin_Cls/Mimic_Cls already in place on "+mesh.FullName+" : creation skipped...")

	
	#ICE softbody dynamics
	if dynamictype == 0:
			
		# init softbody ICETree
		tree = ice.CreateIceTree(mesh,"SoftbodyInit",1)
		compound = xsi.AddICECompoundNode("Init Verlet Geometry Data", str(tree))
		xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")
		
		tree = ice.CreateSimulatedIceTree(mesh,"SoftbodyDynamics")
		
		compound = xsi.AddICECompoundNode("HairSoftBodyDynamics", str(tree))
		xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")
		
		get1 = xsi.AddICENode("GetDataNode", str(tree))
		get2 = xsi.AddICENode("GetDataNode", str(tree))
		get3 = xsi.AddICENode("GetDataNode", str(tree))
		
		xsi.ConnectICENodes(str(compound)+".Target_Name", str(get1)+".OutName")
		xsi.ConnectICENodes(str(compound)+".Head_Name", str(get2)+".OutName")
		xsi.ConnectICENodes(str(compound)+".Collider_Name", str(get3)+".OutName")

		xsi.SetValue(str(get1)+".reference", ice.ReplaceModelNameByThisModel(target,model), "")
		xsi.SetValue(str(get2)+".reference", ice.ReplaceModelNameByThisModel(headbone,model), "")
		
		if collider:
			uti.GroupSetup(model,[collider],"HairCollide")
			
		xsi.SetValue(str(get3)+".reference", "this_model.HairCollide", "")
		
	#ICE syflex dynamics		
	if dynamictype == 1:
		
		tree = ice.CreateSimulatedIceTree(mesh,"SyflexDynamics")
		compound = xsi.AddICECompoundNode("HairSyflexDynamics", str(tree))
		xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")
		
		get1 = xsi.AddICENode("GetDataNode", str(tree))
		get2 = xsi.AddICENode("GetDataNode", str(tree))
		get3 = xsi.AddICENode("GetDataNode", str(tree))
		get4 = xsi.AddICENode("GetDataNode", str(tree))

		xsi.ConnectICENodes(str(compound)+".Target_Name", str(get1)+".OutName")
		xsi.ConnectICENodes(str(compound)+".Collider_Name", str(get2)+".OutName")
		xsi.ConnectICENodes(str(compound)+".Wind_Object", str(get3)+".OutName")
		xsi.ConnectICENodes(str(compound)+".Turbulence_Object", str(get4)+".OutName")

		xsi.SetValue(str(get1)+".reference", ice.ReplaceModelNameByThisModel(target,model), "")
		
		if collider:
			uti.GroupSetup(model,[collider],"HairCollide")
			
		xsi.SetValue(str(get2)+".reference", "this_model.HairCollide", "")
			
		# connect wind
		wind = model.FindChild("HairWindControl")
		if not wind:
			wind = uti.AddNull(parent,10,1.5,"HairWindControl",0,1,0.5)
		xsi.SetValue(str(get3)+".reference", ice.ReplaceModelNameByThisModel(wind,model), "")
		
		#connect turbulence
		turb = model.FindChild("HairTurbulenceControl")
		if not turb:
			turb = wind.AddGeometry("Circle","NurbsCurve","HairTurbulenceControl")
			turb.Parameters("Radius").Value = 0.77
			turb.Parameters("SubdivU").Value = 24
			tree = ice.CreateIceTree(turb,"DeformIcon",0)
			compound = xsi.AddICECompoundNode("HairTurbulenceIcon", str(tree))
			xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Execute")
			uti.SetWireColor(turb,.3,1,.8)	
			xsi.FreezeObj(turb)
		xsi.SetValue(str(get4)+".reference", ice.ReplaceModelNameByThisModel(turb,model), "")
		
	# Restore Shape ICE Tree
	Hair_ApplyRestoreOp(mesh, target)

#---------------------------------------
# Create Tip Polygons Cluster  
#---------------------------------------
def Hair_CreateTipPolygonsCluster():
	sel = xsi.Selection(0)
	if not sel.Type == "polySubComponent":
		XSIUIToolkit.MsgBox("Select Some Polygons on Hair Mesh and try again...",constants.siMsgOkOnly,"HairTools")
		return
	
	obj = sel.SubComponent.Parent3DObject
	if obj.ActivePrimitive.Geometry.Clusters("TipPolygons"):
		XSIUIToolkit.MsgBox("TipPolygons cluster already exists on "+ obj.FullName +" : Use Add or Remove Polygons from TipPolygons Cluster instead...",constants.siMsgOkOnly,"HairTools")
		return

	cls = sel.SubComponent.CreateCluster("TipPolygons")
	xsi.SelectObj(cls)
	
#---------------------------------------
# Add Polygons To Tip Polygons Cluster
#---------------------------------------
def Hair_AddPolygonsToTipPolygonsCluster(PPG):
	
	sel = xsi.Selection(0)
	if not sel.Type == "polySubComponent":
		XSIUIToolkit.MsgBox("Select Some Polygons on Hair Mesh and try again...",constants.siMsgOkOnly,"HairTools")
		return
	
	sub = sel.SubComponent
	obj = sub.Parent3DObject
	
	cls = obj.ActivePrimitive.Geometry.Clusters("TipPolygons")
	if not cls:
		XSIUIToolkit.MsgBox("TipPolygons cluster already exists on "+ obj.FullName +" : Use Create TipPolygons Cluster instead...",constants.siMsgOkOnly,"HairTools")
		return

	# we first delete existing hair setup
	exist = Hair_CleanUp(PPG)
	
	# we add selected polygons
	cls = obj.ActivePrimitive.Geometry.Clusters("TipPolygons")
	uti.UpdateClusterComponent(cls,sub,1)
	
	if exist:
		# we rebuild hair setup
		Hair_EmitFromMesh(PPG)
	
#---------------------------------------
# Remove Polygons From Tip Polygons Cluster
#---------------------------------------
def Hair_RemovePolygonsFromTipPolygonsCluster(PPG):
	sel = xsi.Selection(0)
	if not sel.Type == "polySubComponent":
		XSIUIToolkit.MsgBox("Select Some Polygons on Hair Mesh and try again...",constants.siMsgOkOnly,"HairTools")
		return
	
	sub = sel.SubComponent
	obj = sub.Parent3DObject
	cls = obj.ActivePrimitive.Geometry.Clusters("TipPolygons")

	if not cls:
		XSIUIToolkit.MsgBox("TipPolygons cluster already exists on "+ obj.FullName +" : Use Create TipPolygons Cluster instead...",constants.siMsgOkOnly,"HairTools")
		return

	# we first delete existing hair setup
	exist = Hair_CleanUp(PPG)
	
	# we remove selected polygons
	cls = obj.ActivePrimitive.Geometry.Clusters("TipPolygons")
	uti.UpdateClusterComponent(cls,sub,2)
	
	if exist:
		# we rebuild hair setup
		Hair_EmitFromMesh(PPG)

#---------------------------------------
# Merge Selected Hair Meshes
#---------------------------------------
def Hair_MergeMeshes_Execute():
	sel = xsi.Selection
	if not Hair_CheckSelection(sel):
		return False
	
	tipsmerged = Hair_GetTipPolygonsIndices(sel)
	op = xsi.ApplyGenOp("MeshMerge", "", sel, 3, "siPersistentOperation", "siKeepGenOpInputs", "")(0)
	op.Parameters("Tolerance").Value = 0
	mesh = op.Parent3DObject
	mesh.Name = "Hair_Mesh_Merged"
	mesh.ActivePrimitive.Geometry.AddCluster(constants.siPolygonCluster,"TipPolygons",tipsmerged)
	xsi.FreezeObj(mesh)

	return True

#---------------------------------------
# Create Hair From Selected Mesh
#---------------------------------------
def Hair_EmitFromMesh(prop):
	mesh = prop.Parent3DObject
	model = mesh.model
	root = mesh.Parent3DObject
	tohide = [mesh]
	tocache = [mesh]

	cls = mesh.ActivePrimitive.Geometry.Clusters("TipPolygons")
	if not cls:
		XSIUIToolkit.MsgBox("No TipPolygon Cluster on selected Geometry \nCreate Hairs From Selected Mesh aborted...",constants.siMsgOkOnly,"HairTools")
		return False
	else:
		if not Hair_CheckGeometry(mesh):
			return False
		
		skull = pro.FindChildFromParameterValue(prop,"SkullMesh")
		upvcrv = pro.FindChildFromParameterValue(prop,"UpVectorCurve")
		
		if not skull:
			xsi.LogMessage("[HairTools] You MUST select Skull Object before building hair!!", constants.siWarning)
			prop.SkullMesh.Value = ""
			return None
			
		tohide.append(skull)
		tocache.append(skull)
		if upvcrv:
			tohide.append(upvcrv)
			
		#add FrizzMap and SpreadMap on Cage
		spread = uti.GetWeightMap(mesh,"Spread_Map",1,0,1)
		frizz = uti.GetWeightMap(mesh,"Frizz_Map",1,0,1)
	
		# add density weight map on skull
		density = uti.GetWeightMap(skull,"Density_Map",1,0,1)
		cut = uti.GetWeightMap(skull,"Cut_Map",1,0,1)
		frizz = uti.GetWeightMap(skull,"Frizz_Map",1,0,1)
		clone = uti.GetWeightMap(skull,"Clone_Map",1,0,1)
		spread = uti.GetWeightMap(skull,"Spread_Map",1,0,1)
		
		xsi.FreezeModeling(mesh.FullName+","+skull.FullName)
		
		# get Skull Static and transfert density
		skullstatic = uti.GetMeshDuplicate(xsi.Dictionary.GetObject(skull))
		tree = ice.CreateIceTree(skullstatic,"TransfertDensity",0)
		compound = xsi.AddICECompoundNode("HairTransfertDensityMap",str(tree))
		xsi.ConnectICENodes(str(tree)+".port1",str(compound)+".Execute")
		get = xsi.AddICENode("GetDataNode", str(tree))
		xsi.SetValue(str(get)+".reference", ice.ReplaceModelNameByThisModel(skull,model), "")
		xsi.ConnectICENodes(str(compound)+".Skull_Object", str(get)+".outname")
		tohide.append(skullstatic)
		
		if not mesh.ActivePrimitive.ICETrees("GetMeshData"):
			tree = ice.CreateIceTree(mesh,"GetMeshData",1)
			compound = xsi.AddICECompoundNode("HairGetMeshData", str(tree))
			xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".execute")
		
		if not mesh.ActivePrimitive.ICETrees("SetMeshData"):
			tree = ice.CreateIceTree(mesh,"SetMeshData",3)
			compound = xsi.AddICECompoundNode("HairSetMeshData", str(tree))
			xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".execute")
		
		guide = root.AddPrimitive("PointCloud","Hair_Guide")
		tohide.append(guide)
		prop.Parameters("HairGuide").Value = guide.Name
		tree = ice.CreateIceTree(guide,"Emit",0)
		
		compound = xsi.AddICECompoundNode("HairEmitGuideFromMesh2", str(tree))
		xsi.ConnectICENodes(str(tree)+".port"+str(1), str(compound)+".execute")
		get1 = xsi.AddICENode("GetDataNode", str(tree))
		xsi.SetValue(str(get1)+".reference", str(mesh), "")
		xsi.ConnectICENodes(str(compound)+".Cage_Mesh", str(get1)+".outname")
		
		render = root.AddPrimitive("PointCloud","Hair_Render")
		prop.Parameters("HairRender").Value = render.Name
		tree = ice.CreateIceTree(render,"Emit",0)
		
		compound = xsi.AddICECompoundNode("HairRenderEmit", str(tree))
		xsi.ConnectICENodes(str(tree)+".port"+str(1), str(compound)+".execute")
	
		get1 = xsi.AddICENode("GetDataNode", str(tree))
		xsi.SetValue(str(get1)+".reference", str(ice.ReplaceModelNameByThisModel(skullstatic,model)), "")
		xsi.ConnectICENodes(str(compound)+".Emitter_Static", str(get1)+".outname")
		
		get2 = xsi.AddICENode("GetDataNode", str(tree))
		xsi.SetValue(str(get2)+".reference", str(ice.ReplaceModelNameByThisModel(skull,model)), "")
		xsi.ConnectICENodes(str(compound)+".Emitter_Deform", str(get2)+".outname")
		
		get3 = xsi.AddICENode("GetDataNode", str(tree))
		xsi.SetValue(str(get3)+".reference", str(ice.ReplaceModelNameByThisModel(guide,model)), "")
		xsi.ConnectICENodes(str(compound)+".Guide", str(get3)+".outname")
		
		get4 = xsi.AddICENode("GetDataNode", str(tree))
		if upvcrv:
			xsi.SetValue(str(get4)+".reference", ice.ReplaceModelNameByThisModel(upvcrv,model), "")
		xsi.ConnectICENodes(str(compound)+".UpVector_Curve", str(get4)+".outname")	
		
		Hair_Groom_ReadXML()
		uti.GroupSetup(mesh.Model,tohide,"HairHidden")
		uti.GroupSetup(mesh.Model,[render],"HairRender")
		uti.GroupSetup(mesh.Model,tocache,"PointCache")
		
		# material 
		matName = prop.Parameters("HairMaterial").Value
		mat = None
		try:
			mat = xsi.Dictionary.GetObject(matName)
			render.SetMaterial(mat)

		except:
			xsi.LogMessage("[Hair_Groom] Create New Hair Material")
			mat = render.AddMaterial("Phong",false,"Hair_Mat")	
			shader = xsi.CreateShaderFromProgID("Softimage.rh_renderer.1.0", mat.FullName , "Hair")
			xsi.SIConnectShaderToCnxPoint(shader.FullName+".out", mat.FullName+".surface", False)
			xsi.SIConnectShaderToCnxPoint(shader.FullName+".out", mat.FullName+".shadow", False)
			photon = mat.Parameters("Photon")
			phong = photon.NestedObjects(0)
			if phong:
				xsi.DeleteObj(phong)
			
			lib = mat.Library
			prop.Parameters("HairMaterial").Value = lib.FullName+"."+mat.Name
			
			
	return True

#---------------------------------------
# Delete Hair From Selected Mesh
#---------------------------------------
def Hair_CleanUp(prop):
	mesh = prop.Parent3DObject
	skull = pro.FindChildFromParameterValue(prop,"SkullMesh")
	if skull:
		model = mesh.Model
		skullstatic = model.FindChild(skull.Name+"_Static")
		if skullstatic:
			xsi.DeleteObj(skullstatic)
		
	if not mesh.ActivePrimitive.ICETrees("SetMeshData"):
		xsi.LogMessage(mesh.FullName+" doesn't have HairMeshData ICETree on it...")
		return False
	
	done = Hair_DeleteHairCloud(prop)
	
	if done:
		todelete = []
		
		for h in mesh.ActivePrimitive.ConstructionHistory:
			if(h.FullName.find("marker") == -1\
			and not h.Type == "envelopop"\
			and not h.Type == "clsctr")\
			or h.Name == "Simulation":
				todelete.append(h)
			
		xsi.DeleteObj(todelete)
	
		return True
	else:
		return False

#---------------------------------------
# Utilities
#---------------------------------------
def Hair_DeleteHairCloud(prop):
	Hair_Groom_WriteXML()
	
	guide = pro.FindChildFromParameterValue(prop,"HairGuide")
	render = pro.FindChildFromParameterValue(prop,"HairRender")
	
	if not guide or not render:
		xsi.LogMessage("[Hair_DeleteHairCloud] Can't find corresponding Hair Guide or Hair Render ---> Deletion aborted...",constants.siWarning)
		xsi.LogMessage("[Hair_DeleteHairCloud] You'll have to delete them yourself...",constants.siWarning)
		return False
		
	else:
		#store material info
		mat = render.Materials(0)
		lib = mat.Library
		
		prop.Parameters("HairMaterial").Value = lib.FullName+"."+mat.Name
		
		#delete point clouds
		xsi.DeleteObj(guide.FullName+","+render.FullName)
		return True
					
def Hair_IsMeshEqual(inMesh,inNodeValue):
	if inMesh.FullName == inNodeValue:
		return True
	
	model = inMesh.Model
	s = inMesh.FullName.replace(model.Name,"this_model")
	if s == inNodeValue:
		return True
		
	return false
	
def Hair_IsHairCloud(inObj,compoundName):
	if not inObj.Type == "pointcloud":
		return False
	
	trees = inObj.ActivePrimitive.ICETrees
	if trees.Count == 0:
		return False
	
	for t in trees:
		if t.Name == "Emit":
			for n in t.Nodes:
				if n.Name == compoundName:
					return True		
	return False
	
def Hair_GetTipPolygonsIndices(sel):
	tips = []
	offset = 0
	
	for s in sel:
		cls = s.ActivePrimitive.Geometry.Clusters("TipPolygons")
		elems = cls.Elements.Array
		for e in elems:
			tips.append(e+offset)
		offset += s.ActivePrimitive.Geometry.Polygons.Count
		
	return tips
	
def Hair_CheckSelection(sel):
	for s in sel:
		mesh = xsi.Selection(0)
		if not s.type=="polymsh":
			xsi.LogMessage("Select Polygon Meshes you want to merge")
			return false
			
		cls = mesh.ActivePrimitive.Geometry.Clusters("TipPolygons")
		if not cls:
			xsi.LogMessage("No TipPolygon Cluster on selected Geometry : Add Hair From Mesh aborted...")
			return false
	return true
	
def Hair_CheckGeometry(inMesh):
	# check if geometry have triangles or N-gons outside of TipPolygons cluster
	geom = inMesh.ActivePrimitive.Geometry
	polygons = geom.facets
	tippolygons = geom.Clusters("TipPolygons")
	sub = geom.CreateSubComponent(constants.siPolygonCluster)
	check = False
	for p in polygons:
		if not p.NbPoints == 4:
			#check if belongs to TipPolygons Cluster(if NOT, geo is invalid...)
			if tippolygons.FindIndex(p.Index) == -1:
				sub.AddElement(p.Index)
				check = True
	
	if check:
		XSIUIToolkit.MsgBox("Selected Geometry Contains Triangles or N-Gons outside of TipPolygons cluster ---> Invalid Geometry...",constants.siMsgOkOnly,"HairTools")
		xsi.SelectObj(sub)
		return		

	return True
	
def Hair_ApplyRestoreOp(obj, target):
	restoremap = uti.GetWeightMap(obj,"Restore_Map",0,0,1)
	tree = ice.CreateIceTree(obj,"DynamicsRestore",3)
	xsi.MoveOperatorAfter(obj.ActivePrimitive, tree, str(obj.ActivePrimitive)+".simulationmarker")

	compound = xsi.AddICECompoundNode("HairRestoreShape", str(tree))
	xsi.ConnectICENodes(str(tree)+".port1", str(compound)+".Result")
	xsi.SetValue(str(compound)+".Target_Mesh_Reference", ice.ReplaceModelNameByThisModel(target,obj.model), "")
