# ---------------------------------------
# ClothToolsPlugin
# ---------------------------------------
from win32com.client import constants
from Globals import XSI
from Globals import XSIUIToolKit
from Globals import XSIUtils
import Utils
import Property
import Cloth
import ICETree
import XML
import time


def XSILoadPlugin(in_reg):
	in_reg.Author = "benmalartre"
	in_reg.Name = "ClothTools"
	in_reg.Major = 1
	in_reg.Minor = 0
	
	in_reg.RegisterProperty("Cloth_Rigger")
	in_reg.RegisterCommand("Cloth_Rig", "Cloth_Rig")
	in_reg.RegisterCommand("Cloth_Clean", "Cloth_Clean")
	
	in_reg.RegisterMenu(constants.siMenuTbCreateClothID, "ClothTools", False, False)


def ClothTools_Init(context):
	menu = context.Source
	item = menu.AddItem('Cloth', constants.siMenuItemSection)
	item.SetBackgroundColor(178, 191, 194)
	menu.AddCallBackItem('Cloth Builder', 'Cloth_GetBuilder')
	menu.AddCallBackItem('Cloth Control', 'Cloth_GetControl')
	menu.AddCallBackItem('Help Page', 'Cloth_GetHelpPage')
	
	
def Cloth_GetBuilder(context):
	mesh = XSI.Selection(0)
	if not mesh or not mesh.type == 'polymsh':
		XSIUIToolKit.MsgBox(
			'Select PolygonMesh and try again...',
			constants.siMsgOkOnly,
			'ClothTools')
		return
	prop = mesh.Properties('Cloth_Rigger')
	if not prop:
		prop = mesh.AddProperty('Cloth_Rigger')
	
	XSI.InspectObj(prop, '', '', constants.siLock)
	
	
def Cloth_GetControl(context):
	model = XSI.Selection(0)
	if not model or not model.type == '#model':
		XSIUIToolKit.MsgBox(
			'Select Model and try again...',
			constants.siMsgOkOnly,
			'ClothTools'
		)
		return
		
	prop = model.Properties('ClothControl')
	if not prop:
		prop = model.AddProperty('Cloth_Control', False, 'ClothControl')
	
	XSI.InspectObj(prop, '', '', constants.siLock)


def Cloth_GetHelpPage(context):
	XSI.LogMessage('Cloth_Tools Help Page not implemented !')


def Cloth_Rigger_Define(context):
	prop = context.Source
	prop.AddParameter3('SetClothStiffness', constants.siBool, False, False, True, False, False)
	prop.AddParameter3('StiffnessDriver', constants.siString)
	prop.AddParameter3('ColliderMeshes', constants.siString)
	prop.AddParameter3('ColliderMeshesList', constants.siInt4, 0, 0, 100, False, False)
	prop.AddParameter3('SelfCollide', constants.siBool, True, False, True, False, False)
	prop.AddParameter3('CreateTargetMesh', constants.siBool, False, False, False, False, False)
	prop.AddParameter3('TargetMesh', constants.siString)
	prop.AddParameter3('PresetFile', constants.siString, '')
	prop.AddParameter3('PresetList', constants.siInt4, 0, 0, 100, False, False)
	prop.AddParameter3('PresetMethod', constants.siInt4, 0, 0, 3, False, False)
	prop.AddParameter3('ClothLogo', constants.siString)
	prop.AddParameter3('OutputMesh', constants.siString)
	
	return True


def Cloth_Rigger_OnInit():
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
	
	layout.AddTab('Builder')
	Property.GetLogo(prop)
	layout.AddGroup('Output Mesh')
	layout.AddRow()
	layout.AddItem('OutputMesh', 'Mesh')
	layout.AddButton('PickOutputMesh', 'Pick')
	layout.EndRow()
	layout.EndGroup()
	layout.AddGroup('Tools')
	item = layout.AddButton('BuildCloth', 'BUILD/REBUILD')
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 30)
	item = layout.AddButton('CleanCloth', 'CLEAN + STORE PRESET')
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 30)
	item = layout.AddButton('InspectCloth', 'INSPECT')
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 30)
	layout.EndGroup()


def Cloth_Rigger_AddDriverTab(prop):
	layout = prop.PPGLayout
	layout.AddTab('Driver')
	Property.GetLogo(prop)
	layout.AddGroup('Driver Object')
	layout.AddStaticText(
		'The driver object is used for setting cloth stiffness\n'
		'according to the character velocity\n'
		'In general we connect it to the Local SRT controller\n'
		'For an hair object, we connect it to the head bone'
	)
	layout.AddGroup()
	stiffness = ['Don\'t Correct Stiffness', 0, 'Select Driver Object', 1]
	item = layout.AddEnumControl('SetClothStiffness', stiffness, '', constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel, True)
	
	if prop.Parameters('SetClothStiffness').Value:
		layout.AddRow()
		item = layout.AddItem('StiffnessDriver')
		item.SetAttribute(constants.siUINoLabel, True)
		layout.AddButton('PickStiffnessDriver', 'Pick')
		layout.EndRow()
	layout.EndGroup()
	layout.EndGroup()


def Cloth_Rigger_AddWeightMapTab(prop):
	obj = prop.Parent3DObject
	layout = prop.PPGLayout
	all_items = Property.BuildListWeightMaps(obj, 'FurMaps')
	
	layout.AddGroup('WeightMaps')
	if len(all_items) == 0:
		layout.AddStaticText('This object contains no weightmap')
	else:
		item = layout.AddEnumControl(
			'WeightMapChooser',
			all_items,
			'List',
			constants.siControlListBox
		)
		item.SetAttribute(constants.siUINoLabel, True)


def Cloth_Rigger_AddTargetTab(prop):
	layout = prop.PPGLayout
	layout.AddTab('Target')
	Property.GetLogo(prop)
	layout.AddGroup('Target Mesh')
	layout.AddStaticText(
		'The target mesh is used for 3 things:\n'
		'	-Init Shape\n'
		'	-Restore Shape\n'
		'	-Driving Shape'
	)
	layout.AddGroup()
	target_mesh_options = ['Duplicate Mesh', 0, 'Use Existing Mesh', 1]
	item = layout.AddEnumControl(
		'CreateTargetMesh',
		target_mesh_options,
		'',
		constants.siControlCombo
	)
	item.SetAttribute(constants.siUINoLabel, True)
	
	if prop.Parameters('CreateTargetMesh').Value:
		layout.AddRow()
		layout.AddItem('TargetMesh')
		layout.AddButton('PickTargetMesh', 'Pick')
		layout.EndRow()
		
	else:
		btn = layout.AddButton('DuplicateTargetMesh', 'Duplicate')
		btn.SetAttribute(constants.siUICX, 288)
		btn.SetAttribute(constants.siUICY, 33)
	layout.EndGroup()
	layout.EndGroup()
	
	if prop.Parameters('CreateTargetMesh').Value:
		layout.AddGroup('Rigid Mimic Cluster')
		layout.AddRow()
		item = layout.AddButton('CreateRigidMimicCluster', 'Create')
		item.SetAttribute(constants.siUICX, 109)
		item.SetAttribute(constants.siUICY, 30)
		item = layout.AddButton('AddPointsRigidMimicCluster', 'Add')
		item.SetAttribute(constants.siUICX, 59)
		item.SetAttribute(constants.siUICY, 30)
		item = layout.AddButton('RemovePointsFromRigidMimicCluster', 'Remove')
		item.SetAttribute(constants.siUICX, 79)
		item.SetAttribute(constants.siUICY, 30)
		item = layout.AddButton('ShowRigidMimicCluster', 'Show')
		item.SetAttribute(constants.siUICX, 49)
		item.SetAttribute(constants.siUICY, 30)
		layout.EndRow()
		layout.EndGroup()
		
		layout.AddGroup('Soft Mimic Cluster')
		layout.AddRow()
		item = layout.AddButton('CreateSoftMimicCluster', 'Create')
		item.SetAttribute(constants.siUICX, 109)
		item.SetAttribute(constants.siUICY, 30)
		item = layout.AddButton('AddPointsSoftMimicCluster', 'Add')
		item.SetAttribute(constants.siUICX, 59)
		item.SetAttribute(constants.siUICY, 30)
		item = layout.AddButton('RemovePointsFromSoftMimicCluster', 'Remove')
		item.SetAttribute(constants.siUICX, 79)
		item.SetAttribute(constants.siUICY, 30)
		item = layout.AddButton('ShowSoftMimicCluster', 'Show')
		item.SetAttribute(constants.siUICX, 49)
		item.SetAttribute(constants.siUICY, 30)
		layout.EndRow()
		layout.EndGroup()


def Cloth_Rigger_AddColliderTab(prop):
	layout = prop.PPGLayout
	layout.AddTab('Collider')
	Property.GetLogo(prop)
	Property.AddObjectList(prop, 'ColliderMeshes')
	
	colliders = prop.Parameters('ColliderMeshes').Value
	
	if not colliders:
		layout.AddStaticText('No colliders for this syflex object !')
		layout.AddGroup('Self Collide')
		layout.AddItem('SelfCollide', 'Apply')
		layout.EndGroup()
		return

	layout.AddGroup('Cloth Point Cluster')
	layout.AddRow()
	item = layout.AddButton('CreateCollidePointCluster', 'Create')
	item.SetAttribute(constants.siUICX, 109)
	item.SetAttribute(constants.siUICY, 30)
	item = layout.AddButton('AddPointsToCollidePointCluster', 'Add')
	item.SetAttribute(constants.siUICX, 59)
	item.SetAttribute(constants.siUICY, 30)
	item = layout.AddButton('RemovePointsFromCollidePointCluster', 'Remove')
	item.SetAttribute(constants.siUICX, 79)
	item.SetAttribute(constants.siUICY, 30)
	item = layout.AddButton('ShowCollidePointCluster', 'Show')
	item.SetAttribute(constants.siUICX, 49)
	item.SetAttribute(constants.siUICY, 30)
	layout.EndRow()
	layout.EndGroup()
	layout.AddGroup('Collider Polygon Cluster')
	layout.AddRow()
	item = layout.AddButton('CreateColliderPolygonCluster', 'Create')
	item.SetAttribute(constants.siUICX, 109)
	item.SetAttribute(constants.siUICY, 30)
	item = layout.AddButton('AddPolygonsToColliderPolygonCluster', 'Add')
	item.SetAttribute(constants.siUICX, 59)
	item.SetAttribute(constants.siUICY, 30)
	item = layout.AddButton('RemovePolygonsFromColliderPolygonCluster', 'Remove')
	item.SetAttribute(constants.siUICX, 79)
	item.SetAttribute(constants.siUICY, 30)
	item = layout.AddButton('ShowColliderPolygonCluster', 'Show')
	item.SetAttribute(constants.siUICX, 49)
	item.SetAttribute(constants.siUICY, 30)
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup('Self Collide')
	layout.AddItem('SelfCollide', 'Apply Self Collide')
	layout.EndGroup()
	

def Cloth_Rigger_AddPresetTab(prop):
	layout = prop.PPGLayout
	layout.AddTab('Presets')
	Property.GetLogo(prop)
	layout.AddGroup('Method')
	items = ['AutoSaved', 0, 'Presets', 1, 'Save/Load', 2]
	item = layout.AddEnumControl('PresetMethod', items, 'Method', constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel, True)
	layout.EndGroup()
	
	method = prop.Parameters('PresetMethod').Value
	
	if method == 0:
		layout.AddGroup('AutoSaved')
		layout.AddStaticText('Preset file saved as : \n')
		layout.EndGroup()
		
	elif method == 1:
		layout.AddGroup('Presets')
		presets = []
		
		layout.AddRow()
		item = layout.AddEnumControl('PresetList', presets, 'Presets', constants.siControlListBox)
		item.SetAttribute(constants.siUINoLabel, True)
		button = layout.AddButton('ApplyPreset', 'Apply')
		button.SetAttribute(constants.siUICX, 80)
		button.SetAttribute(constants.siUICY, 120)
		layout.EndRow()
		layout.EndGroup()
		
	elif method == 2:
		layout.AddGroup('Preset')
		item = layout.AddEnumControl('PresetFile', [], 'Preset', constants.siControlFilePath)
		item.SetAttribute(constants.siUIFileFilter, 'XML files (*.xml)|*.xml:*.xml|All Files (*.*)|*.*||')
		item.SetAttribute(constants.siUINoLabel, True)
		item.SetAttribute(constants.siUIFileMustExist, True)

		layout.AddRow()
		btn = layout.AddButton('SavePreset', 'Save Preset')
		btn.SetAttribute(constants.siUICX, 149)
		btn.SetAttribute(constants.siUICY, 30)
		btn = layout.AddButton('LoadPreset', 'Load Preset')
		btn.SetAttribute(constants.siUICX, 149)
		btn.SetAttribute(constants.siUICY, 30)
		layout.EndRow()
		
		layout.EndGroup()
	

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
	

def Cloth_Rigger_PickOutputMesh_OnClicked():
	pick = Utils.PickElement(constants.siObjectFilter, 'Pick Output Mesh')
	if pick:
		PPG.OutputMesh.Value = pick.Name


def Cloth_Rigger_DuplicateTargetMesh_OnClicked():
	prop = PPG.Inspected(0)
	Cloth_GetTargetMesh(prop)


def Cloth_Rigger_AddColliderMeshes_OnClicked():
	prop = PPG.Inspected(0)
	Property.AddObjectToList(prop, 'ColliderMeshes', 'Pick Collider Mesh')
	Property.AddColliderObject(prop)

	Cloth_Rigger_RebuildLayout(prop)
	PPG.Refresh()


def Cloth_Rigger_RemoveColliderMeshes_OnClicked():
	prop = PPG.Inspected(0)
	Property.RemoveColliderObject(prop)
	Property.RemoveObjectFromList(PPG.Inspected(0), 'ColliderMeshes')
	
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def Cloth_Rigger_AddStaticMeshes_OnClicked():
	Property.AddObjectToList(PPG.Inspected(0), 'StaticMeshes', 'Pick Static Mesh')
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def Cloth_Rigger_RemoveStaticMeshes_OnClicked():
	Property.RemoveObjectFromList(PPG.Inspected(0), 'StaticMeshes')
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def Cloth_Rigger_AddDynamicMeshes_OnClicked():
	Property.AddObjectToList(PPG.Inspected(0), 'DynamicMeshes', 'Pick Dynamic Mesh')
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def Cloth_Rigger_RemoveDynamicMeshes_OnClicked():
	Property.RemoveObjectFromList(PPG.Inspected(0), 'DynamicMeshes')
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def Cloth_Rigger_PickTargetMesh_OnClicked():
	pick = Utils.PickElement(constants.siPolyMeshFilter, 'Pick Target Mesh')
	if pick:
		PPG.TargetMesh.Value = pick.Name


def Cloth_Rigger_PickStiffnessDriver_OnClicked():
	pick = Utils.PickElement(constants.siObjectFilter, 'Pick Stiffness Driver')
	if pick:
		PPG.StiffnessDriver.Value = pick.Name


def Cloth_Rigger_BuildCloth_OnClicked():
	for prop in PPG.Inspected:
		XSI.Cloth_Rig(prop)


def Cloth_Rigger_CleanCloth_OnClicked():
	for prop in PPG.Inspected:
		XSI.Cloth_Clean(prop)


def Cloth_Rigger_CreateCollidePointCluster_OnClicked():
	items = PPG.PPGLayout.Item('ColliderMeshesList').UIItems
	value = PPG.ColliderMeshesList.Value
	collider = items[value*2]
	
	cluster_name = 'Collide_{}'.format(collider)
	cluster_type = constants.siVertexCluster
	target = PPG.Inspected(0).Parent3DObject
		
	Utils.CreateClusterFromSelection(target, cluster_name, cluster_type)


def Cloth_Rigger_CreateColliderPolygonCluster_OnClicked():
	prop = PPG.Inspected(0)
	obj = prop.Parent3DObject
	cluster_name = 'Collide_{}'.format(obj.Name)
	cluster_type = constants.siPolygonCluster
	target = Property.GetObjectFromSelectedUIItem(prop, 'ColliderMeshes')
		
	Utils.CreateClusterFromSelection(target, cluster_name, cluster_type)


def Cloth_Rigger_CreateRigidMimicCluster_OnClicked():
	cluster_name = 'RigidMimicCls'
	Utils.CreateClusterFromSelection(cluster_name)


def Cloth_Rigger_CreateSoftMimicCluster_OnClicked():
	prop = PPG.Inspected(0)
	target = prop.Parent3DObject
	cluster_name = 'SoftMimicCls'
		
	Utils.CreateClusterFromSelection(target, cluster_name)


def Cloth_Rigger_SelectColliderPolygonCluster_OnClicked():
	item_id = Property.GetIDFromSelectedUIItem(PPG.Inspected(0), 'ColliderMeshes')
	target = Property.GetObjectFromSelectedUIItem(PPG.Inspected(0), 'ColliderMeshes')
	cluster = XSI.OpenTransientExplorer(target, constants.siSEFilterPropertiesAndPrimitives, 3)(0)
	if cluster and cluster.Type == 'poly':
		PPG.Inspected(0).Parameters('ColliderPolygonCluster{}'.format(item_id)).Value = cluster.Name
	else:
		XSI.LogMessage(
			'[Cloth_Rigger] Invalid Selection : Select a Polygon Cluster on Collider Geometry',
			constants.siWarning
		)


def Cloth_Rigger_SelectCollidePointCluster_OnClicked():
	item_id = Property.GetIDFromSelectedUIItem(PPG.Inspected(0), "ColliderMeshes")
	target = PPG.Inspected(0).Parent3DObject
	cluster = XSI.OpenTransientExplorer(target, constants.siSEFilterPropertiesAndPrimitives, 3)(0)
	if cluster and cluster.Type == 'pnt':
		PPG.Inspected(0).Parameters('CollidePointCluster{}'.format(item_id)).Value = cluster.Name
	else:
		XSI.LogMessage(
			'[Cloth_Rigger] Invalid Selection : Select a Point Cluster on Cloth Geometry',
			constants.siWarning
		)


def Cloth_Rigger_SelectRigidMimicCluster_OnClicked():
	target = PPG.Inspected(0).Parent3DObject
	cluster = XSI.OpenTransientExplorer(target, constants.siSEFilterPropertiesAndPrimitives, 3)(0)
	if cluster and cluster.Type == 'pnt':
		PPG.Inspected(0).Parameters('RigidMimicCluster').Value = cluster.Name
		Utils.GetWeightMap(target, 'RigidMimicMap', 1, 0, 1, cluster)
	else:
		XSI.LogMessage(
			'[Cloth_Rigger] Invalid Selection : Select a Point Cluster on Cloth Geometry',
			constants.siWarning
		)


def Cloth_Rigger_SelectSoftMimicCluster_OnClicked():
	target = PPG.Inspected(0).Parent3DObject
	cls = XSI.OpenTransientExplorer(target, constants.siSEFilterPropertiesAndPrimitives, 3)(0)
	if cls and cls.Type == 'pnt':
		PPG.Inspected(0).Parameters('SoftMimicCluster').Value = cls.Name
		Utils.GetWeightMap(target, 'SoftMimicMap', 1, 0, 1, cls)
	else:
		XSI.LogMessage(
			'[Cloth_Rigger] Invalid Selection : Select a Point Cluster on Cloth Geometry',
			constants.siWarning
		)
	

def Cloth_Rig_Init(ctxt):
	cmd = ctxt.Source
	cmd.Description = 'Build Cloth Rig From Given Cloth_Rigger PPG'

	args = cmd.Arguments
	args.Add('prop', constants.siArgumentInput)


def Cloth_Rig_Execute(prop):	
	obj = prop.Parent3DObject

	#if cloth op, we delete it, as well as ICE trees
	XSI.Cloth_Clean(prop)
		
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
	cmd.Description = 'Clean Cloth Rig From Given Cloth_Rigger PPG'

	args = cmd.Arguments
	args.Add('prop', constants.siArgumentInput)


def Cloth_Clean_Execute(prop):

	#if cloth op, we store a preset
	Cloth_Rigger_WriteXML(prop)
	
	obj = prop.Parent3DObject
	trees = obj.ActivePrimitive.ICETrees
	
	syflex_op = Cloth.GetClothOperator(obj)
	if syflex_op:
		XSI.DeleteObj(syflex_op)
	
	if trees.Count > 0:
		restore = obj.ActivePrimitive.ICETrees.Find('ClothRestore')
		if restore:
			XSI.DeleteObj(restore)
		
		correct = obj.ActivePrimitive.ICETrees.Find('ClothCorrectStiffness')
		if correct:
			XSI.DeleteObj(correct)
			
		init = obj.ActivePrimitive.ICETrees.Find('ClothInit')
		if init:
			XSI.DeleteObj(init)
	
	model = obj.Model
	target = model.FindChild('{}_Target'.format(obj.Name))
	if target:
		XSI.DeleteObj(target)


def Cloth_ApplyInitOp(obj, target):
	tree = ICETree.CreateICETree(obj, 'ClothInit', 1)
	compound = XSI.AddICECompoundNode('ClothInitShape', str(tree))
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.execute'.format(compound))
	XSI.SetValue('{}.Reference'.format(compound), ICETree.ReplaceModelNameByThisModel(target, obj.Model), '')


def Cloth_ApplyStiffnessOp(obj, target):
	tree = ICETree.CreateICETree(obj, 'ClothStiffness', 1)
	compound = XSI.AddICECompoundNode('ClothSetMimicStiffness', str(tree))
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.execute'.format(compound))
	
	if obj.Model.FullName == target.Model.FullName:
		XSI.SetValue('{}.Velocity_Driver_Reference'.format(compound), ICETree.ReplaceModelNameByThisModel(target), '')
	else:
		XSI.SetValue('{}.Velocity_Driver_Reference'.format(compound), target.FullName, '')


def Cloth_ApplyRestoreOp(obj, target):
	tree = ICETree.CreateICETree(obj, 'ClothRestore', 2)
	compound = XSI.AddICECompoundNode('ClothRestoreShape', str(tree))
	XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Result'.format(compound))
	XSI.SetValue('{}.Target_Mesh_Reference'.format(compound), ICETree.ReplaceModelNameByThisModel(target), '')


def Cloth_ApplySyflexOp(obj):
	syflex_op = obj.ActivePrimitive.ConstructionHistory.Find('syCloth')
	if syflex_op:
		XSI.DeleteObj(syflex_op)
		
	cloth_cluster = obj.ActivePrimitive.Geometry.Clusters('ClothCls')
	if not cloth_cluster:
		cloth_cluster = Utils.CreateCompleteButNotAlwaysCluster(obj, constants.siVertexCluster, 'ClothCls')
	
	mass_map = Utils.GetWeightMap(obj, 'MassMap', 1, 0, 1, cloth_cluster)
	stiffness_map = Utils.GetWeightMap(obj, 'StiffMap', 1, 0, 1, cloth_cluster)
	restore_map = Utils.GetWeightMap(obj, 'RestoreMap', 0, 0, 1, cloth_cluster)
	
	XSI.FreezeObj(mass_map.FullName + "," + stiffness_map.FullName)
	syflex_op = XSI.ApplyOp(
		'syCloth',
		';'.join(obj.FullName, mass_map.FullName, stiffness_map.FullName),
		3, 0, None, 2)(0)
	stf = XSI.ApplyOperator('syProperties', syflex_op)
	start_frame = Utils.GetStartFrame()
	syflex_op.Parameters('FirstFrame').Value = start_frame
	XSI.SaveKey(syflex_op.Parameters('Update'), 1, 1)
	
	stf.Parameters('Density').Value = 0.5
	stf.Parameters('StretchStiffness').Value = 1
	stf.Parameters('ShearStiffness').Value = 0.2
	stf.Parameters('BendStiffness').Value = 0.1
	stf.Parameters('StretchDamping').Value = 0.1
	stf.Parameters('ShearDamping').Value = 0.02
	stf.Parameters('BendDamping').Value = 0.01

	return syflex_op


def Cloth_GetTargetMesh(prop):
	mesh = prop.Parent3DObject
	model = mesh.Model
	
	dup = prop.Parameters('CreateTargetMesh').Value
	target = None
	if not dup:
		target = Cloth_DuplicateMesh(mesh)
	else:
		try:
			target = model.FindChild(prop.Parameters('TargetMesh').Value)
			if not target:
				target = Cloth_DuplicateMesh(mesh)
		except RuntimeError:
			target = Cloth_DuplicateMesh(mesh)
		'''
		if not uti.CheckGeometryMatch(mesh,target):
			XSIUIToolkit.MsgBox('Target Mesh Invalid: we generate a new one',constants.siMsgExclamation,'Cloth_Tools')
			target = Cloth_DuplicateMesh(mesh)
		'''
		
	#Transfer Envelope from Output mesh if not exists	
	if not target.ActivePrimitive.ConstructionHistory.Find('envelopop'):
		out = Cloth_GetOutputMesh(prop)
		if out:
			envelope = out.ActivePrimitive.ConstructionHistory.Find('envelopop')
			if envelope:
				env = XSI.Dictionary.GetObject(envelope)
				weights = env.PortAt(4,0,0).Target2
				XSI.ApplyGenOp(
					'Gator', '', ';'.join([target.FullName,out.FullName]),
					3, 'siPersistentOperation', 'siKeepGenOpInputs', ''
				)
				XSI.TransferClusterPropertiesAcrossGenOp(
					'{}.polymsh.TransferAttributes'.format(target.FullName),
					target.FullName, str(weights), 'Envelope_Weights', '')
				XSI.FreezeModeling(target)
	
	# Deform OutputMesh
	Cloth_DeformOutputMesh(prop)
		
	# Create Init ICE Tree
	trees = target.ActivePrimitive.ICETrees
	if not trees.Find('InitTarget'):
		tree = ICETree.CreateICETree(target, 'InitTarget', 1)
		compound = XSI.AddICECompoundNode('ClothInitTarget', str(tree))
		XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(compound))
	
	# Create PreRoll ICE Tree
	if not trees.Find('PreRollTarget'):
		tree = ICETree.CreateICETree(target, 'PreRollTarget', 3)
		compound = XSI.AddICECompoundNode('ClothTargetPreRoll', str(tree))
		XSI.ConnectICENodes('{}.port1'.format(tree), '{}.Execute'.format(compound))
	
	# Update Cloth Rigger
	prop.Parameters('CreateTargetMesh').Value = 1
	prop.Parameters('TargetMesh').Value = target.Name
	Utils.SetVisibility([target], False, False)
	
	Cloth_Rigger_RebuildLayout(prop)
	try:
		PPG.Refresh()
	except RuntimeError:
		pass
	
	return target


def Cloth_GetOutputMesh(prop):
	out_name = prop.OutputMesh.Value
	if out_name == "":
		return None
		
	obj = prop.Parent3DObject
	model = obj.Model
	out = model.FindChild(out_name)
	if out:
		return out
	return None
	
	
def Cloth_DeformOutputMesh(prop):
	out = Cloth_GetOutputMesh(prop)
	cloth = prop.Parent3DObject
	
	if not out:
		XSI.LogMessage(
			'[Cloth_Rigger]There is no Output Mesh set for "{}"'.format(cloth.FullName),
			constants.siWarning
		)
		return
	
	# delete cage if already exists
	cage = out.ActivePrimitive.ConstructionHistory.Find('cagedeformop')
		
	if cage:
		XSI.DeleteObj(cage)
	
	# delete all cagedeform cluster
	all_clusters = out.ActivePrimitive.Geometry.Clusters
	for cluster in all_clusters:
		if not cluster.Name.find('CageWeightCls') == -1:
			XSI.DeleteObj(cluster)
	
	cage = XSI.ApplyDeformByCage(
		'{};{}'.format(out.FullName, cloth.FullName),
		constants.siConstructionModeAnimation)(0)
	cage.Strength = 10
	cage.Falloff = 0.2
	
	# Deactivate EnvelopeOp
	env = out.ActivePrimitive.ConstructionHistory.Find('envelopop')
	if env:
		env.Mute = True


def Cloth_DuplicateMesh(mesh):
	data = mesh.ActivePrimitive.Geometry.Get2()
	target = XSI.ActiveSceneRoot.AddPolygonMesh (data[0], data[1])
	target.Name = '{}_Target'.format(mesh.Name)
	mesh.Parent3DObject.AddChild(target)
	return target


def Cloth_GetColliderMeshes(prop):
	obj = prop.Parent3DObject
	model = obj.Model
	colliders = []
	tokens = prop.Parameters('ColliderMeshes').Value.split("|")

	for token in tokens:
		if token:
			collide_mesh = model.FindChild(token)

			if not collide_mesh:
				XSI.LogMessage(
					'[Cloth_Rigger] Collider named "{}" not found !'.format(token), constants.siWarning)
				continue

			collider = Cloth.Collider(obj, collide_mesh, False)
			collider.GetPntCluster()
			collider.GetPolyCluster()
			colliders.append(collider)
			collider.Rig()
				
	return colliders


def Cloth_GetExtraCollider(prop):
	obj = prop.Parent3DObject
	parent = obj.Parent3DObject
	
	model = obj.Model
	collide_mesh = model.FindChild('ExtraCollider')
	if not collide_mesh:
		collide_mesh = parent.AddPrimitive('EmptyPolygonMesh', 'ExtraCollider')
		tree = ICETree.CreateICETree(collide_mesh, 'ClothExtraCollider', 0)
		compound = XSI.AddICECompoundNode('ClothExtraCollider', str(tree))
		XSI.ConnectICENodes('{}.port1'.format(tree), '{}.execute'.format(compound))
		
	extra = Cloth.Collider(obj, collide_mesh, True)
	extra.GetPntCluster()
	extra.GetPolyCluster()
	extra.Rig()
		
	return extra


def Cloth_GetRigidMimic(prop, target):
	Cloth_GetMimic(prop, target, 'Rigid')


def Cloth_GetSoftMimic(prop, target):
	Cloth_GetMimic(prop, target, 'Soft')


def Cloth_GetMimic(prop, target, prefix):
	obj = prop.Parent3DObject
	syflex_op = obj.ActivePrimitive.ConstructionHistory.Find('syCloth')
	
	mimic_cluster = obj.ActivePrimitive.Geometry.Clusters('{}MimicCls'.format(prefix))
	if not mimic_cluster:
		mimic_cluster = Utils.CreateCompleteButNotAlwaysCluster(
			obj,
			constants.siVertexCluster,
			'{}MimicCls'.format(prefix)
		)
	
	mimic_map = Utils.GetWeightMap(obj, '{}MimicMap'.format(prefix), 1, 0, 1, mimic_cluster)
	mimic_op = XSI.ApplyOperator(
		'syMimic',
		';'.join([
			syflex_op.FullName,
			target.FullName,
			target.Kinematics.FullName,
			mimic_cluster.FullName,
			obj.Kinematics.FullName,
			mimic_map.FullName])
	)
									
	mimic_op.Reset = 1
	mimic_op.Active = False
	
	if prefix == 'Soft':
		mimic_op.Distance = 0.001
		mimic_op.Stiff = 0.1
		mimic_op.Damp = 0.01
		XSI.SaveKey(mimic_op.ModifMap, 0, True)
		
	else:
		mimic_op.Distance = 0
		mimic_op.Damp = 0.01
		mimic_op.Stiff = 1


def Cloth_GetGravity(op):
	gravity = XSI.ApplyOperator('syGravity', op)
	gravity.Parameters('Gy').Value = -0.1
	
	model = op.Parent3DObject.Model
	icon = model.FindChild('Cloth_Gravity')
	if not icon:
		icon = XSI.GravityIcon()
		icon.Name = 'Cloth_Gravity'
		op.Parent3DObject.Parent3DObject.AddChild(icon)
		
	XSI.ApplySyGravityIconOp(gravity, icon)


def Cloth_GetWind(op):
	wind = XSI.ApplyOperator('syWind', op)
	model = op.Parent3DObject.Model
	icon = model.FindChild('Cloth_Wind')
	if not icon:
		icon = XSI.WindIcon()
		icon.Name = 'Cloth_Wind'
		op.Parent3DObject.Parent3DObject.AddChild(icon)
		
	wind.Parameters('Intensity').Value = 0.1
	wind.Parameters('Active').Value = False
	
	XSI.ApplySyWindIconOp(wind, icon)


def Cloth_GetAir(op):
	air = XSI.ApplyOperator('syAir', op)
	air.Parameters('Damp').Value = 0.05


def Cloth_GetSelfCollide(obj):
	syflex_op = obj.ActivePrimitive.ConstructionHistory.Find('syCloth')
	
	cls = obj.ActivePrimitive.Geometry.Clusters('SelfCollideCls')
	if not cls:
		cls = Utils.CreateCompleteButNotAlwaysCluster(obj, constants.siPolygonCluster, 'SelfCollideCls')

	self_collide_op = XSI.ApplyOperator(
		'sySelfCollide',
		';'.join([syflex_op.FullName, obj.FullName, cls.FullName])
	)
	self_collide_op.Reset = 1
	self_collide_op.Envelope = 0.05


def Cloth_GetPresetList():
	plugins = XSI.Plugins
	all_presets = []

	for plugin in plugins:
		if plugin.Name == 'ClothToolsPlugin':
			plugin_path = plugin.OriginPath
			preset_path = plugin_path.replace('Application\\Plugins\\Cloth\\', 'Data\\ClothPresets')
			XSIUtils.EnsureFolderExists(preset_path)
			regex = '.*\.xml$'
			files = XSI.FindFilesInFolder(preset_path, regex, True, False)

			for idx, _file in enumerate(files):
				all_presets.append(_file)
				all_presets.append(idx)
			return all_presets


def Cloth_PresetMethod_OnChanged():
	Cloth_Rigger_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def Cloth_Rigger_WriteXML(prop, filename=None):
	if not filename:
		filename = Cloth_Rigger_GetXMLFile(prop)

	cloth = prop.Parent3DObject
	cloth_op = cloth.ActivePrimitive.ConstructionHistory.Find('syCloth')
	if cloth_op:
		XML.WriteSyflexXMLPreset(filename, cloth_op)


def Cloth_Rigger_ReadXML(prop, filename=None):
	if not filename:
		filename = Cloth_Rigger_GetXMLFile(prop)

	cloth = prop.Parent3DObject
	cloth_op = cloth.ActivePrimitive.ConstructionHistory.Find('syCloth')
	if cloth_op:
		XML.ReadSyflexXMLPreset(filename, cloth_op)


def Cloth_Rigger_GetXMLFile(prop):
	if prop.Parameters('PresetFile').Value == '':
		filename = Cloth_Rigger_GetAutoSavePresetPath(prop)
		return filename
	else:
		filename = prop.Parameters('PresetFile').Value
		if not XML.CheckFileExists(filename):
			prop.Parameters('PresetFile').Value = ''
			Cloth_Rigger_GetXMLFile(prop)
		else:
			return filename


def Cloth_Rigger_GetAutoSavePresetPath(PPG):
	path = XSI.ActiveProject2.Path
	obj = PPG.Parent3DObject
	preset_folder = XSIUtils.BuildPath(path, 'ClothPresets')
	XSIUtils.EnsureFolderExists(preset_folder, False)
	return XSIUtils.BuildPath(preset_folder, '{}.xml'.format(obj.Name))

	
def Cloth_Rigger_SavePreset_OnClicked():
	filename = PPG.PresetFile.Value
	if filename:
		Cloth_Rigger_WriteXML(PPG.Inspected(0), filename)


def Cloth_Rigger_LoadPreset_OnClicked():
	filename = PPG.PresetFile.Value
	if filename:
		Cloth_Rigger_ReadXML(PPG.Inspected(0), filename)
