#---------------------------------------
#
# 	Hair_Control
#
#	October 2012
#	benmalartre@hotmail.com
#
#---------------------------------------
from Globals import XSI
import Cloth as clo
import Utils as uti
import XML as xml
import Correct as correct
import Property as pro
import File as fil


def XSILoadPlugin( in_reg ):
	in_reg.Author = "benmalartre"
	in_reg.Name = "HairControl"
	in_reg.Email = "benmalartre@hotmail.com"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterProperty("Hair_Control")
	in_reg.RegisterCommand("Hair_GetModelControl","Hair_GetModelControl")
		
	return True
	
#---------------------------------------
# Execute Hair_GetModelControl
#---------------------------------------
def Hair_GetModelControl_Execute():
	model = XSI.Selection
	props = []
	for m in model:
		if not m.Type == "#model" or m.Name == "Scene_Root":
			XSI.LogMessage("[Hair_GetModelControl] Hair_Control can only be applied on Model, sorry!!", constants.siError)
			continue
		p = m.Properties("Hair_Control")
		if not p:
			p = m.AddProperty("Hair_Control")
		props.append(p)
		
	XSI.InspectObj(props, "", "", constants.siLock, False)
	
#---------------------------------------
# Define Hair_Control
#---------------------------------------
def Hair_Control_Define( in_ctxt ):
	prop = in_ctxt.Source
	
	# cloth objects
	prop.AddParameter3("HairObjects",constants.siString)
	prop.AddParameter3("HairObjectChooser",constants.siInt4,0,0,100,False,False)
	
	#prop.AddParameter3("HairObjectIconFolder",constants.siString)
	#prop.AddParameter3("HairObjectIcon",constants.siString)
	prop.AddParameter3("HairControlLogo", constants.siString)
	prop.AddParameter3("HairCharacterPicture", constants.siString)
	
	#common parameters
	prop.AddParameter3("HairActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("PreRoll",constants.siInt4,30,-1000,1000,False,False)
	prop.AddParameter3("FirstFrame",constants.siInt4,1,-10000,10000,False,False)
	prop.AddParameter3("Precision",constants.siFloat,0.001,0.0001,1,False,False)
	
	#properties
	prop.AddParameter3("Mass",constants.siFloat,5.0,0.01,100.0,False,False)
	prop.AddParameter3("Stiffness",constants.siFloat,3.0,0.001,100.0,False,False)
	prop.AddParameter3("Shear",constants.siFloat,.1,0.001,5.0,False,False)
	prop.AddParameter3("Bend",constants.siFloat,.01,0.001,5.0,False,False)
	prop.AddParameter3("Damping",constants.siFloat,.01,0.001,5.0,False,False)
	
	#gravity: we should connect it to icon and multiply by factor
	prop.AddParameter3("GravityActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("GravityX",constants.siFloat,0.0,-10,10,False,False)
	prop.AddParameter3("GravityY",constants.siFloat,-0.1,-10,10,False,False)
	prop.AddParameter3("GravityZ",constants.siFloat,0.0,-10,10,False,False)
	
	# Damp Global
	prop.AddParameter3("DampingGlobal",constants.siFloat,.01,.001,10.0,False,False)
	
	#Collide
	prop.AddParameter3("CollideExt",constants.siFloat,0.05,0.005,2.0,False,False)
	prop.AddParameter3("CollideInt",constants.siFloat,-0.05,-2.0,-0.005,False,False)
	prop.AddParameter3("CollideFriction",constants.siFloat,0.2,0.0,1.0,False,False)
	prop.AddParameter3("CollideDamp",constants.siFloat,0.1,0.0,1.0,False,False)
	prop.AddParameter3("CollideSticky",constants.siFloat,0.0,0.0,1.0,False,False)
	prop.AddParameter3("CollideList",constants.siString)
	prop.AddParameter3("CollideChooser",constants.siInt4,0,0,10,False,False)
	
	#wind: we should connect it to icon and multiply by factor
	prop.AddParameter3("WindActive",constants.siBool,False,False,True,False,False)
	prop.AddParameter3("TurbActive",constants.siBool,False,False,True,False,False)
	
	prop.AddParameter3("WindFrequency",constants.siFloat,200,0.0,500,False,False)
	prop.AddParameter3("WindMultiplier",constants.siFloat,0.01,0.00,10.0,False,False)
	
	#self collide
	prop.AddParameter3("SelfCollideActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("SelfCollideDistance",constants.siFloat,0.1,0,5.0,False,False)
	prop.AddParameter3("SelfCollideFriction",constants.siFloat,.01,0,10.0,False,False)
	prop.AddParameter3("SelfCollideDamping",constants.siFloat,.01,0,1.0,False,False)
	
	#mimic
	prop.AddParameter3("MimicActive",constants.siBool,False,False,True,False,False)
	prop.AddParameter3("MimicDistance",constants.siFloat,0.001,0.0,1.0,False,False)
	prop.AddParameter3("MimicStiffness",constants.siFloat,0.001,0.0,1.0,False,False)
	prop.AddParameter3("MimicDamping",constants.siFloat,0.001,0.0,1.0,False,False)
	
	#blend
	prop.AddParameter3("DynamicsBlend",constants.siFloat,1.0,0.0,1.0,False,False)
	
	#caching
	prop.AddParameter3("CachingMode",constants.siInt4,0,0,2,False,False)
	prop.AddParameter3("WriteSyflexCache",constants.siBool,False,False,True,False,False)
	prop.AddParameter3("ReadSyflexCache",constants.siBool,False,False,True,False,False)
	
	# Output Cache
	prop.AddParameter3("ScenePath",constants.siString)
	prop.AddParameter("OutputFolder",constants.siString)
	prop.AddParameter3("OutputFolderSelection",constants.siInt4,0,0,2,False,False)
	prop.AddParameter3("WriteMethod",constants.siInt4,0,0,2,False,False)
	
	#Preset
	prop.AddParameter3("PresetName",constants.siString,"")
	prop.AddParameter3("PresetFolder",constants.siString,"")
	prop.AddParameter3("PresetList",constants.siInt4,0,0,100,False,False)
	
	#Isolate
	prop.AddParameter3("IsolateCharacter",constants.siBool,False,False,True,False,False)
	
	# Restore
	prop.AddParameter3("RestoreShape",constants.siBool,False,False,True,False,False)
	prop.AddParameter3("RestoreBlend",constants.siFloat,0,0,1,False,False)
	

def Hair_Control_OnInit():
	prop = PPG.Inspected(0)
	#path = "X:\\benjamin.malartre\\bWorkgroup\\Data\\Icons\\Cloth"
	#prop.ClothObjectIconFolder.Value = path
	
	#path += "\\All_Cloth_Objects.bmp"
	#prop.ClothObjectIcon.Value = path
	compounds = Hair_GetDynamicNode(prop)
	
	Hair_Control_FirstFrame_OnChanged()
	
	if len(compounds)>0:
		Hair_Control_CompoundToProperty(prop,compounds[0])
	Hair_Control_RebuildLayout(prop)
	
	#Set Environment to Interactive
	try:
		XSI.SetValue("Environments.Environment.simtimectrl.playmode", 2, "")
	except:
		XSI.LogMessage("[Hair_Control] Can't set Environment to Interactive, Sorry...", constants.siWarning)
	PPG.Refresh()
	
def Hair_Control_RebuildLayout(prop):
	layout = prop.PPGLayout
	layout.Clear()
	
	Hair_Control_AddGlobalsTab(prop)
	Hair_Control_AddForceTab(prop)
	Hair_Control_AddMimicTab(prop)
	Hair_Control_AddCollideTab(prop)
	Hair_Control_AddPresetTab(prop)

def Hair_Control_AddGlobalsTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Hair")
	
	Hair_Control_AddHairObjectChooser(prop)
	layout.AddGroup("Globals")
	layout.AddItem("HairActive","Active")
	layout.EndGroup()
	
	sequencepath = Hair_Control_GetSequencePath()
	outputfolder = prop.Parameters("OutputFolderSelection").Value
	
	layout.AddGroup("Scene Options")
	layout.AddRow()
	item = layout.AddItem("FirstFrame","FirstFrame")
	item.SetAttribute(constants.siUINoSlider,True)
	item = layout.AddItem("PreRoll","PreRoll")
	item.SetAttribute(constants.siUINoSlider,True)
	layout.EndRow()
		
	if not sequencepath:
		outputfolder = 2
	else:
		#layout.AddGroup("Method")
		outputfolderselection = ["Sequence Cache",0,"Shot Cache",1,"Alternative Folder",2]
		item = layout.AddEnumControl("OutputFolderSelection",outputfolderselection,"Output Folder",constants.siControlCombo)
		item.SetAttribute(constants.siUINoLabel,True)
	
	#layout.AddEnumControl("ScenePath",[],"Scene Path",constants.siControlCombo);
	if outputfolder == 1:
		layout.AddRow()
		shotList = cam.GetShotList(sequencepath)
		item = layout.AddEnumControl( "ScenePath", shotList, "", constants.siControlListBox )
		item.SetAttribute(constants.siUINoLabel,True)
		grp = layout.AddGroup("Write Shot Caches")
		
		methodlist = ["ALL AFTER",0,"ONLY SELECTED",1]
		item = layout.AddEnumControl("WriteMethod",methodlist,"",constants.siControlRadio)
		item.SetAttribute(constants.siUINoLabel,True)

		grp.SetAttribute(constants.siUICY,300)
		layout.EndGroup()
		layout.EndRow()
	
		
	btn = layout.AddButton("SetupSimuCharacter","SETUP SCENE")
	btn.SetAttribute(constants.siUICX,310)
	btn.SetAttribute(constants.siUICY,30)
	
	layout.EndGroup()
	
	layout.AddGroup("Syflex Cache")

	item = layout.AddItem("WriteSyflexCache","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "WRITE")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)
	item.SetAttribute(constants.siUICX,310)
	
	item = layout.AddItem("ReadSyflexCache","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "READ")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)
	item.SetAttribute(constants.siUICX,310)
	
	layout.EndGroup()
	layout.AddGroup("Mode")
	readcachearray = ["Simulate",0,"Read Cache",1]
	item = layout.AddEnumControl("ReadCache",readcachearray,"Mode",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,true)
	layout.EndGroup()

	layout.AddGroup("Final Cache")
	
	
	#layout.EndGroup()
	#layout.AddSpacer()

	btn = layout.AddButton("CacheWriteCache", " WRITE FINAL CACHE")
	btn.SetAttribute(constants.siUICX,310)
	btn.SetAttribute(constants.siUICY,32)
	
	if outputfolder == 2:
		layout.AddItem( "OutputFolder","Folder", constants.siControlFolder)
	
	layout.EndGroup()
	
def Hair_Control_AddMimicTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Mimic")
	Hair_Control_AddHairObjectChooser(prop)
	
	layout.AddGroup("Mimic")
	layout.AddItem("MimicActive","Active")
	layout.AddItem("MimicDistance","Distance")
	layout.AddItem("MimicStiffness","Stiffness")
	layout.AddItem("MimicDamping","Damping")
	layout.EndGroup()
	
	layout.AddGroup("Blend Dynamics")
	layout.AddItem("DynamicsBlend","Blend")
	btn = layout.AddButton("PaintDynamicsMap","PAINT")
	btn.SetAttribute(constants.siUICX,310)
	btn.SetAttribute(constants.siUICY,30)
	layout.EndGroup()
	
	layout.AddGroup("Restore Shape")
	layout.AddItem("RestoreShape","Active")
	layout.AddItem("RestoreBlend","Blend")
	btn = layout.AddButton("PaintRestoreMap","PAINT")
	btn.SetAttribute(constants.siUICX,310)
	btn.SetAttribute(constants.siUICY,30)
	layout.EndGroup()

	
def Hair_Control_AddForceTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Forces")
	Hair_Control_AddHairObjectChooser(prop)
	layout.AddGroup("Properties")
	layout.AddItem("Mass","Mass")
	layout.AddItem("Stiffness","Stiffness")
	layout.AddItem("Shear")
	layout.AddItem("Bend")
	layout.AddItem("Damping")
	layout.EndGroup()
	
	layout.AddGroup("Gravity")
	layout.AddItem("GravityActive")
	layout.AddRow()
	item = layout.AddItem("GravityX","X")
	item.SetAttribute(constants.siUINoLabel,True)
	item = layout.AddItem("GravityY","Y")
	item.SetAttribute(constants.siUINoLabel,True)
	item = layout.AddItem("GravityZ","Z")
	item.SetAttribute(constants.siUINoLabel,True)
	layout.EndRow()
	layout.EndGroup()
	layout.AddGroup("Wind")

	layout.AddRow()
	layout.AddItem("WindActive","Wind")
	btn = layout.AddButton("SelectWindIcon","SelectWindIcon")
	btn.SetAttribute(constants.siUICX,150)
	layout.EndRow()
	layout.AddRow()
	layout.AddItem("TurbActive","Turbulence")
	btn = layout.AddButton("SelectTurbIcon","SelectTurbIcon")
	btn.SetAttribute(constants.siUICX,150)
	layout.EndRow()
	layout.AddItem("WindFrequency","Frequency")
	layout.AddItem("WindMultiplier","Multiplier")
	layout.EndGroup()
	
	layout.AddGroup()
	layout.AddItem("DampingGlobal","Damping Global")
	layout.EndGroup()

	
def Hair_Control_AddCollideTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Collision")
	Hair_Control_AddHairObjectChooser(prop)
	layout.AddGroup("Collide")
	layout.AddItem("CollideActive","Active")
	layout.AddItem("CollideExt","CollideExt")
	layout.AddItem("CollideInt","CollideInt")
	layout.AddItem("CollideFriction","Friction")
	layout.AddItem("CollideDamp","Damping")
	layout.AddItem("CollideSticky","Sticky")
	#collidelist = ["One",0,"Two",1,"Three",2,"Four",3,"Five",4,"Six",5]
	collidelist = Hair_Control_GetCollideList(prop)

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
	
	#layout.AddItem("CollideChooser")
	layout.EndGroup()
	
	layout.AddGroup("Self Collide")
	layout.AddItem("SelfCollideActive","Active")
	layout.AddItem("SelfCollideDistance","Distance")
	layout.AddItem("SelfCollideFriction","Friction")
	layout.AddItem("SelfCollideDamping","Damping")
	layout.EndGroup()
	

def Hair_Control_AddCacheTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Cache")
	Hair_Control_AddHairObjectChooser(prop)
	layout.AddGroup("Syflex Cache")

	item = layout.AddItem("WriteSyflexCache","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "WRITE")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)
	item.SetAttribute(constants.siUICX,310)
	
	item = layout.AddItem("ReadSyflexCache","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "READ")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)
	item.SetAttribute(constants.siUICX,310)
	
	layout.EndGroup()
	layout.AddGroup("Mode")
	readcachearray = ["Simulate",0,"Read Cache",1]
	item = layout.AddEnumControl("ReadCache",readcachearray,"Mode",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,true)
	layout.EndGroup()

	layout.AddGroup("Final Cache")
	
	#layout.AddGroup("Method")
	outputfolderselection = ["Sequence Cache",0,"Shot Cache",1,"Alternative Folder",2]
	item = layout.AddEnumControl("OutputFolderSelection",outputfolderselection,"Output Folder",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,True)
	outputfolder = prop.Parameters("OutputFolderSelection").Value
	
	#layout.AddEnumControl("ScenePath",[],"Scene Path",constants.siControlCombo);
	if outputfolder == 1:
		layout.AddRow()
		sequencepath = Hair_Control_GetSequencePath()
		shotList = cam.GetShotList(sequencepath)
		item = layout.AddEnumControl( "ScenePath", shotList, "", constants.siControlListBox )
		item.SetAttribute(constants.siUINoLabel,True)
		text = "Warning : all shots BELOW selected one will be filled with the current simulation cache!"
		layout.AddStaticText(text,150,100)
		
		layout.EndRow()
	
	elif outputfolder == 2:
		layout.AddItem( "OutputFolder","Folder", constants.siControlFolder)
	
	#layout.EndGroup()
	#layout.AddSpacer()
	layout.AddRow()
	btn = layout.AddButton("CacheDeleteCache", " Delete Cache ")
	btn.SetAttribute(constants.siUICX,150)
	btn.SetAttribute(constants.siUICY,32)
	btn = layout.AddButton("CacheWriteCache", " Write Cache ")
	btn.SetAttribute(constants.siUICX,150)
	btn.SetAttribute(constants.siUICY,32)
	layout.EndRow()
	
	layout.EndGroup()
	
	
def Hair_Control_AddIsolateButton(prop):
	layout = prop.PPGLayout
	item = layout.AddItem("IsolateCharacter","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "ISOLATE CHARACTER")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)
	item.SetAttribute(constants.siUICX,320)
	item.SetAttribute(constants.siUICY,50)
	
def Hair_Control_AddPresetTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Presets")
	Hair_Control_AddHairObjectChooser(prop)
	
	layout.AddGroup("Presets Folder")
	item = layout.AddEnumControl("PresetFolder",[],"Folder",constants.siControlFolder)
	item.SetAttribute(constants.siUIFileMustExist,True)
	layout.EndGroup()
	
	layout.AddGroup("Save Preset")
	layout.AddRow()
	layout.AddItem("PresetName","Name")
	btn = layout.AddButton("SaveDynamicsPreset","Save");
	btn.SetAttribute(constants.siUICX,80)
	btn.SetAttribute(constants.siUICY,30)
	layout.EndRow()
	layout.EndGroup()

	layout.AddGroup("Load Preset")
	aPresets = Hair_Control_GetPresetList()
	
	if not aPresets or len(aPresets) == 0:
		layout.AddStaticText("No Presets Stored in Preset Folder!")
	else:
		layout.AddRow()
		item = layout.AddEnumControl("PresetList",aPresets,"Presets",constants.siControlListBox)
		item.SetAttribute(constants.siUINoLabel,True)
		item.SetAttribute(constants.siUIWidthPercentage,90)
		layout.AddGroup()
		button = layout.AddButton("LoadDynamicsPreset","Load")
		button.SetAttribute(constants.siUICX,80)
		button.SetAttribute(constants.siUICY,70)
		button = layout.AddButton("DeleteDynamicsPreset","Delete")
		button.SetAttribute(constants.siUICX,80)
		button.SetAttribute(constants.siUICY,30)
		layout.EndGroup()
		layout.EndRow()
	layout.EndGroup()
	
def Hair_Control_AddHairObjectChooser(prop):
	layout = prop.PPGLayout
	pro.GetLogo(prop)
	
	Hair_Control_AddIsolateButton(prop)
	
	grp = layout.AddGroup("Choose Hair Object")
	layout.AddRow()
	items = Hair_Control_GetHairObjectsList(prop)
	if items:
		item = layout.AddEnumControl( "HairObjectChooser", items, "HairObjectChooser", constants.siControlCombo)
		item.SetAttribute(constants.siUINoLabel, True)

	else:
		layout.AddStaticText("There are NO hair objects under this model, sorry!")
		layout.EndRow()
		layout.EndGroup()
		return
		
	layout.EndRow()
	layout.EndGroup()
	
	
	
	if items:
		return layout.Item("HairObjectChooser").UIItems[prop.Parameters("HairObjectChooser").Value * 2]
	

def Hair_Control_GetHairObjectsList(prop):
	hairs = Hair_Control_GetHairObjects(prop)
	
	if len(hairs) == 0:
		return None
	items = []
	if len(hairs) == 1:
		items.append(hairs[0].Name)
		items.append(0)
	else:	
		items.append("All Hair Cages")
		items.append(0)
	
	for h in range(len(hairs)):
		items.append(hairs[h].Name)
		items.append(h+1)
	
	return items
	
def Hair_Control_GetHairObjects(prop):
	model = prop.Parent3DObject
	icetrees =  XSI.FindObjects2(constants.siICETreeID)
	hairs = []
	for i in icetrees:
		if i.CompoundNodes.Find("HairSyflexDynamics") and i.Parent3DObject.Model.FullName == model.FullName:
			hairs.append(i.Parent3DObject)
			
	return hairs
		
def Hair_Control_GetTargetObjects(prop):
	hairs = Hair_Control_GetHairObjects(prop)
	targets = []
	for h in hairs:
		tree = c.ActivePrimitive.ICETrees.Find("ClothInit")
		if tree:
			node = tree.Nodes("HairInitShape")
			targetname = XSI.GetValue(str(node) + ".Reference")
			targetname = targetname.replace("this_model",model.Name)
			target = XSI.Dictionary.GetObject(targetname)
			if target:
				targets.append(target)
				
	return targets

def Hair_Control_GetColliderObjects(prop):
	model = prop.Parent3DObject
	hairs = Hair_Control_GetHairObjects(prop)
	colliders = []
	for h in hairs:
		#Get Hair Cage History
		hist = h.ActivePrimitive.ConstructionHistory
		
		colliderICEname = ""
	
		for h in hist:
			if h.Name == "SyflexDynamics":
				root = h.RootNodeContainer
				gets = root.DataProviderNodes
				
				#Get Collider ICE name
				colliderICEname = gets(1).Parameters("reference").Value
				
		if not colliderICEname:
			continue
		
		# append collider to collider list
		collidername = colliderICEname.replace("this_model.","")
		collider = model.FindChild(collidername)
		if collider:
			if collider.Type == "#Group":
				for c in collider.Members:
					colliders.append(c)
			else:
				colliders.append(collider)

	return colliders	
	
#--------------------------------------------
# Collider Gestion
#--------------------------------------------
def Hair_Control_GetCollideList(prop):
	model = prop.Parent3DObject
	if not model.Groups("HairCollide"):
		colliders = Hair_Control_GetColliderObjects(prop)
		list = []
		id = 0
		for c in colliders:
			list.append(c.FullName)
			list.append(id)
			id = id+1
		return list
	else:
		grp = model.Groups("HairCollide")
		members = grp.Members
		if members.Count == 0:
			XSI.LogMessage("[Hair_Control_GetCollideList] HairCollide Group is Empty!!", constants.siError)
			return []
		else:
			list = []
			id = 0
			for m in members:
				list.append(m.FullName)
				list.append(id)
				id = id+1
			return list

def Hair_Control_AddCollider_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	pick = uti.PickElement(constants.siPolyMeshFilter,"Pick Polymesh Collider")
	grp = model.Groups("HairCollide")
	if not grp:
		XSI.LogMessage("[Hair_Control_AddCollider] No HairCollide Group under " + model.Name + ", Check your Asset!", constants.siError)
		return
		
	if pick:
		grp.AddMember(pick)
		Hair_Control_RebuildLayout(prop)
		PPG.Refresh()

def Hair_Control_RemoveCollider_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	grp = model.Groups("HairCollide")
	if not grp:
		XSI.LogMessage("[Hair_Control_AddCollider] No HairCollide Group under " + model.Name + ", Check your Asset!", constants.siError)
		return
		
	items = PPG.PPGLayout.Item("CollideChooser").UIItems
	value = prop.Parameters("CollideChooser").Value
	
	if len(items)<=2:
		XSI.LogMessage("[Hair_Control_RemoveCollider] There is only ONE collider in HairCollideGroup : Can't remove it!", constants.siWarning)
		return
	else:
		collider = XSI.Dictionary.GetObject(items[value * 2])
		if collider:
			grp.RemoveMember(collider)
			Hair_Control_RebuildLayout(prop)
			PPG.Refresh()
	
def Hair_Control_GetOutObjects(prop):
	model = prop.Parent3DObject
	clothes = Hair_Control_GetHairObjects(prop)
	outmeshes = []
	for c in clothes:
		out = xml.GetCageDeformedObjectFromConnectionStack(c)
		if out:
			outmeshes.append(out)
		else:
			outmeshes.append(c)
	return outmeshes
	
def Hair_Control_IsolateCharacter_OnChanged():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	XSI.SelectObj("B:" + model.FullName)
	XSI.IsolateSelected(1 - prop.Parameters("IsolateCharacter").Value, -1)
	XSI.DeselectAll()
	
def Hair_Control_WriteSyflexCache_OnChanged():
	prop = PPG.Inspected(0)
	value = 0
	if prop.Parameters("WriteSyflexCache").Value:
		prop.Parameters("ReadSyflexCache").Value = False
		value = 1
		
	nodes = Hair_GetDynamicNode(prop)

	for n in nodes:
		XSI.LogMessage(n)
		n.InputPorts("Cache_Mode").Value = value

def Hair_Control_ReadSyflexCache_OnChanged():
	prop = PPG.Inspected(0)
	value = 0
	if prop.Parameters("ReadSyflexCache").Value == True:
		prop.Parameters("WriteSyflexCache").Value = False
		value = 2
		
	nodes = Hair_GetDynamicNode(prop)
	#xsi.LogMessage(value)
	for n in nodes:
		n.InputPorts("Cache_Mode").Value = value
		
def Hair_Control_SetupSimuCharacter_OnClicked():
	prop = PPG.Inspected(0)			
	Hair_Control_ApplyBindPose(prop)
	
def Hair_Control_CacheWriteCache_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	mode = prop.Parameters("OutputFolderSelection").Value
	
	# exit if syflexcache isn't in READ mode
	read = prop.Parameters("ReadSyflexCache").Value
	if not read:
		XSI.LogMessage("[Hair_Control] Syflex Cache isn't in READ mode: Write Cache aborted!!", constants.siError)
		return
		
	# exit if syflexcache doesn't exist(check for first and last frames)
	# to implement
	
	sequencepath = Hair_Control_GetSequencePath()
	if not sequencepath:
		mode = 2
		
	startframe = uti.GetStartFrame()
	endframe = uti.GetEndFrame()
	coord = True
	format = 0
	rate = 1.0
	checkexist = True
	
	# Sequence Cache Mode (Fill All :
	if mode == 0:
		shots = cam.GetShotsFromFile(sequencepath)
		cachepath = XSIUtils.BuildPath(sequencepath,"Cache")
		buttonPressed = XSIUIToolkit.Msgbox( "Write Sequence Cache", constants.siMsgYesNo | constants.siMsgQuestion, "Hair_Dynamics" )
		if buttonPressed == constants.siMsgNo:
			XSI.LogMessage("[Hair_WriteCache] Write Sequence Cache Cancelled by the User...", constants.siWarning)
			return
		else:
			XSI.LogMessage("[Hair_Control_WriteCache] Write SEQUENCE Cache : " + cachepath)
			for s in shots:
				startframe = s.frameIn
				endframe = s.frameOut
				shotpath = XSIUtils.BuildPath(cachepath,s.shot)
			
				XSI.LogMessage("[Hair_Control_WriteCache] Write SHOT Cache : " + shotpath)
				XSI.WritePointCacheModel(model, shotpath, startframe, endframe, coord, format, rate, False)
		
	# Shot Cache Mode
	elif mode == 1:
		shots = cam.GetShotsFromFile(sequencepath)
		
		value = int(PPG.ScenePath.Value)
		items = PPG.PPGLayout.Item("ScenePath").UIItems
		currentshot = items[value*2]
		
		started = -1
		cachepath = ""
		for s in shots:
			if currentshot.find(s.shot)>-1:
				started = 0
			if started == 0:
				startframe = s.frameIn
				endframe = s.frameOut
				#endframe = shots[len(shots)-1].frameOut
				cachepath = XSIUtils.BuildPath(sequencepath,"Cache",s.shot)
				msg = "Write Shot Cache\nFrom "+s.shot+" To "+ shots[len(shots)-1].shot
				if prop.WriteMethod.Value == 1:
					msg = "Write Shot Cache For "+s.shot
					
				buttonPressed = XSIUIToolkit.Msgbox( msg, constants.siMsgYesNo | constants.siMsgQuestion, "Hair_Dynamics" )
				if buttonPressed == constants.siMsgNo:
					XSI.LogMessage("[Hair_WriteCache] Write Shot Cache Cancelled by the User...", constants.siWarning)
					return
				else:
					XSI.LogMessage("[Hair_Control_WriteCache] Write SHOT Cache : " + cachepath)
					XSI.WritePointCacheModel(model, cachepath, startframe, endframe, coord, format, rate, checkexist)
					started = 1
					if prop.WriteMethod.Value == 1:
						return
			elif started == 1:
				startframe = s.frameIn
				endframe = s.frameOut
				cachepath = XSIUtils.BuildPath(sequencepath,"Cache",s.shot)
				XSI.WritePointCacheModel(model, cachepath, startframe, endframe, coord, format, rate, False)
		
	# Alternative Folder Cache Mode
	elif mode == 2:
		folder = PPG.Inspected(0).Parameters("OutputFolder").Value
		if not fil.FolderExists(folder):
			XSI.LogMessage("[Hair_Control] Output Folder is Invalid : Write Cache Aborted!", constants.siWarning)
			return
		else:
			XSI.LogMessage("[Hair_Control_WriteCache] Write ALTERNATIVE Cache : " + folder)
			XSI.WritePointCacheModel(model, folder, startframe, endframe, coord, format, rate, checkexist)
	
def Hair_Control_AddExtraCollider_OnClicked():
	prop = PPG.Inspected(0)
	colliders = Hair_Control_GetColliderObjects(prop)
	for c in colliders:
		XSI.LogMessage(c.FullName)
			
#-------------------------------------------------
# Parameters changed callback
#-------------------------------------------------
def Hair_Control_ClothObjectChooser_OnChanged():
	prop = PPG.Inspected(0)
	
	items = PPG.PPGLayout.Item("ClothObjectChooser").UIItems
	value = PPG.ClothObjectChooser.Value
	path = prop.ClothObjectIconFolder.Value

	if value>0:
		selected = items[value*2]
		
		model = PPG.Inspected(0).Parent3DObject
		cloth = model.FindChild(selected)
		Hair_Control_SyflexToProperty(prop,cloth)
		path += "\\"+cloth.Name+".bmp"
		
	else:
		path += "\\All_Cloth_Objects.bmp"
	
	PPG.ClothObjectIcon.Value = path
	Hair_Control_RebuildLayout(prop)
	PPG.Refresh()
		
def Hair_Control_CompoundToProperty(prop,compound):
		
	#Globals
	prop.Parameters("HairActive").Value = compound.InputPorts("Active").Value
	prop.Parameters("FirstFrame").Value = compound.InputPorts("First_Frame").Value + prop.Parameters("PreRoll").Value
	prop.Parameters("Precision").Value = compound.InputPorts("Precision").Value
	
	#Properties
	prop.Parameters("Mass").Value = compound.InputPorts("Mass").Value
	prop.Parameters("Stiffness").Value = compound.InputPorts("Stiffness").Value
	prop.Parameters("Shear").Value = compound.InputPorts("Shear").Value
	prop.Parameters("Bend").Value = compound.InputPorts("Bend").Value
	prop.Parameters("Damping").Value = compound.InputPorts("Damping").Value
	
	#Gravity
	#prop.Parameters("GravityActive").Value = compound.InputPorts("Active").Value
	gravityactive = compound.InputPorts("Active").Value
	if not gravityactive:
		prop.Parameters("GravityX").Value = 0
		prop.Parameters("GravityY").Value = 0
		prop.Parameters("GravityZ").Value = 0
	else:
		prop.Parameters("GravityX").Value = compound.Parameters("Gravity_x").Value
		prop.Parameters("GravityY").Value = compound.Parameters("Gravity_y").Value
		prop.Parameters("GravityZ").Value = compound.Parameters("Gravity_z").Value

	#Wind
	prop.Parameters("WindActive").Value = compound.InputPorts("Use_Wind").Value
	prop.Parameters("TurbActive").Value = compound.InputPorts("Use_Wind_Turbulence").Value
	prop.Parameters("WindFrequency").Value = compound.InputPorts("Wind_Frequency").Value
	prop.Parameters("WindMultiplier").Value = compound.InputPorts("Wind_Multiplier").Value
	
	#Mimic
	prop.Parameters("MimicActive").Value = compound.InputPorts("Use_Mimic").Value
	prop.Parameters("MimicDistance").Value = compound.InputPorts("Mimic_Distance").Value
	prop.Parameters("MimicStiffness").Value = compound.InputPorts("Mimic_Stiff").Value
	prop.Parameters("MimicDamping").Value = compound.InputPorts("Mimic_Damp").Value
			
	#Collide
	prop.Parameters("CollideExt").Value = compound.InputPorts("Collider_Ext").Value
	prop.Parameters("CollideInt").Value = compound.InputPorts("Collider_Int").Value
	prop.Parameters("CollideFriction").Value = compound.InputPorts("Collider_Friction").Value
	prop.Parameters("CollideDamp").Value = compound.InputPorts("Collider_Damp").Value
	prop.Parameters("CollideSticky").Value = compound.InputPorts("ColliderSticky").Value
	
	#Self Collide
	prop.Parameters("SelfCollideActive").Value = compound.InputPorts("Self_Collide").Value
	prop.Parameters("SelfCollideDistance").Value = compound.InputPorts("Self_Collide_Distance").Value
	prop.Parameters("SelfCollideFriction").Value = compound.InputPorts("Self_Collide_Friction").Value
	prop.Parameters("SelfCollideDamping").Value = compound.InputPorts("Self_Collide_Damp").Value

def Hair_Control_PreRoll_OnChanged():
	Hair_Control_FirstFrame_OnChanged()
	Hair_Control_RebuildLayout(PPG.Inspected(0))
	
def Hair_Control_UpdatePreRoll(prop):
	targets = Hair_Control_GetTargetObjects(prop)
	for t in targets:
		tree = t.ActivePrimitive.ICETrees("PreRollTarget")
		if tree:
			node = tree.Nodes("ClothTargetPreRoll")
			if node:
				start = prop.Parameters("FirstFrame").Value
				end = start + prop.Parameters("PreRoll").Value
				node.InputPorts("Start_Frame").Value = start
				node.InputPorts("End_Frame").Value = end
		
	colliders = Hair_Control_GetColliderObjects(prop)
	for c in colliders:
		tree = c.ActivePrimitive.ICETrees("PreRollCollide")
		if tree:
			node = tree.Nodes("ClothTargetPreRoll")
			if node:
				start = prop.Parameters("FirstFrame").Value
				roll = prop.Parameters("PreRoll").Value
				node.InputPorts("Start_Frame").Value = start
				node.InputPorts("End_Frame").Value = start + roll
				
def Hair_Control_OutputFolderSelection_OnChanged():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	mode = prop.Parameters("OutputFolderSelection").Value
	
	sequencepath = Hair_Control_GetSequencePath()	
	preroll = prop.Parameters("PreRoll").Value
	firstframe = prop.Parameters("FirstFrame").Value
	startframe =  firstframe - preroll

	# alternative mode
	if not sequencepath:
		mode  = 2
	
	# sequence/shot mode
	else :
		shots = cam.GetShotsFromFile(sequencepath)
		firstframe = shots[0].frameIn
		startframe = firstframe - (preroll)
		prop.Parameters("FirstFrame").Value = startframe
					
	Hair_Control_ParameterChanged(prop,"FirstFrame","First_Frame")
	
	#Restore First Frame
	prop.Parameters("FirstFrame").Value = firstframe
	
	#Rebuild Layout
	Hair_Control_RebuildLayout(prop)
	PPG.Refresh()
	
def Hair_Control_ScenePath_OnChanged():
	sequencepath = Hair_Control_GetSequencePath()
	shots = cam.GetShotsFromFile(sequencepath)
	prop = PPG.Inspected(0)
	firstframe = prop.Parameters("FirstFrame").Value
	preroll = prop.Parameters("PreRoll").Value

	try:
		value = int(PPG.ScenePath.Value)
	except:
		value = 0
		
	items = PPG.PPGLayout.Item("ScenePath").UIItems
	currentshot = items[value*2]
	
	started = -1
	cachepath = ""
	for s in shots:
		if currentshot.find(s.shot)>-1:
			firstframe = s.frameIn
			startframe = firstframe - (preroll)
			prop.Parameters("FirstFrame").Value = startframe
			break
	
	Hair_Control_ParameterChanged(prop,"FirstFrame","First_Frame")
	
	#Restore First Frame
	prop.Parameters("FirstFrame").Value = firstframe
	
	#Rebuild Layout
	Hair_Control_RebuildLayout(prop)
	PPG.Refresh()

def Hair_Control_FirstFrame_OnChanged():
	prop = PPG.Inspected(0)
	firstframe = prop.Parameters("FirstFrame").Value
	preroll = prop.Parameters("PreRoll").Value
	startframe = firstframe - preroll
	prop.Parameters("FirstFrame").Value = startframe
	Hair_Control_ParameterChanged(prop,"FirstFrame","First_Frame")
	uti.SetStartFrame(startframe)
	uti.SetCurrentFrame(startframe)
	prop.Parameters("FirstFrame").Value = firstframe
		
def Hair_Control_HairActive_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"HairActive","Active")
	
def Hair_Control_Precision_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"Precision","Precision")
	
def Hair_Control_DynamicsBlend_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"DynamicsBlend","Dynamics_Blend")
	
def Hair_Control_Mass_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"Mass","Mass")

def Hair_Control_Stiffness_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"Stiffness","Stiffness")
	
def Hair_Control_Shear_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"Shear","Shear")
	
def Hair_Control_Bend_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"Bend","Bend")
	
def Hair_Control_Damping_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"Damping","Damping")
	
def Hair_Control_GravityFactor_OnChanged():
	XSI.LogMessage("Udpdate Parameter for ScriptedOp GravityIconOp")

def Hair_Control_GravityActive_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"GravityActive","GravityActive")
	
def Hair_Control_GravityX_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"GravityX","Gravity_X")
	
def Hair_Control_GravityY_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"GravityY","Gravity_Y")
	
def Hair_Control_GravityZ_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"GravityZ","Gravity_Z")
	
def Hair_Control_WindFactor_OnChanged():
	XSI.LogMessage("Udpdate Parameter for ScriptedOp WindIconOp")

def Hair_Control_WindActive_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"WindActive","Use_Wind")

def Hair_Control_TurbActive_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"TurbActive","Use_Wind_Turbulence")
	
def Hair_Control_WindFrequency_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"WindFrequency","Wind_Frequency")
	
def Hair_Control_WindMultiplier_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"WindMultiplier","Wind_Multiplier")
	
def Hair_Control_MimicActive_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"MimicActive","Active")
	
def Hair_Control_SelfCollideActive_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideActive","Self_Collide")
	
def Hair_Control_SelfCollideDistance_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideDistance","Self_Collide_Distance")
	
def Hair_Control_SelfCollideStiffness_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideStiffness","Self_Collide_Stiffness")
	
def Hair_Control_SelfCollideDamping_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideDamping","Self_Collide_Damp")
	
def Hair_Control_CollideExt_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"CollideExt","Collider_Ext")
	
def Hair_Control_CollideInt_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"CollideInt","Collider_Int")
	
def Hair_Control_CollideFriction_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"CollideFriction","Collider_Friction")
	
def Hair_Control_CollideDamp_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"CollideDamp","Collider_Damp")

def Hair_Control_CollideSticky_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"CollideSticky","ColliderSticky")
	
def Hair_Control_CollideInt_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"CollideInt","Collider_Int")
	
def Hair_Control_SelfCollideActive_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideActive","Self_Collide")
	
def Hair_Control_SelfCollideDistance_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideDistance","Self_Collide_Distance")
	
def Hair_Control_SelfCollideFriction_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideFriction","Self_Collide_Friction")
	
def Hair_Control_SelfCollideDamping_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideDamping","Self_Collide_Damp")
	
def Hair_Control_MimicActive_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"MimicActive","Use_Mimic")
	
def Hair_Control_MimicDistance_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"MimicDistance","Mimic_Distance")
	
def Hair_Control_MimicStiffness_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"MimicStiffness","Mimic_Stiff")
	
def Hair_Control_MimicDamping_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"MimicDamping","Mimic_Damp")
	
def Hair_Control_RestoreShape_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"RestoreShape","Active")

def Hair_Control_RestoreBlend_OnChanged():
	Hair_Control_ParameterChanged(PPG.Inspected(0),"RestoreBlend","Blend ")
	
def Hair_Control_ParameterChanged(prop,propparam,nodeparam):
	cages = Hair_Control_GetHairObjects(prop)
	restore = propparam.find("Restore")>-1
	
	for c in cages:
		#xsi.LogMessage("Update Hair Dynamics Object ["+c.Name+"]"+opname+"."+opparam+" ---> "+str(PPG.Inspected(0).Parameters(param).Value))
		tree = c.ActivePrimitive.ICETrees("SyflexDynamics")
		if restore:
			tree = c.ActivePrimitive.ICETrees("DynamicsRestore")
		if not tree:
			XSI.LogMessage("[Hair_Control] No SyflexDynamics\RestoreShape ICETree under " + c.FullName)
			return
		node = tree.CompoundNodes("HairSyflexDynamics")
		if restore:
			node = tree.CompoundNodes("HairRestoreShape")
			
		if not node:
			XSI.LogMessage("[Hair_Control] Can't find compound Node under " + tree.FullName)
			return
		try:
			outparam = node.Parameters(nodeparam)
			if outparam == None:
				outparam = node.InputPorts(nodeparam)
			if not outparam:
				XSI.SetValue(str(node) + "." + nodeparam, PPG.Inspected(0).Parameters(propparam).Value, "")
			else:
				outparam.Value = PPG.Inspected(0).Parameters(propparam).Value
		except:
			XSI.LogMessage("[Hair_Control] Can't find InputPort " + nodeparam + " under " + node.FullName + " !", constants.siWarning)
		
def Hair_Control_PresetMethod_OnChanged():
	Hair_Control_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
	
def Hair_Control_PresetFolder_OnChanged():
	Hair_Control_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
	
def Hair_Control_ApplyBindPose(prop):
	model = prop.Parent3DObject
	bindpose = lnk.GetSimuBindPose(model)
	
	hips = model.FindChild2("Hips",constants.siNullPrimType,constants.si3DObjectFamily,True)
	firstframe = prop.Parameters("FirstFrame").Value
	preroll = prop.Parameters("PreRoll").Value
	endframe = uti.GetEndFrame()
	startframe = firstframe-preroll
	hipstranslation = model.FindChild2("HipsTranslation")
	
	if hips and hipstranslation:
		XSI.SelectObj("B:" + hipstranslation.FullName)
		XSI.SelectChildNodes()
	
		marked = XSI.SetMarking("kine.local.sclx,kine.local.scly,kine.local.sclz,kine.local.rotx,kine.local.roty,kine.local.rotz,kine.local.posx,kine.local.posy,kine.local.posz")
		active = prop.Parameters("HairActive").Value
		if active:
			prop.Parameters("HairActive").Value = False
			Hair_Control_ParameterChanged(prop,"HairActive","Active")
			
		uti.SetCurrentFrame(firstframe)
		XSI.Refresh()
		XSI.SaveKeyOnKeyable(marked, firstframe)
	
		tmpaction = XSI.StoreAction(model, None, 2, "tmpAction", True, firstframe, endframe, True)
		XSI.RemoveAnimation(None, None, False, True, True, constants.siAnySource)
		if tmpaction:
			XSI.ApplyAction(tmpaction, model)
		if bindpose:
			XSI.ApplyAction(bindpose, model)
		XSI.SaveKeyOnKeyable(marked, startframe)
		
		uti.SetStartFrame(startframe)
		uti.SetCurrentFrame(startframe)
		
		if active:
			prop.Parameters("HairActive").Value = True
			Hair_Control_ParameterChanged(prop,"HairActive","Active")
		 
	XSI.LogMessage("[Hair_Control] Apply Simulation Bind Pose Called...")
		
def Hair_Control_SelectWindIcon_OnClicked():
	model = PPG.Inspected(0).Parent3DObject
	icon = model.FindChild("HairWindControl")
	if icon:
		XSI.SelectObj(icon)
		
def Hair_Control_SelectTurbIcon_OnClicked():
	model = PPG.Inspected(0).Parent3DObject
	icon = model.FindChild("HairTurbulenceControl")
	if icon:
		XSI.SelectObj(icon)
		
def Hair_Control_SaveDynamicsPreset_OnClicked():
	folder = PPG.PresetFolder.Value
	pname = PPG.PresetName.Value
	if folder and pname:
		try:
			XSIUtils.EnsureFolderExists(folder,false)
		except:
			XSI.LogMessage("[Hair_Control] Preset Folder Invalid!!", constants.siError)
			return
			
	path = XSIUtils.BuildPath(folder,pname+".xml")
	Hair_Control_WriteXML(path)

def Hair_Control_LoadDynamicsPreset_OnClicked():
	folder = PPG.PresetFolder.Value
	#pname = PPG.UIItems
	prop = PPG.Inspected(0)
	selected = prop.Parameters("PresetList").Value
	uiitems = prop.PPGLayout.Item("PresetList").UIItems
	pname = uiitems[selected*2]
	if folder and pname:
		try:
			XSIUtils.EnsureFolderExists(folder,false)
		except:
			XSI.LogMessage("[Hair_Control] Preset Folder Invalid!!", constants.siError)
			return
	
	nodes = Hair_GetDynamicNode(prop)
	if not nodes:	
		XSI.LogMessage("[Hair_Control_LoadDynamicsPreset] There are NO Hair Dynamics nodes under this character...", constants.siWarning)
		return
		
	path = XSIUtils.BuildPath(folder,pname)
	Hair_Control_ReadXML(path)
	
	# Refresh the UI
	Hair_Control_CompoundToProperty(prop,nodes[0])
	PPG.Refresh()
	
def Hair_Control_DeleteDynamicsPreset_OnClicked():
	folder = PPG.PresetFolder.Value
	pname = PPG.PresetName.Value
	if folder and pname:
		try:
			XSIUtils.EnsureFolderExists(folder,false)
		except:
			XSI.LogMessage("[Hair_Control] Preset Folder Invalid!!", constants.siError)
			return
			
	path = XSIUtils.BuildPath(folder,pname+".xml")
	Hair_Control_ReadXML(path)

#--------------------------------------------
# Cache
#--------------------------------------------
def Hair_Control_SetSyflexCache_OnClicked():
	seqpath = Hair_Control_GetSequencePath()
	if not seqpath:
		return
		
	prop = PPG.Inspected(0)
	hairs = Hair_Control_GetHairObjects(prop)
	for h in hairs:
		hist = h.ActivePrimitive.ConstructionHistory
		for op in hist:
			if op.Name == "SyflexDynamics":
				compound = op.CompoundNodes("HairSyflexDynamics")
				cachenodes = []
				for n in compound.Nodes:
					if n.Type == "CacheOnFileNode":
						cachenodes.append(n)
				for c in cachenodes:
					c.Parameters("FilePath").Value = XSIUtils.BuildPath(seqpath,"Cache","Tmp")
		
def Hair_Control_ClearSyflexCache_OnClicked():
	scene = XSI.ActiveProject2.ActiveScene
	
	folder = XSI.InstallationPath(constants.siProjectPath)
	scene = XSI.ActiveProject.ActiveScene
	folder = XSIUtils.BuildPath(folder,"Caches",scene)
	
	prop = PPG.Inspected(0)
	cloth = Hair_Control_GetClothObjects(prop)
	model = cloth[0].Model
	
	# Reset to First Frame
	remote = XSI.Dictionary.GetObject("PlayControl")
	v = remote.Parameters("In").Value
	remote.Parameters( "Current" ).Value = prop.Parameters("FirstFrame").Value
	
	#deactivate delta storing state
	previousdelta = uti.DeactivateDelta(model)
	
	for c in cloth:
		syflex = c.ActivePrimitive.ConstructionHistory.Find( "syCloth" )
		#clear all cache.
		XSI.LogMessage("{Hair_Control] Delete All Syflex Cache File for " + c.FullName)
		syflex.Parameters("ClearCache").Value = 2
		syflex.Parameters("Cache").Value = ""

	#reactivate delta
	if previousdelta>-1:
		uti.ReactivateDelta(model,previousdelta)
	'''
	for ( var i=0; i<PPG.Inspected.Count; i++ )
	{
		var oPPG = PPG.Inspected(i);
		var oModel = oPPG.Parent3DObject;
		var oClothObject = umapGetClothObject(oPPG);
		var oSyflexOp = oClothObject.ActivePrimitive.ConstructionHistory.Find( "syCloth" );
		
		// GetParams.
		var SyflexCacheFileName = oPPG.SyflexCacheFileName.Value;
		
		// Firstframe.
		var oRemoteControl = Dictionary.GetObject( "PlayControl" );
		var inValue = oRemoteControl.Parameters("In").Value;
		oRemoteControl.Parameters( "Current" ).Value = inValue;

		// Desactive Static Pos Delta options.
		try	{ var oDelta = Dictionary.GetObject( oModel.FullName + ".Delta"); }
		catch (e) {}
		if ( oDelta )
		{
			var nMask_previous = oDelta.persist_modifications.Value;
			var nMask = nMask_previous;

			if ( nMask & 1 == 1 ) { nMask = nMask - 1; }// Desactive Static Pos.

			oDelta.persist_modifications = nMask;
		}
		
		// ClearCache.
		oSyflexOp.Cache.Value = SyflexCacheFileName;
		oSyflexOp.ClearCache.Value = 2; // clear all cache.
		oSyflexOp.Cache.Value = "";

		// Restore previous Deltas options.
		if ( oDelta ) { oDelta.persist_modifications.Value = nMask_previous; }

		oPPG.SyflexCacheFileName.Value = "";
	'''
	
def Hair_Control_SetDampingPercentage_OnClicked():
	prop = PPG.Inspected(0)
	
	tmp = XSI.ActiveSceneRoot.AddProperty("CustomProperty", false, "Hair_Control_SetDampingPercentage")
	p = tmp.AddParameter3( "Value", constants.siFloat, 100, 1, 1000, False, False )
	layout = tmp.PPGLayout
	layout.Clear()
	layout.AddGroup("Set Damping : Stiffness divided by ")
	layout.AddItem( "Value")
	layout.EndGroup()
	
	bCancelled  = XSI.InspectObj(tmp, None, "Set Syflex Damping", constants.siModal, False)
	if not bCancelled:
		prop.StretchDamping.Value = prop.StretchStiffness.Value / p.Value
		prop.ShearDamping.Value = prop.ShearStiffness.Value / p.Value
		prop.BendDamping.Value = prop.BendStiffness.Value / p.Value
		Hair_Control_StretchDamping_OnChanged()
		Hair_Control_ShearDamping_OnChanged()
		Hair_Control_BendDamping_OnChanged()

		XSI.DeleteObj(tmp)

	else:
		XSI.DeleteObj(tmp)
		XSI.LogMessage("[Hair_Control]Set Syflex Damping Aborted..")
	
def Hair_Control_GetPresetList():
	path = PPG.PresetFolder.Value
	aPresets = []
	if path:
		try:
			XSIUtils.EnsureFolderExists(path)
		except:
			return
			
		regex = ".*\.xml$"
		files = XSI.FindFilesInFolder(path, regex, True, False)
		
		i = 0
		for f in files:
			aPresets.append(f)
			aPresets.append(i)
			i+=1
		return aPresets
	return None
	
	
def Hair_Control_WriteXML(filename=None):
	prop = PPG.Inspected(0)
	if not filename:
		return

	nodes = Hair_GetDynamicNode(prop)
	
	if nodes:	
		xml.WriteCompoundsXMLPreset(filename,nodes)
	Hair_Control_RebuildLayout(prop)
	PPG.Refresh()

def Hair_Control_ReadXML(filename=None):
	prop = PPG.Inspected(0)
	if not filename:
		return
		
	nodes = Hair_GetDynamicNode(prop)
	
	if nodes:	
		xml.ReadCompoundsXMLPreset(filename,nodes)
		
def Hair_GetDynamicNode(prop):
	nodes = []
	
	cages = Hair_Control_GetHairObjects(prop)
	for c in cages:
		tree = c.ActivePrimitive.ICETrees("SyflexDynamics")
		if tree:
			compound = tree.CompoundNodes("HairSyflexDynamics")
			nodes.append(compound)
	
	return nodes

#--------------------------------------------
# Paint Maps
#--------------------------------------------
def Hair_Control_PaintPush_OnClicked():
	prop = PPG.Inspected(0)
	hairs = Hair_Control_GetHairObjects(prop)
	if len(hairs) == 1:
		selectedpush = prop.CorrectivePushList.value
		cls = hairs[0].ActivePrimitive.Geometry.Clusters("CorrectivePushCls")
		XSI.SelectObj(cls.Properties(selectedpush))
		XSI.PaintTool()
	else:
		XSI.LogMessage("[Hair_Control] Paint Corrective Push only works in Single Item Mode!!", constants.siError)


def Hair_Control_PaintRestoreMap_OnClicked():
	prop = PPG.Inspected(0)
	hairs = Hair_Control_GetHairObjects(prop)
	
	if len(hairs)==1:
		cls = hairs[0].ActivePrimitive.Geometry.Clusters("WeightMapCls");
		if not cls:
			return
		
		map = cls.Properties("Restore_Map")
		if map:
			Hair_Control_PaintMap(map)
		else:
			XSI.LogMessage("[Hair_Control]Can't find Restore Map on " + hairs[0].FullName)
		
	else:
		XSI.LogMessage("[Hair_Control] Paint Restore Map only works in Single Item Mode!!", constants.siError)

def Hair_Control_PaintDynamicsMap_OnClicked():
	prop = PPG.Inspected(0)
	hairs = Hair_Control_GetHairObjects(prop)
	
	if len(hairs)==1:
		cls = hairs[0].ActivePrimitive.Geometry.Clusters("WeightMapCls");
		if not cls:
			return
		
		map = cls.Properties("Dynamics_Map")
		if map:
			Hair_Control_PaintMap(map)
		else:
			XSI.LogMessage("[Hair_Control]Can't find Dynamics Map on " + hairs[0].FullName)
		
	else:
		XSI.LogMessage("[Hair_Control] Paint Dynamics Map only works in Single Item Mode!!", constants.siError)
	
	
def Hair_Control_PaintMap(map):
	object = map.Parent3DObject
	object.Properties("Visibility").Parameters("ViewVis").Value = True
	XSI.SelectObj(map)
	view = XSI.OpenView("Object View", True)
	view.SetAttributeValue("lockstatus",True)
	XSI.PaintTool()
	
	
def Hair_Control_InspectRestoreNode_OnClicked():
	prop = PPG.Inspected(0)
	hairs = Hair_Control_GetHairObjects(prop)
	inspect = []
	for h in hairs:
		tree = h.ActivePrimitive.ICETrees.Find("HairRestore")
		if tree:
			node = tree.CompoundNodes("HairRestoreShape")
			if node:
				inspect.append(node)
	XSI.InspectObj(inspect)
	
def Hair_Control_GetSequencePath():
	assetmanagement = XSI.ActiveSceneRoot.Properties("AssetManagement")
	if not assetmanagement:
		XSI.LogMessage("[Hair_Control_SetSyflexCache] No AssetManagement property in scene!!", constants.siWarning)
		return None
		
	animpath = assetmanagement.Parameters("anim_path").Value
	split = animpath.split("\\")
	last = split.pop()
	basepath = animpath.replace(last,"")
	return basepath

		