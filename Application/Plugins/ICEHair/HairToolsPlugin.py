from win32com.client import constants
from Globals import XSI
from Globals import XSIUtils
from Globals import XSIUIToolKit
import Utils
import ICETree
import Property


def XSILoadPlugin(in_reg):
	in_reg.Author = 'benmalartre'
	in_reg.Name = 'HairTools'
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterCommand('Hair_MergeMeshes', 'Hair_MergeMeshes')
	in_reg.RegisterProperty('Hair_Groom')
	
	in_reg.RegisterCommand('Hair_ApplyDynamics', 'Hair_ApplyDynamics')
	in_reg.RegisterMenu(constants.siMenuTbICEParticlesCreateID, 'HairTools', False, False)


def HairTools_Init(context):
	menu = context.Source
	item = menu.AddItem('Hair', constants.siMenuItemSection)
	item.SetBackgroundColor(178, 191, 194)
	menu.AddCallBackItem('Hair Groom', 'Hair_GetGroomProperty')
	menu.AddCallBackItem('Help Page', 'Hair_GetHelpPage')


def Hair_GetGroomProperty(context):
	mesh = XSI.Selection(0)
	if not mesh or not mesh.Type == 'polymsh':
		XSIUIToolKit.MsgBox('Select PolygonMesh and try again...', constants.siMsgOkOnly, 'HairTools')
		return
	prop = mesh.Properties('Hair_Groom')
	if not prop:
		prop = mesh.AddProperty('Hair_Groom')
	
	Utils.GetWeightMap(mesh, 'Frizz_Map', 1, 0, 1)
	Utils.GetWeightMap(mesh, 'Spread_Map', 1, 0, 1)
	XSI.InspectObj(prop, '', '', constants.siLock)


def Hair_GetHelpPage(context):
	plugin_path = Property.GetPluginPath('HairTools')
	workgroup_path = plugin_path.split('Application')[0]
	doc = XSIUtils.BuildPath(workgroup_path, 'docs', 'Hair_documentation.html')
	XSI.OpenNetView(doc)
	

def Hair_Groom_Define(context):
	prop = context.Source
	prop.AddParameter3('UpVectorCurve', constants.siString, '')
	prop.AddParameter3('SkullMesh', constants.siString, '')
	prop.AddParameter3('SkullDeform', constants.siString, '')
	prop.AddParameter3('PresetFile', constants.siString, '')
	prop.AddParameter3('PresetList', constants.siInt4, 0, 0, 100, False, False)
	prop.AddParameter3('PresetMethod', constants.siInt4, 0, 0, 3, False, False)
	prop.AddParameter3('HairGuide', constants.siString, '')
	prop.AddParameter3('HairRender', constants.siString, '')
	prop.AddParameter3('HairMaterial', constants.siString)
	prop.AddParameter3('HairLogo', constants.siString)
	
	prop.AddParameter3('DynamicsType', constants.siInt4, 1, 0, 2, False, False)
	prop.AddParameter3('ApplyDynamicsMap', constants.siBool, True, False, True, False, False)
	prop.AddParameter3('CreateMeshTarget', constants.siBool, False, False, True, False, False)
	prop.AddParameter3('TargetMesh', constants.siString)
	prop.AddParameter3('HeadBone', constants.siString)
	prop.AddParameter3('ColliderMesh', constants.siString)
	prop.AddParameter3('CollideChooser', constants.siInt4, 0, 0, 10, False, False)
	
	prop.AddParameter2(
		'SkullMapChooser', constants.siInt4, 0, 0, 100, 0, 100,
		constants.siClassifUnknown, constants.siPersistable
	)
	prop.AddParameter2(
		'CageMapChooser', constants.siInt4, 0, 0, 100, 0, 100,
		constants.siClassifUnknown, constants.siPersistable
	)
	
	
def Hair_Groom_OnInit():
	prop = PPG.Inspected(0)
	Hair_Groom_RebuildLayout(prop)
	PPG.Refresh()
	
	
def Hair_Groom_RebuildLayout(prop):
	layout = prop.PPGLayout
	layout.Clear()
	
	layout.AddTab("Groom")
	Property.GetLogo(prop)
	
	layout.AddGroup("Skull Mesh")
	layout.AddRow()
	item = layout.AddItem("SkullMesh")
	item.SetAttribute(constants.siUINoLabel, True)
	layout.AddButton("SelectSkullMesh", "Select")
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("UpVector Curve")
	layout.AddRow()
	item = layout.AddItem("UpVectorCurve")
	item.SetAttribute(constants.siUINoLabel, True)
	layout.AddButton("SelectUpVectorCurve", "Select")
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("Tip Polygons Cluster")
	layout.AddRow()
	item = layout.AddButton("CreateTipPolygonsCluster", "Create From Sel")
	item.SetAttribute(constants.siUICX, 109)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("AddPolygonsToTipPolygonsCluster", "Add Sel")
	item.SetAttribute(constants.siUICX, 59)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("RemovePolygonsFromTipPolygonsCluster", "Remove Sel")
	item.SetAttribute(constants.siUICX, 79)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("ShowTipPolygonsCluster", "Show")
	item.SetAttribute(constants.siUICX, 49)
	item.SetAttribute(constants.siUICY, 25)
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("Tools")
	item = layout.AddButton("BuildHair", "Build/Rebuild")
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("DeleteHair", "Clean/Save Preset")
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("InspectHairGuide", "Inspect Guide")
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("InspectHairRender", "Inspect Render")
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("HideHairObjects", "Hide/Unhide Objects")
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("BuildDebugEmitTriangles", "Build Debug Triangles")
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 25)

	layout.EndGroup()
	
	if prop.Parameters("SkullMapChooser") and prop.Parameters("CageMapChooser"):
		layout.AddTab("Maps")
		layout.AddGroup("Skull Maps")
		#layout.AddTab("WeightMaps")
		skull = Property.FindChildFromParameterValue(prop,"SkullMesh")
		skull_list = []
		if skull:
			skull_list = Property.BuildListWeightMaps(skull, "WeightMapCls")

		if len(skull_list) == 0:
			layout.AddStaticText("No skull or Skull contains no weightmap")
		else:
			item = layout.AddEnumControl("SkullMapChooser", skull_list, "List", constants.siControlListBox)
			item.SetAttribute(constants.siUINoLabel, True)
			btn = layout.AddButton("PaintSkullMap", "Paint in Object View")
			btn.SetAttribute(constants.siUICX, 300)
		layout.EndGroup()
		
		layout.AddGroup("Cage Maps")
		#layout.AddTab("WeightMaps")
		cage = prop.Parent3DObject
		cagelist = Property.BuildListWeightMaps(cage, "WeightMapCls")

		if len(cagelist) == 0:
			layout.AddStaticText("Cage contains no weightmap right now...")
		else:
			item = layout.AddEnumControl("CageMapChooser", cagelist, "List", constants.siControlListBox)
			item.SetAttribute(constants.siUINoLabel, True)
			btn = layout.AddButton("PaintCageMap", "Paint in Object View")
			btn.SetAttribute(constants.siUICX, 300)
		layout.EndGroup()
	
	layout.AddTab("Dynamics")
	Property.GetLogo(prop)
	layout.AddGroup("Input Objects")
	dynamic_type = prop.Parameters("DynamicsType").Value
	if dynamic_type == 0:
		layout.AddGroup("Head Bone")
		layout.AddRow()
		item = layout.AddItem("HeadBone")
		item.SetAttribute(constants.siUINoLabel,True)
		layout.AddButton("PickHeadBone", "Pick")
		layout.EndRow()
		layout.EndGroup()
	layout.AddGroup("Target Mesh")
	mesh_target_options = ["Duplicate Mesh", 0, "Pick Existing Mesh", 1]
	item = layout.AddEnumControl("CreateMeshTarget", mesh_target_options, "", constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel, True)
	if prop.Parameters("CreateMeshTarget").Value:
		layout.AddRow()
		item = layout.AddItem("TargetMesh")
		#item.SetAttribute(constants.siUINoLabel,True)
		layout.AddButton("PickTargetMesh", "Pick")
		layout.EndRow()
	layout.EndGroup()
	#layout.AddGroup("Collider Mesh")
	#layout.AddRow()
	Property.CheckParameterExist(prop, "CollideChooser", constants.siInt4, True)
	
	collide_list = Hair_Groom_GetCollideList(prop)

	layout.AddGroup("Colliders List")
	item = layout.AddEnumControl("CollideChooser", collide_list, "", constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel, True)
	item.setAttribute(constants.siUICY, 50)
	layout.EndGroup()
	layout.AddRow()
	btn = layout.AddButton("AddCollider", "ADD")
	btn.SetAttribute(constants.siUICY, 25)
	btn.SetAttribute(constants.siUICX, 100)
	btn = layout.AddButton("RemoveCollider", "REMOVE")
	btn.SetAttribute(constants.siUICY, 25)
	btn.SetAttribute(constants.siUICX, 100)
	btn = layout.AddButton("ShowCollider", "SHOW")
	btn.SetAttribute(constants.siUICY, 25)
	btn.SetAttribute(constants.siUICX, 100)
	layout.EndRow()
	
	#item = layout.AddItem("ColliderMesh")
	#item.SetAttribute(constants.siUINoLabel,True)
	#layout.AddButton("PickColliderMesh","Pick")
	#layout.EndRow()
	#layout.EndGroup()
	layout.EndGroup()
	
	layout.AddGroup("Dynamics Map")
	layout.AddItem("ApplyDynamicsMap", "Apply ICETree on Dynamics Map")
	layout.EndGroup()
	
	dynamic_type_items = ["ICE SoftBody", 0, "ICE Syflex", 1, "Regular Syflex(NOT implemented!!)", 2]
	layout.AddGroup("Choose Dynamics Type")
	item = layout.AddEnumControl("DynamicsType", dynamic_type_items, "Type", constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel, True)
	layout.EndGroup()
	
	layout.AddGroup()
	layout.AddRow()
	item = layout.AddButton("CreateHairDynamics", "Create Hair Dynamics")
	item.SetAttribute(constants.siUICY, 25)
	item.SetAttribute(constants.siUICX, 300)
	layout.EndRow()
	
	layout.EndGroup()
	
	layout.AddTab("Presets")
	Property.GetLogo(prop)
	
	layout.AddGroup("Method")
	method_items = ["AutoSaved", 0, "Presets", 1, "Save/Load", 2]
	item = layout.AddEnumControl("PresetMethod", method_items, "Method", constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel, True)
	layout.EndGroup()
	
	method = prop.Parameters("PresetMethod").Value
	
	if method == 0:
		layout.AddGroup("AutoSaved")
		path = Hair_Groom_GetAutoSavePresetPath(prop)
		layout.AddStaticText("Preset File Saved as : \n"+path)
		layout.EndGroup()
		
	elif method == 1:
		layout.AddGroup("Presets")
		all_presets = Hair_Groom_GetPresetList()
		layout.AddRow()
		item = layout.AddEnumControl("PresetList", all_presets, "Presets", constants.siControlListBox)
		item.SetAttribute(constants.siUINoLabel, True)
		button = layout.AddButton("ApplyPreset", "Apply")
		button.SetAttribute(constants.siUICX, 80)
		button.SetAttribute(constants.siUICY, 120)
		layout.EndRow()
		layout.EndGroup()
		
	elif method == 2:
		layout.AddGroup("Preset")
		item = layout.AddEnumControl("PresetFile", [], "Preset", constants.siControlFilePath)
		item.SetAttribute(constants.siUIFileFilter, "XML files (*.xml)|*.xml:*.xml|All Files (*.*)|*.*||")
		item.SetAttribute(constants.siUINoLabel, True)
		item.SetAttribute(constants.siUIFileMustExist, True)

		layout.AddRow()
		btn = layout.AddButton("SaveHairPreset", "Save Preset");
		btn.SetAttribute(constants.siUICX, 149)
		btn.SetAttribute(constants.siUICY, 60)
		btn = layout.AddButton("LoadHairPreset", "Load Preset");
		btn.SetAttribute(constants.siUICX, 149)
		btn.SetAttribute(constants.siUICY, 60)
		layout.EndRow()
		
		layout.EndGroup()


def Hair_Groom_PaintMap(weight_map):
	parent_3dobject = weight_map.Parent3DObject
	parent_3dobject.Properties("Visibility").Parameters("ViewVis").Value = True
	XSI.SelectObj(weight_map)
	view = XSI.OpenView("Object View", True)
	view.SetAttributeValue("lockstatus", True)
	XSI.PaintTool()


def Hair_Groom_GetPresetList():
	path = Property.GetPluginPath('HairToolsPlugin')
	preset_items = []
	if path:
		preset_path = path.replace("Application\\Plugins\\Hair\\","Data\\Presets\\Hair")
		XSIUtils.EnsureFolderExists(preset_path)
		XSI.LogMessage(preset_path)
		regex = '.*\.xml$'
		files = XSI.FindFilesInFolder(preset_path, regex, True, False)
		
		i = 0
		for f in files:
			preset_items.append(f)
			preset_items.append(i)
			i += 1
		return preset_items
	return None


def Hair_Groom_DynamicsType_OnChanged():
	prop = PPG.Inspected(0)
	Hair_Groom_RebuildLayout(prop)
	PPG.Refresh()


def Hair_Groom_SkullMapChooser_OnChanged():
	prop = PPG.Inspected(0)

	uiitems = prop.PPGLayout.Item("SkullMapChooser").UIItems
	value = prop.Parameters("SkullMapChooser").Value
	skull = Property.FindChildFromParameterValue(prop,"SkullMesh")
	if not skull:
		return
		
	cls = skull.ActivePrimitive.Geometry.Clusters("WeightMapCls")

	if cls:
		wm = cls.LocalProperties(uiitems[value*2])
		if wm:
			XSI.SelectObj(wm)
			
		else:
			XSI.LogMessage(
				'[Hair_Groom] Can\'t find WeightMap {} !'.format(uiitems[value*2]),
				constants.siWarning
			)


def Hair_Groom_CageMapChooser_OnChanged():
	prop = PPG.Inspected(0)

	items = prop.PPGLayout.Item('CageMapChooser').UIItems
	value = prop.Parameters('CageMapChooser').Value
	cluster = prop.Parent3DObject.ActivePrimitive.Geometry.Clusters('WeightMapCls')

	if cluster:
		wm = cluster.LocalProperties(items[value*2])
		if wm:
			XSI.SelectObj(wm)
			
		else:
			XSI.LogMessage(
				'[Hair_Groom] Can\'t find WeightMap {} !'.format(items[value*2]),
				constants.siWarning
			)


def Hair_Groom_BuildDebugEmitTriangles_OnClicked():
	prop = PPG.Inspected(0)
	guide = Property.FindChildFromParameterValue(prop, 'HairGuide')
	if guide:
		debug = guide.Parent3DObject.AddPrimitive("EmptyPolygonMesh", "DebugEmitTriangles")
		tree = ICETree.CreateIceTree(debug, "DebugTriangles",0)
		compound = XSI.AddICECompoundNode("HairDebugEmitTriangles", str(tree))
		XSI.ConnectICENodes(str(tree) + ".Port1", str(compound) + ".Execute")
		mat = debug.AddMaterial("Lambert", False, "DebugTrianglesMaterial")
		XSI.SetInstanceDataValue("", str(mat) + ".CAV", "Color")
		XSI.DisconnectICENodePort(str(compound) + ".Set_Data.Value")
		XSI.ConnectICENodes(str(compound) + ".Set_Data.Value", str(compound) + ".RandomValueNode.value")
		
	else:
		XSI.LogMessage("[Hair_Groom] Build Debug Emit Triangles, no Guide Cloud found, Sorry...", constants.siWarning)
		
	
def Hair_Groom_PaintCageMap_OnClicked():
	prop = PPG.Inspected(0)

	items = prop.PPGLayout.Item('CageMapChooser').UIItems
	value = prop.Parameters('CageMapChooser').Value
	cluster = prop.Parent3DObject.ActivePrimitive.Geometry.Clusters('WeightMapCls')

	if cluster:
		weight_map = cluster.LocalProperties(items[value*2])
		if weight_map:
			Hair_Groom_PaintMap(weight_map)


def Hair_Groom_PaintSkullMap_OnClicked():
	prop = PPG.Inspected(0)

	items = prop.PPGLayout.Item('CageMapChooser').UIItems
	value = prop.Parameters('CageMapChooser').Value
	skull = Property.FindChildFromParameterValue(prop, 'SkullMesh')
	if not skull:
		return
	cluster = skull.ActivePrimitive.Geometry.Clusters('WeightMapCls')

	if cluster:
		weight_map = cluster.LocalProperties(items[value*2])
		if weight_map:
			Hair_Groom_PaintMap(weight_map)


def Hair_Groom_PresetMethod_OnChanged():
	Hair_Groom_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def Hair_Groom_SelectUpVectorCurve_OnClicked():
	pick = Utils.PickElement(constants.siCurveFilter, 'Pick UpVector Curve')
	model = Property.FindPPGModel(PPG.Inspected(0))
	if pick and pick.Model.Name == model.Name:
		PPG.UpVectorCurve.Value = pick.Name
	else:
		XSI.LogMessage('Ensure the UpVector Curve is under the same model as the Hair Cage', constants.siWarning)


def Hair_Groom_SelectSkullMesh_OnClicked():
	pick = Utils.PickElement(constants.siPolyMeshFilter, 'Pick Skull Mesh')
	model = Property.FindPPGModel(PPG.Inspected(0))
	if pick and pick.Model.Name == model.Name:
		PPG.SkullMesh.Value = pick.Name
	else:
		XSI.LogMessage('Ensure the Skull Object is under the same model as the Hair Cage', constants.siWarning)


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
		cls = XSI.Dictionary.GetObject(cls)
		XSI.SelectObj(cls)

	else:
		XSIUIToolKit.MsgBox(
			'TipPolygons cluster doesn\'t exists on "{}"'.format(obj.FullName),
			constants.siMsgOkOnly,
			'HairTools'
		)


def Hair_Groom_BuildHair_OnClicked():
	valid = Hair_EmitFromMesh(PPG.Inspected(0))


def Hair_Groom_DeleteHair_OnClicked():
	Hair_Groom_WriteXML()
	Hair_CleanUp(PPG.Inspected(0))


def Hair_Groom_InspectHairGuide_OnClicked():
	guide = Property.FindChildFromParameterValue(PPG.Inspected(0),"HairGuide")
	tree = guide.ActivePrimitive.ICETrees("Emit")
	node = tree.Nodes.Find("HairEmitGuideFromMesh2")
	XSI.InspectObj(node, "", "", constants.siLock)


def Hair_Groom_InspectHairRender_OnClicked():
	render = Property.FindChildFromParameterValue(PPG.Inspected(0),"HairRender")
	tree = render.ActivePrimitive.ICETrees("Emit")
	node = tree.Nodes.Find("HairRenderEmit")
	XSI.InspectObj(node, "", "", constants.siLock)


def Hair_Groom_HideHairObjects_OnClicked():
	prop = PPG.Inspected(0)
	obj = prop.Parent3DObject
	model = obj.Model
	grp = model.Groups("HairHidden")
	if grp:
		XSI.ToggleVisibility(grp)
	else:
		XSI.LogMessage("[Hair_Groom] There is no HairHidden Group under " + model.FullName, constants.siError)


def Hair_Groom_WriteXML(filename=None):
	prop = PPG.Inspected(0)
	if not filename:
		filename = Hair_Groom_GetXMLFile(prop)

	nodes = Hair_GetCompoundNodes(prop)
	
	if nodes:	
		XML.WriteCompoundsXMLPreset(filename,nodes)


def Hair_Groom_ReadXML(filename=None):
	prop = PPG.Inspected(0)
	
	if not filename:
		filename = Hair_Groom_GetAutoSavePresetPath(prop)

	nodes = Hair_GetCompoundNodes(prop)
	
	if nodes:	
		XML.ReadCompoundsXMLPreset(filename, nodes)


def Hair_Groom_GetXMLFile(prop):
	if prop.Parameters('PresetFile').Value == '':
		filename = Hair_Groom_GetAutoSavePresetPath(prop)
		return filename
	else:
		filename = prop.Parameters('PresetFile').Value
		if not XML.CheckFileExists(filename):
			prop.Parameters('PresetFile').Value = ""
			Hair_Groom_GetXMLFile(prop)
		else:
			return filename


def Hair_Groom_GetAutoSavePresetPath(PPG):
	project_path = XSI.ActiveProject2.Path
	obj = PPG.Parent3DObject
	preset_folder = XSIUtils.BuildPath(project_path, "Presets", "Hair")
	XSIUtils.EnsureFolderExists(preset_folder, False)
	preset_path = XSIUtils.BuildPath(preset_folder, obj.Name+".xml")
	return preset_path


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
		
	guide = Property.FindChildFromParameterValue(prop,"HairGuide")
	if not guide:
		XSI.LogMessage("[Hair_GetCompoundNodes] Can't find corresponding Hair Guide Cloud!!")
	else:
		tree = guide.ActivePrimitive.ICETrees("Emit")
		if tree:
			node = tree.CompoundNodes.Find("HairEmitGuideFromMesh2")
			if node:
				nodes.append(node)
				
	render = Property.FindChildFromParameterValue(prop,"HairRender")
	if not render:
		XSI.LogMessage("[Hair_GetCompoundNodes] Can't find corresponding Hair Render Cloud")
	
	else:
		tree = render.ActivePrimitive.ICETrees("Emit")
		if tree:
			node = tree.CompoundNodes.Find("HairRenderEmit")
			nodes.append(node)
			
	if len(nodes) > 0:
		return nodes
	else:
		return None


def Hair_Groom_CreateMeshTarget_OnChanged():
	Hair_Groom_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def Hair_Groom_PickTargetMesh_OnClicked():
	pick = Utils.PickElement(constants.siPolyMeshFilter, "Pick Target Mesh")
	if pick:
		if not pick.Model.Name == PPG.Inspected(0).Parent3DObject.Model.Name:
			XSI.LogMessage("[Hair_Groom] Target Mesh have to be under same model as Hair Cage object!!", constants.siWarning)
			return 
		PPG.TargetMesh.Value = pick.Name


def Hair_Groom_PickHeadBone_OnClicked():
	pick = Utils.PickElement(constants.siObjectFilter, "Pick Head Bone")
	if pick:
		if not pick.Model.Name == PPG.Inspected(0).Parent3DObject.Model.Name:
			XSI.LogMessage("[Hair_Groom] Head Bone have to be under same model as Hair Cage object!!", constants.siWarning)
			return 
			
		PPG.HeadBone.Value = pick.Name
		PPG.Refresh()


def Hair_Groom_PickColliderMesh_OnClicked():
	pick = Utils.PickElement(constants.siPolyMeshFilter, "Pick Collider Mesh")
	if pick:
		if not pick.Model.Name == PPG.Inspected(0).Parent3DObject.Model.Name:
			XSI.LogMessage("[Hair_Groom] Collider have to be under same model as Hair Cage object!!", constants.siWarning)
			return 
		
		
		PPG.ColliderMesh.Value = pick.Name


def Hair_Groom_GetCollideList(prop):
	model = prop.Parent3DObject.Model
	if not model.Groups("HairCollide"):
		Utils.GroupSetup(model, None, "HairCollide")
		return []

	else:
		grp = model.Groups("HairCollide")
		members = grp.Members
		if members.Count == 0:
			XSI.LogMessage("[Hair_Control_GetCollideList] HairCollide Group is Empty!!", constants.siWarning)
			return []
		else:
			return [members[i/2] if not (i % 2) else i/2 for i in xrange(2 * len(members))]


def Hair_Groom_AddCollider_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject.Model
	pick = Utils.PickElement(constants.siPolyMeshFilter, "Pick Polymesh Collider")

	if pick:
		Utils.GroupSetup(model, [pick], "HairCollide")
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
	
	# get dynamics type
	dynamic_type = prop.Parameters("DynamicsType").Value
	if type == 2:
		XSI.LogMessage("[Hair_DynamicsProperty] Regular Syflex NOT yet implemented, you have to do it manually...", constants.siError)
		return
	
	# duplicate hair mesh to be used as dynamics target
	dup = prop.Parameters("CreateMeshTarget").Value
	target = None
	if not dup:
		data = mesh.ActivePrimitive.Geometry.Get2()
		target = XSI.ActiveSceneRoot.AddPolygonMesh (data[0], data[1])
		target.Name = mesh.Name+"_Target"
		parent.AddChild(target)
	else:
		target = None
		try:
			target = model.FindChild(prop.Parameters("TargetMesh").Value)
		except ValueError:
			XSI.LogMessage("[Hair_DynamicsProperty] Target Mesh doesn't exist : we create it...", constants.siInfo)
			data = mesh.ActivePrimitive.Geometry.Get2()
			target = XSI.ActiveSceneRoot.AddPolygonMesh (data[0], data[1])
			target.Name = mesh.Name+"_Target"
			parent.AddChild(target)
	
	
	# get head bone
	hn = prop.Parameters("HeadBone").Value
	head_bone = None
	if hn:
		try:
			head_bone = model.FindChild(hn)
		except ValueError:
			pass
		
	if not head_bone and dynamic_type == 0:
		head_bone = model.AddNull("Hair_Driver")
	
	# get collider object
	collider_name = prop.Parameters("ColliderMesh").Value
	
	collider = None
	if collider_name:
		collider = model.FindChild(collider_name)
	
	apply_dynamic_map = prop.Parameters("ApplyDynamicsMap").Value
		
	XSI.Hair_ApplyDynamics(mesh, target, collider, head_bone, dynamic_type, apply_dynamic_map)


def Hair_ApplyDynamics_Init(in_ctxt):
	cmd = in_ctxt.Source
	cmd.Description = ""
	cmd.ReturnValue = True

	args = cmd.Arguments
	args.Add('mesh', constants.siArgumentInput)
	args.Add('target', constants.siArgumentInput)
	args.Add('collider', constants.siArgumentInput)
	args.Add('headbone', constants.siArgumentInput)
	args.Add('dynamictype', constants.siArgumentInput)
	args.Add('setmap', constants.siArgumentInput)
	
	return True


def Hair_ApplyDynamics_Execute(mesh, target, collider, headBone, dynamicType, setMap):
	model = mesh.Model
	parent = mesh.Parent3DObject

	# if no target mesh duplicate hair mesh to be used as dynamics target
	if not target:
		data = mesh.ActivePrimitive.Geometry.Get2()
		target = XSI.ActiveSceneRoot.AddPolygonMesh (data[0], data[1])
		target.Name = mesh.Name+"_Target"
		parent.AddChild(target)
			
	# get head bone if softbody dynamic type
	if not headBone and dynamicType == 0:
		headBone = model.AddNull("Hair_Driver")
		
	# set dynamics weight map
	XSI.FreezeModeling(mesh)
	
	Utils.GetWeightMap(mesh, 'Dynamics_Map', 1, 0, 1)
	Utils.GetWeightMap(mesh, 'Blend_Map', 1, 0, 1)
	
	# Set Dynamic Map
	if setMap:
		tree = ICETree.CreateIceTree(mesh, "SetDynamicsMap", 0)
		compound = XSI.AddICECompoundNode("HairTipGradientMap", str(tree))
		XSI.ConnectICENodes(str(tree) + ".port1", str(compound) + ".Execute")
		if dynamicType < 2:
			XSI.LogMessage("Set Dynamic Map Value")
			XSI.SetValue(str(compound) + ".Out_Start", 0, "")
			XSI.SetValue(str(compound) + ".Out_End", 1, "")
		
		compound = XSI.AddICECompoundNode("HairSmoothDynamicMap", str(tree))
		XSI.AddPortToICENode(str(tree) + ".port1", "siNodePortDataInsertionLocationAfter")
		XSI.ConnectICENodes(str(tree) + ".port2", str(compound) + ".Execute")
		
	# init dynamics ICETree
	tree = ICETree.CreateIceTree(mesh, "InitDynamics",2)
	compound = XSI.AddICECompoundNode("ClothInitShape", str(tree))
	XSI.ConnectICENodes(str(tree) + ".port1", str(compound) + ".Execute")
	XSI.SetValue(str(compound) + ".Reference", ICETree.ReplaceModelNameByThisModel(target, model), "")
	
	# create pin/mimic cluster if not exists
	pin_cluster = mesh.ActivePrimitive.Geometry.Clusters("Pin_Cls")
	mimic_cluster = mesh.ActivePrimitive.Geometry.Clusters("Mimic_Cls")
	if not pin_cluster or not mimic_cluster:
		sub = []
		add = []
		for p in mesh.ActivePrimitive.Geometry.Points:
			if p.IsBoundary:
				sub.append(p.Index)
			else:
				add.append(p.Index)
		
		if not pin_cluster:
			mesh.ActivePrimitive.Geometry.AddCluster(constants.siVertexCluster, "Pin_Cls", sub)
		if not mimic_cluster:
			mesh.ActivePrimitive.Geometry.AddCluster(constants.siVertexCluster, "Mimic_Cls", add)
		
	else:
		XSI.LogMessage("Pin_Cls/Mimic_Cls already in place on " + mesh.FullName + " : creation skipped...")

	# ICE softbody dynamics
	if dynamicType == 0:
			
		# init softbody ICETree
		tree = ICETree.CreateIceTree(mesh, 'SoftbodyInit', 1)
		compound = XSI.AddICECompoundNode("Init Verlet Geometry Data", str(tree))
		XSI.ConnectICENodes(str(tree) + ".port1", str(compound) + ".Execute")
		
		tree = ICETree.CreateSimulatedIceTree(mesh,"SoftbodyDynamics")
		
		compound = XSI.AddICECompoundNode("HairSoftBodyDynamics", str(tree))
		XSI.ConnectICENodes(str(tree) + ".port1", str(compound) + ".Execute")
		
		get1 = XSI.AddICENode("GetDataNode", str(tree))
		get2 = XSI.AddICENode("GetDataNode", str(tree))
		get3 = XSI.AddICENode("GetDataNode", str(tree))
		
		XSI.ConnectICENodes(str(compound) + ".Target_Name", str(get1) + ".OutName")
		XSI.ConnectICENodes(str(compound) + ".Head_Name", str(get2) + ".OutName")
		XSI.ConnectICENodes(str(compound) + ".Collider_Name", str(get3) + ".OutName")

		XSI.SetValue(str(get1) + ".reference", ICETree.ReplaceModelNameByThisModel(target, model), "")
		XSI.SetValue(str(get2) + ".reference", ICETree.ReplaceModelNameByThisModel(headBone, model), "")
		
		if collider:
			Utils.GroupSetup(model, [collider], "HairCollide")
			
		XSI.SetValue(str(get3) + ".reference", "this_model.HairCollide", "")
		
	# ICE syflex dynamics
	if dynamicType == 1:
		
		tree = ICETree.CreateSimulatedIceTree(mesh,"SyflexDynamics")
		compound = XSI.AddICECompoundNode("HairSyflexDynamics", str(tree))
		XSI.ConnectICENodes(str(tree) + ".port1", str(compound) + ".Execute")
		
		get1 = XSI.AddICENode("GetDataNode", str(tree))
		get2 = XSI.AddICENode("GetDataNode", str(tree))
		get3 = XSI.AddICENode("GetDataNode", str(tree))
		get4 = XSI.AddICENode("GetDataNode", str(tree))

		XSI.ConnectICENodes(str(compound) + ".Target_Name", str(get1) + ".OutName")
		XSI.ConnectICENodes(str(compound) + ".Collider_Name", str(get2) + ".OutName")
		XSI.ConnectICENodes(str(compound) + ".Wind_Object", str(get3) + ".OutName")
		XSI.ConnectICENodes(str(compound) + ".Turbulence_Object", str(get4) + ".OutName")

		XSI.SetValue(str(get1) + ".reference", ICETree.ReplaceModelNameByThisModel(target, model), '')
		
		if collider:
			Utils.GroupSetup(model, [collider], "HairCollide")
			
		XSI.SetValue(str(get2) + ".reference", "this_model.HairCollide", "")
			
		# connect wind
		wind = model.FindChild("HairWindControl")
		if not wind:
			wind = Utils.AddNull(parent, 10, 1.5, "HairWindControl", 0, 1, 0.5)
		XSI.SetValue(str(get3) + ".reference", ICETree.ReplaceModelNameByThisModel(wind, model), "")
		
		# connect turbulence
		turbulence = model.FindChild('HairTurbulenceControl')
		if not turbulence:
			turbulence = wind.AddGeometry('Circle', 'NurbsCurve', 'HairTurbulenceControl')
			turbulence.Parameters('Radius').Value = 0.77
			turbulence.Parameters('SubdivU').Value = 24
			tree = ICETree.CreateIceTree(turbulence, 'DeformIcon', 0)
			compound = XSI.AddICECompoundNode('HairTurbulenceIcon', str(tree))
			XSI.ConnectICENodes('{}.port1'.Format(tree), '{}.Execute'.format(compound))
			Utils.SetWireColor(turbulence, 0.3, 1.0, 0.8)
			XSI.FreezeObj(turbulence)
		XSI.SetValue(str(get4) + ".reference", ICETree.ReplaceModelNameByThisModel(turbulence, model), "")
		
	# Restore Shape ICE Tree
	Hair_ApplyRestoreOp(mesh, target)


def Hair_CreateTipPolygonsCluster():
	sel = XSI.Selection(0)
	if not sel.Type == 'polySubComponent':
		XSIUIToolKit.MsgBox(
			'Select some polygons on hair mesh and try again !',
			constants.siMsgOkOnly,
			'HairTools')
		return
	
	obj = sel.SubComponent.Parent3DObject
	if obj.ActivePrimitive.Geometry.Clusters('TipPolygons'):
		XSIUIToolKit.MsgBox(
			'TipPolygons cluster already exists on "{}"\n'
			'Use Add or Remove polygons from TipPolygons cluster instead !'.format(obj.FullName),
			constants.siMsgOkOnly,
			'HairTools'
		)
		return

	cls = sel.SubComponent.CreateCluster('TipPolygons')
	XSI.SelectObj(cls)
	

def Hair_AddPolygonsToTipPolygonsCluster(PPG):
	
	sel = XSI.Selection(0)
	if not sel.Type == 'polySubComponent':
		XSIUIToolKit.MsgBox(
			'Select some polygons on hair mesh and try again...',
			constants.siMsgOkOnly,
			'HairTools'
		)
		return
	
	sub_component = sel.SubComponent
	obj = sub_component.Parent3DObject
	
	cluster = obj.ActivePrimitive.Geometry.Clusters('TipPolygons')
	if not cluster:
		XSIUIToolKit.MsgBox(
			'TipPolygons cluster already exists on "{}"\n'
			'Use Create TipPolygons Cluster instead !'.format(obj.FullName),
			constants.siMsgOkOnly,
			'HairTools'
		)
		return

	# we first delete existing hair setup
	exist = Hair_CleanUp(PPG)
	
	# we add selected polygons
	cluster = obj.ActivePrimitive.Geometry.Clusters("TipPolygons")
	Utils.UpdateClusterComponent(cluster, sub_component, 1)
	
	if exist:
		# we rebuild hair setup
		Hair_EmitFromMesh(PPG)
	

def Hair_RemovePolygonsFromTipPolygonsCluster(PPG):
	sel = XSI.Selection(0)
	if not sel.Type == 'polySubComponent':
		XSIUIToolKit.MsgBox(
			'Select Some Polygons on Hair Mesh and try again...',
			constants.siMsgOkOnly,
			'HairTools'
		)
		return
	
	sub = sel.SubComponent
	obj = sub.Parent3DObject
	cls = obj.ActivePrimitive.Geometry.Clusters('TipPolygons')

	if not cls:
		XSIUIToolKit.MsgBox(
			'TipPolygons cluster already exists on "{}"\n'
			'Use Create TipPolygons Cluster instead !'.format(obj.FullName),
			constants.siMsgOkOnly,
			'HairTools'
		)
		return

	# we first delete existing hair setup
	exist = Hair_CleanUp(PPG)
	
	# we remove selected polygons
	cls = obj.ActivePrimitive.Geometry.Clusters('TipPolygons')
	Utils.UpdateClusterComponent(cls, sub, 2)
	
	if exist:
		# we rebuild hair setup
		Hair_EmitFromMesh(PPG)


def Hair_MergeMeshes_Execute():
	sel = XSI.Selection
	if not Hair_CheckSelection(sel):
		return False
	
	tipsmerged = Hair_GetTipPolygonsIndices(sel)
	op = XSI.ApplyGenOp("MeshMerge", "", sel, 3, "siPersistentOperation", "siKeepGenOpInputs", "")(0)
	op.Parameters("Tolerance").Value = 0
	mesh = op.Parent3DObject
	mesh.Name = "Hair_Mesh_Merged"
	mesh.ActivePrimitive.Geometry.AddCluster(constants.siPolygonCluster,"TipPolygons",tipsmerged)
	XSI.FreezeObj(mesh)

	return True


def Hair_EmitFromMesh(prop):
	mesh = prop.Parent3DObject
	model = mesh.model
	root = mesh.Parent3DObject
	to_hide = [mesh]
	to_cache = [mesh]

	cls = mesh.ActivePrimitive.Geometry.Clusters('TipPolygons')
	if not cls:
		XSIUIToolKit.MsgBox(
			'No TipPolygon Cluster on selected Geometry \n'
			'Create Hairs From Selected Mesh aborted !',
			constants.siMsgOkOnly,
			'HairTools'
		)
		return False
	else:
		if not Hair_CheckGeometry(mesh):
			return False
		
		skull = Property.FindChildFromParameterValue(prop, 'SkullMesh')
		upvcrv = Property.FindChildFromParameterValue(prop, 'UpVectorCurve')
		
		if not skull:
			XSI.LogMessage(
				'[HairTools] You must select skull object before building hair !',
				constants.siWarning
			)
			prop.SkullMesh.Value = ''
			return None
			
		to_hide.append(skull)
		to_cache.append(skull)
		if upvcrv:
			to_hide.append(upvcrv)
			
		# add weight maps
		spread = Utils.GetWeightMap(mesh, 'Spread_Map', 1, 0, 1)
		frizz = Utils.GetWeightMap(mesh, 'Frizz_Map', 1, 0, 1)
	
		# add density weight map on skull
		Utils.GetWeightMap(skull, 'Density_Map', 1, 0, 1)
		Utils.GetWeightMap(skull, 'Cut_Map', 1, 0, 1)
		Utils.GetWeightMap(skull, 'Frizz_Map', 1, 0, 1)
		Utils.GetWeightMap(skull, 'Clone_Map', 1, 0, 1)
		Utils.GetWeightMap(skull, 'Spread_Map', 1, 0, 1)
		
		XSI.FreezeModeling('{},{}'.format(mesh.FullName, skull.FullName))
		
		# get Skull Static and transfer density
		skull_static = Utils.GetMeshDuplicate(XSI.Dictionary.GetObject(skull))
		tree = ICETree.CreateIceTree(skull_static, 'TransferDensity', 0)
		compound = XSI.AddICECompoundNode("HairTransfertDensityMap", str(tree))
		XSI.ConnectICENodes(str(tree) + ".port1", str(compound) + ".Execute")
		get = XSI.AddICENode("GetDataNode", str(tree))
		XSI.SetValue(str(get) + ".reference", ICETree.ReplaceModelNameByThisModel(skull, model), "")
		XSI.ConnectICENodes(str(compound) + ".Skull_Object", str(get) + ".outname")
		to_hide.append(skull_static)
		
		if not mesh.ActivePrimitive.ICETrees("GetMeshData"):
			tree = ICETree.CreateIceTree(mesh, "GetMeshData", 1)
			compound = XSI.AddICECompoundNode("HairGetMeshData", str(tree))
			XSI.ConnectICENodes(str(tree) + ".port1", str(compound) + ".execute")
		
		if not mesh.ActivePrimitive.ICETrees("SetMeshData"):
			tree = ICETree.CreateIceTree(mesh, "SetMeshData", 3)
			compound = XSI.AddICECompoundNode("HairSetMeshData", str(tree))
			XSI.ConnectICENodes(str(tree) + ".port1", str(compound) + ".execute")
		
		guide = root.AddPrimitive("PointCloud", "Hair_Guide")
		to_hide.append(guide)
		prop.Parameters("HairGuide").Value = guide.Name
		tree = ICETree.CreateIceTree(guide, "Emit", 0)
		
		compound = XSI.AddICECompoundNode("HairEmitGuideFromMesh2", str(tree))
		XSI.ConnectICENodes(str(tree) + ".port" + str(1), str(compound) + ".execute")
		get1 = XSI.AddICENode("GetDataNode", str(tree))
		XSI.SetValue(str(get1) + ".reference", str(mesh), "")
		XSI.ConnectICENodes(str(compound) + ".Cage_Mesh", str(get1) + ".outname")
		
		render = root.AddPrimitive("PointCloud","Hair_Render")
		prop.Parameters("HairRender").Value = render.Name
		tree = ICETree.CreateIceTree(render, "Emit", 0)
		
		compound = XSI.AddICECompoundNode("HairRenderEmit", str(tree))
		XSI.ConnectICENodes(str(tree) + ".port" + str(1), str(compound) + ".execute")
	
		get1 = XSI.AddICENode("GetDataNode", str(tree))
		XSI.SetValue(str(get1) + ".reference", str(ICETree.ReplaceModelNameByThisModel(skull_static, model)), "")
		XSI.ConnectICENodes(str(compound) + ".Emitter_Static", str(get1) + ".outname")
		
		get2 = XSI.AddICENode("GetDataNode", str(tree))
		XSI.SetValue(str(get2) + ".reference", str(ICETree.ReplaceModelNameByThisModel(skull, model)), "")
		XSI.ConnectICENodes(str(compound) + ".Emitter_Deform", str(get2) + ".outname")
		
		get3 = XSI.AddICENode("GetDataNode", str(tree))
		XSI.SetValue(str(get3) + ".reference", str(ICETree.ReplaceModelNameByThisModel(guide, model)), "")
		XSI.ConnectICENodes(str(compound) + ".Guide", str(get3) + ".outname")
		
		get4 = XSI.AddICENode("GetDataNode", str(tree))
		if upvcrv:
			XSI.SetValue(str(get4) + ".reference", ICETree.ReplaceModelNameByThisModel(upvcrv, model), "")
		XSI.ConnectICENodes(str(compound) + ".UpVector_Curve", str(get4) + ".outname")
		
		Hair_Groom_ReadXML()
		Utils.GroupSetup(mesh.Model, to_hide, "HairHidden")
		Utils.GroupSetup(mesh.Model, [render], "HairRender")
		Utils.GroupSetup(mesh.Model, to_cache, "PointCache")
		
		# material 
		material_name = prop.Parameters("HairMaterial").Value
		material = None
		try:
			material = XSI.Dictionary.GetObject(material_name)
			render.SetMaterial(material)
		except ValueError:
			XSI.LogMessage("[Hair_Groom] Create New Hair Material")
			material = render.AddMaterial("Phong", False, "Hair_Mat")
			shader = XSI.CreateShaderFromProgID("Softimage.rh_renderer.1.0", material.FullName, "Hair")
			XSI.SIConnectShaderToCnxPoint(shader.FullName + ".out", material.FullName + ".surface", False)
			XSI.SIConnectShaderToCnxPoint(shader.FullName + ".out", material.FullName + ".shadow", False)
			photon = material.Parameters("Photon")
			phong = photon.NestedObjects(0)
			if phong:
				XSI.DeleteObj(phong)
			
			lib = material.Library
			prop.Parameters("HairMaterial").Value = lib.FullName+"."+material.Name
	return True


def Hair_CleanUp(prop):
	mesh = prop.Parent3DObject
	skull = Property.FindChildFromParameterValue(prop, 'SkullMesh')
	if skull:
		model = mesh.Model
		skull_static = model.FindChild('{}_Static'.format(skull.Name))
		if skull_static:
			XSI.DeleteObj(skull_static)
		
	if not mesh.ActivePrimitive.ICETrees('SetMeshData'):
		XSI.LogMessage('{} doesn\'t have HairMeshData ICETree on it !'.format(mesh.FullName))
		return False
	
	done = Hair_DeleteHairCloud(prop)
	
	if done:
		to_delete = []
		for h in mesh.ActivePrimitive.ConstructionHistory:
			if(h.FullName.find('marker') == -1 and h.Type not in ['envelopop', 'clsctr']) or h.Name == 'Simulation':
				to_delete.append(h)
			
		XSI.DeleteObj(to_delete)
		return True
	else:
		return False


def Hair_DeleteHairCloud(prop):
	Hair_Groom_WriteXML()
	
	guide = Property.FindChildFromParameterValue(prop, 'HairGuide')
	render = Property.FindChildFromParameterValue(prop, 'HairRender')
	
	if not guide or not render:
		XSI.LogMessage(
			'[Hair_DeleteHairCloud] Can\'t find corresponding Hair Guide or Hair Render !',
			constants.siWarning
		)
		return False
		
	else:
		material = render.Materials(0)
		library = material.Library
		
		prop.Parameters('HairMaterial').Value = library.FullName+"."+material.Name

		XSI.DeleteObj('{},{}'.format(guide.FullName, render.FullName))
		return True


def Hair_IsMeshEqual(inMesh, inNodeValue):
	if inMesh.FullName == inNodeValue:
		return True
	
	model = inMesh.Model
	s = inMesh.FullName.replace(model.Name, 'this_model')
	if s == inNodeValue:
		return True
		
	return False


def Hair_IsHairCloud(inObj, compoundName):
	if not inObj.Type == 'pointcloud':
		return False
	
	trees = inObj.ActivePrimitive.ICETrees
	if trees.Count == 0:
		return False
	
	for t in trees:
		if t.Name == 'Emit':
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


def Hair_CheckSelection(selection):
	for selected in selection:
		mesh = XSI.Selection(0)
		if not selected.type == 'polymsh':
			XSI.LogMessage('Select Polygon Meshes you want to merge')
			return False
			
		cls = mesh.ActivePrimitive.Geometry.Clusters('TipPolygons')
		if not cls:
			XSI.LogMessage('No TipPolygon Cluster on selected geometry !')
			return False
	return True


def Hair_CheckGeometry(mesh):
	geom = mesh.ActivePrimitive.Geometry
	polygons = geom.facets
	tip_polygons = geom.Clusters('TipPolygons')
	sub = geom.CreateSubComponent(constants.siPolygonCluster)
	check = False
	for p in polygons:
		if not p.NbPoints == 4:
			# check if belongs to TipPolygons Cluster(if NOT, geo is invalid...)
			if tip_polygons.FindIndex(p.Index) == -1:
				sub.AddElement(p.Index)
				check = True
	
	if check:
		XSIUIToolKit.MsgBox(
			'Selected Geometry Contains Triangles or N-Gons outside of TipPolygons cluster !',
			constants.siMsgOkOnly,
			'HairTools'
		)
		XSI.SelectObj(sub)
		return		

	return True


def Hair_ApplyRestoreOp(obj, target):
	restore_map = Utils.GetWeightMap(obj, 'Restore_Map', 0, 0, 1)
	tree = ICETree.CreateIceTree(obj, 'DynamicsRestore', 3)
	XSI.MoveOperatorAfter(obj.ActivePrimitive, tree, str(obj.ActivePrimitive) + '.simulationmarker')

	compound = XSI.AddICECompoundNode('HairRestoreShape', str(tree))
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Result'.format(compound))
	XSI.SetValue(
		'{}.Target_Mesh_Reference'.format(compound),
		ICETree.ReplaceModelNameByThisModel(target, obj.model), ''
	)
