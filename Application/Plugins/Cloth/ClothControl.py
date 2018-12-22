#---------------------------------------
# 	Cloth_Control
#---------------------------------------
from Globals import xsi
import Cloth as clo
import Utils as uti
import XML as xml
import Correct as correct
import Property as pro
import os


#---------------------------------------
# Load Plugin
#---------------------------------------
def XSILoadPlugin( in_reg ):
	in_reg.Author = "benmalartre"
	in_reg.Name = "ClothControl"
	in_reg.Email = "benmalartre@hotmail.com"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterProperty("Cloth_Control")
	in_reg.RegisterCommand("Cloth_CorrectiveShape","Cloth_CorrectiveShape")
	in_reg.RegisterCommand("Cloth_CorrectiveSmooth","Cloth_CorrectiveSmooth")
	in_reg.RegisterCommand("Cloth_CorrectivePush","Cloth_CorrectivePush")
		
	return True
	
#---------------------------------------
# Define
#---------------------------------------
def Cloth_Control_Define( in_ctxt ):
	prop = in_ctxt.Source
	
	# cloth objects
	prop.AddParameter3("ClothObjects",constants.siString)
	prop.AddParameter3("ClothObjectChooser",constants.siInt4,0,0,100,False,False)
	
	prop.AddParameter3("ClothObjectIconFolder",constants.siString)
	prop.AddParameter3("ClothObjectIcon",constants.siString)
	prop.AddParameter3("ClothControlBanner", constants.siString)
	
	#common parameters
	prop.AddParameter3("ClothActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("FirstFrame",constants.siInt4,1,-10000,10000,False,False)
	prop.AddParameter3("PreRoll",constants.siInt4,12,1,100,False,False)
	prop.AddParameter3("Precision",constants.siFloat,0.001,0.0001,1,False,False)
	prop.AddParameter3("SubSampling",constants.siFloat,0.0,0.0,1.0,False,False)
	
	#properties
	prop.AddParameter3("Density",constants.siFloat,0.5,0.01,6,False,False)
	prop.AddParameter3("Rescale",constants.siFloat,1,0.1,10.0,False,False)
	prop.AddParameter3("StretchStiffness",constants.siFloat,1,0.01,100,False,False)
	prop.AddParameter3("ShearStiffness",constants.siFloat,0.2,0.01,100,False,False)
	prop.AddParameter3("BendStiffness",constants.siFloat,0.1,0.01,100,False,False)
	prop.AddParameter3("StretchDamping",constants.siFloat,0.1,0.0001,1,False,False)
	prop.AddParameter3("ShearDamping",constants.siFloat,0.02,0.0001,1,False,False)
	prop.AddParameter3("BendDamping",constants.siFloat,0.01,0.0001,1,False,False)
	
	#gravity: we should connect it to icon and multiply by factor
	prop.AddParameter3("GravityActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("GravityX",constants.siFloat,0.0)
	prop.AddParameter3("GravityY",constants.siFloat,-0.1)
	prop.AddParameter3("GravityZ",constants.siFloat,0.0)
	prop.AddParameter3("GravityFactor",constants.siFloat,1.0,0.0,10.0,False,False)
	
	#wind: we should connect it to icon and multiply by factor
	prop.AddParameter3("WindActive",constants.siBool,False,False,True,False,False)
	prop.AddParameter3("WindX",constants.siFloat,1.0,0.0,10.0,False,False)
	prop.AddParameter3("WindY",constants.siFloat,0.0,0.0,10.0,False,False)
	prop.AddParameter3("WindZ",constants.siFloat,0.0,0.0,10.0,False,False)
	prop.AddParameter3("WindPerturbX",constants.siFloat,0.0,0.0,10.0,False,False)
	prop.AddParameter3("WindPerturbY",constants.siFloat,0.0,0.0,10.0,False,False)
	prop.AddParameter3("WindPerturbZ",constants.siFloat,0.0,0.0,10.0,False,False)
	prop.AddParameter3("WindIntensity",constants.siFloat,0.1,0.0,10.0,False,False)
	prop.AddParameter3("WindTime",constants.siFloat,10.0,0.0,10.0,False,False)
	prop.AddParameter3("WindFactor",constants.siFloat,1.0,0.0,10.0,False,False)
	prop.AddParameter3("WindOneSide",constants.siBool,False,False,True,False,False)
	
	#air
	prop.AddParameter3("AirActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("AirDamping",constants.siFloat,0.05,0.0,1.0,False,False)
	
	#mimic
	prop.AddParameter3("RigidMimicActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("SoftMimicActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("SoftMimicDistance",constants.siFloat,0.001,0.0,1.0,False,False)
	prop.AddParameter3("SoftMimicStiffness",constants.siFloat,0.05,0.0,1.0,False,False)
	prop.AddParameter3("SoftMimicDamping",constants.siFloat,0.01,0.0,1.0,False,False)
	
	#self collide
	prop.AddParameter3("SelfCollideActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("SelfCollideEnvelope",constants.siFloat,0.05,0.0,1.0,False,False)
	
	#collide
	prop.AddParameter3("CollideObjects",constants.siString)
	prop.AddParameter3("CollideObjectChooser",constants.siInt4,0,0,100,False,False)
	prop.AddParameter3("CollideActive",constants.siBool,True,False,True,False,False)
	prop.AddParameter3("CollideFriction",constants.siFloat,0.2,0.0,1.0,False,False)
	prop.AddParameter3("CollideDamping",constants.siFloat,0.01,0.0,1.0,False,False)
	prop.AddParameter3("CollideBounce",constants.siFloat,0.0,0.0,1.0,False,False)
	prop.AddParameter3("CollideExtEnvelope",constants.siFloat,0.05,0.0,1.0,False,False)
	prop.AddParameter3("CollideIntEnvelope",constants.siFloat,-0.05,-1.0,0.0,False,False)
	
	# correction lists
	prop.AddParameter3("CorrectiveShapeList", constants.siString);
	prop.AddParameter3("CorrectiveSmoothList", constants.siString);
	prop.AddParameter3("CorrectivePushList", constants.siString);
	
	# Output Cache
	prop.AddParameter("ScenePath",constants.siString);
	prop.AddParameter("OutputFolder",constants.siString);
	prop.AddParameter3("OutputFolderSelection",constants.siInt4,0,0,2,False,False);


def Cloth_Control_OnInit():
	prop = PPG.Inspected(0)
	path = os.path.join(__sipath__, "Icons")
	prop.Parameters("ClothObjectIconFolder").Value = path
	
	path = os.path.join(path, "Cloth.bmp")
	prop.ClothObjectIcon.Value = path
	Cloth_Control_RebuildLayout(prop)
	PPG.Refresh()
	
	
def Cloth_Control_RebuildLayout(prop):
	layout = prop.PPGLayout
	layout.Clear()
	
	layout.AddTab("Cloth")
	Cloth_Control_AddClothObjectChooser(prop)
	
	grp = layout.AddGroup("Globals")
	layout.AddItem("ClothActive","Active")
	layout.AddItem("FirstFrame","First Frame")
	layout.AddItem("PreRoll","Pre Roll")
	layout.AddItem("Precision")
	layout.AddItem("SubSampling")
	layout.EndGroup()

	layout.AddGroup("Properties")
	layout.AddItem("Density","Density")
	layout.AddItem("Rescale","Rescale")
	layout.EndGroup()
	layout.AddGroup("Springs")
	layout.AddItem("StretchStiffness")
	layout.AddItem("ShearStiffness")
	layout.AddItem("BendStiffness")
	
	btn = layout.AddButton("SetDampingPercentage","Set Damping Percentage")
	btn.SetAttribute(constants.siUICX,280)
	btn.SetAttribute(constants.siUICY,30)
	
	layout.AddItem("StretchDamping")
	layout.AddItem("ShearDamping")
	layout.AddItem("BendDamping")
	
	layout.EndGroup()
	
	layout.AddTab("Forces")
	Cloth_Control_AddClothObjectChooser(prop)
	layout.AddGroup("Gravity")
	layout.AddItem("GravityActive")
	layout.AddItem("GravityFactor")
	btn = layout.AddButton("SelectGravityIcon","SelectGravityIcon")
	btn.SetAttribute(constants.siUICX,280)
	layout.EndGroup()
	layout.AddGroup("Wind")
	layout.AddItem("WindActive")
	layout.AddItem("WindFactor")
	layout.AddItem("WindTime")
	layout.AddItem("WindOneSide")
	btn = layout.AddButton("SelectWindIcon","SelectWindIcon")
	btn.SetAttribute(constants.siUICX,280)
	layout.EndGroup()
	layout.AddGroup("Air")
	layout.AddItem("AirActive")
	layout.AddItem("AirDamping")
	layout.EndGroup()
	
	layout.AddTab("Mimic")
	Cloth_Control_AddClothObjectChooser(prop)
	layout.AddGroup("Rigid Mimic")
	layout.AddItem("RigidMimicActive","Active")
	layout.EndGroup()
	layout.AddGroup("Soft Mimic")
	layout.AddItem("SoftMimicActive","Active")
	layout.AddItem("SoftMimicDistance","Distance")
	layout.AddItem("SoftMimicStiffness","Stiffness")
	layout.AddItem("SoftMimicDamping","Damping")
	btn = layout.AddButton("PaintSoftMimic","PAINT")
	btn.SetAttribute(constants.siUICX,280)
	layout.EndGroup()

	Cloth_Control_AddCollideTab(prop)
	Cloth_Control_AddCorrectionTab(prop)
	Cloth_Control_AddCacheTab(prop)
	
def Cloth_Control_AddCollideTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Collide")
	cloth = Cloth_Control_AddClothObjectChooser(prop)
	collider = Cloth_Control_AddColliderObjectChooser(prop)

	layout.AddGroup("Collide")
	layout.AddItem("CollideActive","Active")
	layout.AddItem("CollideFriction","Friction")
	layout.AddItem("CollideDamping","Damping")
	layout.AddItem("CollideBounce","Bounce")
	layout.AddItem("CollideExtEnvelope","ExtEnvelope")
	layout.AddItem("CollideIntEnvelope","IntEnvelope")
	layout.EndGroup()
	
	layout.AddGroup("Self Collide")
	layout.AddItem("SelfCollideActive","Active")
	layout.AddItem("SelfCollideEnvelope","Envelope")
	layout.EndGroup()
	
def Cloth_Control_AddCorrectionTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Correct")
	
	Cloth_Control_AddClothObjectChooser(prop)
	
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes) == 1:
	
		layout.AddGroup("Restore Shape")
		btn = layout.AddButton("PaintRestoreMap","PAINT")
		btn.SetAttribute(constants.siUICX,280)
		btn = layout.AddButton("InspectRestoreNode","INSPECT")
		btn.SetAttribute(constants.siUICX,280)
		layout.EndGroup()
	
		layout.AddGroup("Smooth Map")
		layout.AddRow()
		
		layout.AddGroup()
		button = layout.AddButton("StoreCorrectiveSmooth","Store Smooth")
		button.SetAttribute(constants.siUICX,120)
		button = layout.AddButton("InspectCorrectiveSmooth","Inspect Smooth")
		button.SetAttribute(constants.siUICX,120)
		button = layout.AddButton("DeleteCorrectiveSmooth","Delete Smooth")
		button.SetAttribute(constants.siUICX,120)
		button = layout.AddButton("DeleteAllCorrectiveSmooth","Delete All Smooth")
		button.SetAttribute(constants.siUICX,120)
		layout.EndGroup()
		
		layout.AddGroup()
		smoothlist = Cloth_Control_GetCorrectiveList(prop,"Smooth")
		item = layout.AddEnumControl("CorrectiveSmoothList",smoothlist,"",constants.siControlListBox)
		item.SetAttribute(constants.siUINoLabel,True)
		item.SetAttribute(constants.siUICY,88)
		layout.EndGroup()
		
		layout.EndRow()
		layout.AddRow()
		layout.AddSpacer()
		layout.AddButton("PaintSmooth","Paint Selected Smooth Map")
		layout.EndRow()
		layout.EndGroup()

		layout.AddGroup("Push Map")
		layout.AddRow()
		
		layout.AddGroup()
		button = layout.AddButton("StoreCorrectivePush","Store Push")
		button.SetAttribute(constants.siUICX,120)
		button = layout.AddButton("InspectCorrectivePush","Inspect Push")
		button.SetAttribute(constants.siUICX,120)
		button = layout.AddButton("DeleteCorrectivePush","Delete Push")
		button.SetAttribute(constants.siUICX,120)
		button = layout.AddButton("DeleteAllCorrectivePush","Delete All Push")
		button.SetAttribute(constants.siUICX,120)
		layout.EndGroup()
		
		layout.AddGroup()
		pushlist = Cloth_Control_GetCorrectiveList(prop,"Push")
		item = layout.AddEnumControl("CorrectivePushList",pushlist,"",constants.siControlListBox)
		item.SetAttribute(constants.siUINoLabel,true)
		item.SetAttribute(constants.siUICY,88)
		layout.EndGroup()
		
		layout.EndRow()
		layout.AddRow()
		layout.AddSpacer()
		layout.AddButton("PaintPush","Paint Selected Push Map")
		layout.EndRow()
		layout.EndGroup()
		
		layout.AddGroup("Corrective Shape")
		layout.AddRow()
		
		layout.AddGroup()
		button = layout.AddButton("StoreCorrectiveShape","Store Shape")
		button.SetAttribute(constants.siUICX,120)
		button = layout.AddButton("InspectCorrectiveShape","Inspect Shape")
		button.SetAttribute(constants.siUICX,120)
		button = layout.AddButton("DeleteCorrectiveShape","Delete Shape")
		button.SetAttribute(constants.siUICX,120)
		button = layout.AddButton("DeleteAllCorrectiveShape","Delete All Shape")
		button.SetAttribute(constants.siUICX,120)
		layout.EndGroup()
		
		layout.AddGroup()
		shapelist = Cloth_Control_GetCorrectiveList(prop,"Shape")
		item = layout.AddEnumControl("CorrectiveShapeList",shapelist,"",constants.siControlListBox)
		item.SetAttribute(constants.siUINoLabel,True)
		item.SetAttribute(constants.siUICY,88)
		layout.EndGroup()
		
		layout.EndRow()
		layout.EndGroup()
	
	else:
		layout.AddGroup("Correction")
		layout.AddStaticText("Correction NOT Available\n when operating on All Cloth Objects!!")
		layout.EndGroup()
		
		
def Cloth_Control_AddCacheTab(prop):
	layout = prop.PPGLayout
	layout.AddTab("Cache")
	Cloth_Control_AddClothObjectChooser(prop)
	layout.AddGroup("Syflex Cache")
	btn = layout.AddButton("SetSyflexCache","Set Path")
	btn.SetAttribute(constants.siUICX,280)
	btn = layout.AddButton("ClearSyflexCache","Clear")
	btn.SetAttribute(constants.siUICX,280)
	layout.EndGroup()
	layout.AddGroup("Mode")
	readcachearray = ["Simulate",0,"Read Cache",1]
	item = layout.AddEnumControl("ReadCache",readcachearray,"Mode",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,true)
	layout.EndGroup()

	layout.AddGroup(" Point Cache")
	
	layout.AddGroup()
	outputfolderselection = ["Track",0,"Alternative Folder",1]
	item = layout.AddEnumControl("OutputFolderSelection",outputfolderselection,"Output Folder",constants.siControlCombo)
	item.SetAttribute(constants.siUINoLabel,True)
	outputfolder = prop.Parameters("OutputFolderSelection").Value
	
	layout.AddEnumControl("ScenePath",[],"Scene Path",constants.siControlCombo);

	''' Track'''
	
	layout.AddItem( "OutputFolder","Folder", constants.siControlFolder)
	
	if outputfolder == 0:
		prop.OutputFolder.SetCapabilityFlag(constants.siNotInspectable,True)
		prop.ScenePath.SetCapabilityFlag(constants.siNotInspectable,False)
	
	else:
		prop.OutputFolder.SetCapabilityFlag(constants.siNotInspectable,False);
		prop.ScenePath.SetCapabilityFlag(constants.siNotInspectable,True);
	
	layout.EndGroup()
	layout.AddRow()
	layout.AddSpacer()
	layout.AddButton("CacheDeleteCache", " Delete Cache ")
	layout.AddButton("CacheWriteCache", " Write Cache ")
	layout.EndRow()
	
	layout.EndGroup()
	
	
def Cloth_Control_AddClothObjectChooser(prop):
	layout = prop.PPGLayout

	grp = layout.AddGroup("Choose Cloth Object")
	layout.AddRow()
	items = Cloth_Control_GetClothObjectsList(prop)
	if items:
		item = layout.AddEnumControl( "ClothObjectChooser", items, "ClothObjectChooser", constants.siControlCombo)
		item.SetAttribute(constants.siUINoLabel, True)
		item.SetAttribute(constants.siUIWidthPercentage,90)
		item.SetAttribute(constants.siUICY,64)
		
		item = layout.AddItem("ClothObjectIcon","",constants.siControlBitmap)
		item.SetAttribute(constants.siUIFilePath,PPG.ClothObjectIcon.Value)
		item.SetAttribute(constants.siUINoLabel, True)
		
	else:
		layout.AddStaticText("There are NO cloth objects under this model!!")
		return
		
	layout.EndRow()
	layout.EndGroup()
	
	if items:
		return layout.Item("ClothObjectChooser").UIItems[prop.Parameters("ClothObjectChooser").Value * 2]
	
	
def Cloth_Control_AddColliderObjectChooser(prop):
	layout = prop.PPGLayout
	model = prop.Parent3DObject
	layout.AddGroup("Choose Collider Object")
	items = Cloth_Control_GetColliderObjectsList(prop)
	if items:
		item = layout.AddEnumControl( "CollideObjectChooser", items, "CollideObjectChooser", constants.siControlCombo)
		item.SetAttribute(constants.siUINoLabel, True)
	layout.EndGroup()
	
	'''
	if items and len(items)>2:
		collider = model.FindChild(layout.Item("CollideObjectChooser").UIItems[prop.Parameters("CollideObjectChooser").Value * 2])
		return collider
	else:
		return None
	'''
	

def Cloth_Control_GetClothObjectsList(prop):
	model = prop.Parent3DObject
	cloth = clo.FindSyflexMeshes(model)
	
	items = ["All Cloth Objects",0]
	for c in range(len(cloth)):
		#items.append(cloth(c).Name.replace("_Cloth",""))
		items.append(cloth[c].Name)
		items.append(c+1)
	
	if len(items) == 2:
		return None
	
	return items
	
	
def Cloth_Control_GetColliderObjectsList(prop):
	model = prop.Parent3DObject
	prop = PPG.Inspected(0)
	uiitems = PPG.PPGLayout.Item("ClothObjectChooser").UIItems
	items = ["All Collider Objects",0]
	value = PPG.ClothObjectChooser.Value
	
	if value>0:
		selected = uiitems[value*2]
		
		model = PPG.Inspected(0).Parent3DObject
		cloth = model.FindChild(selected)
		colliders = clo.FindColliderMeshes(cloth)
		
		for c in range(len(colliders)):
			items.append(colliders[c].Name)
			items.append(c+1)
		
		if len(items) == 2:
			prop.PPGLayout.AddStaticText("NO colliders for this Cloth!!")
			return None
		return items
		
	else:
		prop.CollideObjectChooser.Value = 0
		
	return items


def Cloth_Control_GetClothObjects(prop):
	model = PPG.Inspected(0).Parent3DObject
	items = prop.PPGLayout.Item("ClothObjectChooser").UIItems
	value = prop.ClothObjectChooser.Value
	if value>0:
		selected = items[value*2]
		
		cloth = model.FindChild(selected)
		return [cloth]
	else:
		cloth = []
		for i in range(1,len(items)/2):
			c = model.FindChild(items[i*2])
			cloth.append(c)
		return cloth
		
def Cloth_Control_GetTargetObjects(prop):
	model = prop.Parent3DObject
	clothes = Cloth_Control_GetClothObjects(prop)
	targets = []
	for c in clothes:
		tree = c.ActivePrimitive.ICETrees.Find("ClothInit")
		if tree:
			node = tree.Nodes("ClothInitShape")
			targetname = xsi.GetValue(str(node)+".Reference")
			targetname = targetname.replace("this_model",model.Name)
			target = xsi.Dictionary.GetObject(targetname)
			if target:
				targets.append(target)
				
	return targets

def Cloth_Control_GetColliderObjects(prop):
	model = prop.Parent3DObject
	clothes = Cloth_Control_GetClothObjects(prop)
	colliders = []
	for c in clothes:
		coll = clo.FindColliderMeshes(c)
		
		for cc in coll:
			colliders.append(cc)

	return colliders	
	
def Cloth_Control_GetOutObjects(prop):
	model = prop.Parent3DObject
	clothes = Cloth_Control_GetClothObjects(prop)
	outmeshes = []
	for c in clothes:
		out = xml.GetCageDeformedObjectFromConnectionStack(c)
		if out:
			outmeshes.append(out)
		else:
			outmeshes.append(c)
	return outmeshes
			
			
#-------------------------------------------------
# Parameters changed callback
#-------------------------------------------------
def Cloth_Control_ClothObjectChooser_OnChanged():
	prop = PPG.Inspected(0)
	
	items = PPG.PPGLayout.Item("ClothObjectChooser").UIItems
	value = PPG.ClothObjectChooser.Value
	path = prop.ClothObjectIconFolder.Value
	xsi.LogMessage(path)
	if value>0:
		selected = items[value*2]
		
		model = PPG.Inspected(0).Parent3DObject
		cloth = model.FindChild(selected)
		Cloth_Control_SyflexToProperty(prop,cloth)
		path += "\\"+cloth.Name+".bmp"
		
	else:
		path += "\\All_Cloth_Objects.bmp"
	
	PPG.ClothObjectIcon.Value = path
	Cloth_Control_RebuildLayout(prop)
	PPG.Refresh()
		
def Cloth_Control_CollideObjectChooser_OnChanged():
	prop = PPG.Inspected(0)
	Cloth_Control_CollideToProperty(prop)
	PPG.Refresh()
	
def Cloth_Control_FindCollideOp(syflex,id):
	collides = []
	for n in syflex.NestedObjects:
		if n.Type == "syCollide":
			collides.append(n)
			xsi.LogMessage(n)
	
	if len(collides)>=id:
		return collides[id]
	return None
	
def Cloth_Control_CollideToProperty(prop):
	prop = PPG.Inspected(0)
	items = prop.PPGLayout.Item("CollideObjectChooser").UIItems
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes)==1:
		value = PPG.CollideObjectChooser.Value
		
		if value>0:
			model = PPG.Inspected(0).Parent3DObject
			syflex = clothes[0].ActivePrimitive.ConstructionHistory.Find("syCloth")
			op = Cloth_Control_FindCollideOp(syflex,value-1)
			if op:
				xsi.LogMessage("We will treat Collide : "+op.FullName)
				prop.Parameters("CollideActive").Value = op.Parameters("Active").Value
				prop.Parameters("CollideFriction").Value = op.Parameters("Friction").Value
				prop.Parameters("CollideDamping").Value = op.Parameters("Damp").Value
				prop.Parameters("CollideBounce").Value = op.Parameters("Bounce").Value
				prop.Parameters("CollideExtEnvelope").Value = op.Parameters("ExtEnvelope").Value
				prop.Parameters("CollideIntEnvelope").Value = - op.Parameters("IntEnvelope").Value
			
		else:
			xsi.LogMessage("You are actually driving ALL Collider Objects")
	else:
		xsi.LogMessage("You are actually driving ALL Cloth Objects")
		
def Cloth_Control_SyflexToProperty(prop,obj):
	if not clo.IsSyflex(obj):
		return None
		
	#Globals
	syflex = obj.ActivePrimitive.ConstructionHistory.Find("syCloth")
	prop.Parameters("ClothActive").Value = syflex.Parameters("Active").Value
	prop.Parameters("FirstFrame").Value = syflex.Parameters("FirstFrame").Value
	prop.Parameters("Precision").Value = syflex.Parameters("Precision").Value
	prop.Parameters("SubSampling").Value = syflex.Parameters("SubSampling").Value
	
	#Properties
	properties = xsi.Dictionary.GetObject(syflex.NestedObjects.Find("syProperties"))
	prop.Parameters("Density").Value = properties.Parameters("Density").Value
	prop.Parameters("ShearStiffness").Value = properties.Parameters("ShearStiffness").Value
	prop.Parameters("StretchStiffness").Value = properties.Parameters("StretchStiffness").Value
	prop.Parameters("ShearStiffness").Value = properties.Parameters("ShearStiffness").Value
	prop.Parameters("BendStiffness").Value = properties.Parameters("BendStiffness").Value
	prop.Parameters("StretchDamping").Value = properties.Parameters("StretchDamping").Value
	prop.Parameters("ShearDamping").Value = properties.Parameters("ShearDamping").Value
	prop.Parameters("BendDamping").Value = properties.Parameters("BendDamping").Value
	prop.Parameters("Rescale").Value = properties.Parameters("Rescale").Value
	
	#Gravity
	gravity = xsi.Dictionary.GetObject(syflex.NestedObjects.Find("syGravity"))
	prop.Parameters("GravityActive").Value = gravity.Parameters("Active").Value

	#Wind
	wind = xsi.Dictionary.GetObject(syflex.NestedObjects.Find("syWind"))
	prop.Parameters("WindActive").Value = wind.Parameters("Active").Value
	prop.Parameters("WindTime").Value = wind.Parameters("TimeInterval").Value
	
	#Air
	air = xsi.Dictionary.GetObject(syflex.NestedObjects.Find("syAir"))
	prop.Parameters("AirActive").Value = air.Parameters("Active").Value
	prop.Parameters("AirDamping").Value = air.Parameters("Damp").Value
	
	#Rigid Mimic
	rigid = xsi.Dictionary.GetObject(syflex.NestedObjects.Find("syMimic"))
	prop.Parameters("RigidMimicActive").Value = rigid.Parameters("Active").Value
	
	#Soft Mimic
	for n in syflex.NestedObjects:
		if n.FullName.find("syMimic[1]")>-1:
			prop.Parameters("SoftMimicActive").Value = n.Parameters("Active").Value
			prop.Parameters("SoftMimicDistance").Value = n.Parameters("Distance").Value
			prop.Parameters("SoftMimicStiffness").Value = n.Parameters("Stiff").Value
			prop.Parameters("SoftMimicDamping").Value = n.Parameters("Damp").Value
			
	#Collide
	Cloth_Control_CollideToProperty(prop)
	
	#Self Collide
	selfcollide = xsi.Dictionary.GetObject(syflex.NestedObjects.Find("sySelfCollide"))
	prop.Parameters("SelfCollideActive").Value = selfcollide.Parameters("Active").Value
	prop.Parameters("SelfCollideEnvelope").Value = selfcollide.Parameters("Envelope").Value
	
def Cloth_Control_FirstFrame_OnChanged():
	prop = PPG.Inspected(0)
	Cloth_Control_ParameterChanged(prop,"FirstFrame","","FirstFrame")
	Cloth_Control_UpdatePreRoll(prop)

def Cloth_Control_PreRoll_OnChanged():
	prop = PPG.Inspected(0)
	Cloth_Control_UpdatePreRoll(prop)	
	
def Cloth_Control_UpdatePreRoll(prop):
	targets = Cloth_Control_GetTargetObjects(prop)
	for t in targets:
		tree = t.ActivePrimitive.ICETrees("PreRollTarget")
		if tree:
			node = tree.Nodes("ClothTargetPreRoll")
			if node:
				start = prop.Parameters("FirstFrame").Value
				end = start + prop.Parameters("PreRoll").Value
				node.InputPorts("Start_Frame").Value = start
				node.InputPorts("End_Frame").Value = end
		
	colliders = Cloth_Control_GetColliderObjects(prop)
	for c in colliders:
		tree = c.ActivePrimitive.ICETrees("PreRollCollide")
		if tree:
			node = tree.Nodes("ClothTargetPreRoll")
			if node:
				start = prop.Parameters("FirstFrame").Value
				roll = prop.Parameters("PreRoll").Value
				node.InputPorts("Start_Frame").Value = start
				node.InputPorts("End_Frame").Value = start + roll
		
def Cloth_Control_ClothActive_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"ClothActive","","Active")
	
def Cloth_Control_Precision_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"Precision","","Precision")
	
def Cloth_Control_SubSampling_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"SubSampling","","SubSampling")
	
def Cloth_Control_Density_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"Density","syProperties","Density")

def Cloth_Control_Rescale_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"Rescale","syProperties","Rescale")
	
def Cloth_Control_StretchStiffness_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"StretchStiffness","syProperties","StretchStiffness")
	
def Cloth_Control_ShearStiffness_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"ShearStiffness","syProperties","ShearStiffness")
	
def Cloth_Control_BendStiffness_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"BendStiffness","syProperties","BendStiffness")
	
def Cloth_Control_StretchDamping_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"StretchDamping","syProperties","StretchDamping")
	
def Cloth_Control_ShearDamping_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"ShearDamping","syProperties","ShearDamping")
	
def Cloth_Control_BendDamping_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"BendDamping","syProperties","BendDamping")
	
def Cloth_Control_GravityFactor_OnChanged():
	xsi.LogMessage("Udpdate Parameter for ScriptedOp GravityIconOp")

def Cloth_Control_GravityActive_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"GravityActive","syGravity","Active")
	
def Cloth_Control_WindFactor_OnChanged():
	xsi.LogMessage("Udpdate Parameter for ScriptedOp WindIconOp")

def Cloth_Control_WindActive_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"WindActive","syWind","Active")
	
def Cloth_Control_WindTime_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"WindTime","syWind","TimeInterval")
	
def Cloth_Control_WindOneSide_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"WindOneSide","syWind","OneSide")
	
def Cloth_Control_AirActive_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"AirActive","syAir","Active")

def Cloth_Control_AirDamping_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"AirDamping","syAir","Damp")
	
def Cloth_Control_RigidMimicActive_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"RigidMimicActive","syMimic","Active")

def Cloth_Control_SoftMimicActive_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"SoftMimicActive","syMimic[1]","Active")

def Cloth_Control_SoftMimicDistance_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"SoftMimicDistance","syMimic[1]","Distance")
	
def Cloth_Control_SoftMimicStiffness_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"SoftMimicStiffness","syMimic[1]","Stiff")
	
def Cloth_Control_SoftMimicDamping_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"SoftMimicDamping","syMimic[1]","Damp")

'''	
	#Rigid Mimic
	rigid = xsi.Dictionary.GetObject(syflex.NestedObjects.Find("syMimic"))
	prop.Parameters("RigidMimicActive").Value = rigid.Parameters("Active").Value
	
	#Soft Mimic
	for n in syflex.NestedObjects:
		if n.FullName.find("syMimic[1]")>-1:
			prop.Parameters("SoftMimicActive").Value = n.Parameters("Active").Value
			prop.Parameters("SoftMimicDistance").Value = n.Parameters("Distance").Value
			prop.Parameters("SoftMimicStiffness").Value = n.Parameters("Stiff").Value
			prop.Parameters("SoftMimicDamping").Value = n.Parameters("Damp").Value
'''

def Cloth_Control_CollideActive_OnChanged():
	prop = PPG.Inspected(0)
	collideName = Cloth_Control_GetActiveCollideName(prop)
	if collideName:
		Cloth_Control_ParameterChanged(PPG.Inspected(0),"CollideActive",collideName,"Active")
	
def Cloth_Control_CollideFriction_OnChanged():
	prop = PPG.Inspected(0)
	collideName = Cloth_Control_GetActiveCollideName(prop)
	if collideName:
		Cloth_Control_ParameterChanged(PPG.Inspected(0),"CollideFriction",collideName,"Friction")
	
def Cloth_Control_CollideDamping_OnChanged():
	prop = PPG.Inspected(0)
	collideName = Cloth_Control_GetActiveCollideName(prop)
	if collideName:
		Cloth_Control_ParameterChanged(PPG.Inspected(0),"CollideDamping",collideName,"Damp")
	

def Cloth_Control_CollideBounce_OnChanged():
	prop = PPG.Inspected(0)
	collideName = Cloth_Control_GetActiveCollideName(prop)
	if collideName:
		Cloth_Control_ParameterChanged(PPG.Inspected(0),"CollideBounce",collideName,"Bounce")
	
def Cloth_Control_CollideExtEnvelope_OnChanged():
	prop = PPG.Inspected(0)
	collideName = Cloth_Control_GetActiveCollideName(prop)
	if collideName:
		Cloth_Control_ParameterChanged(PPG.Inspected(0),"CollideExtEnvelope",collideName,"ExtEnvelope")
	
def Cloth_Control_CollideIntEnvelope_OnChanged():
	prop = PPG.Inspected(0)
	collideName = Cloth_Control_GetActiveCollideName(prop)
	if collideName:
		Cloth_Control_ParameterChanged(PPG.Inspected(0),"CollideIntEnvelope",collideName,"IntEnvelope")	
	
def Cloth_Control_SelfCollideActive_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideActive","sySelfCollide","Active")
	
def Cloth_Control_SelfCollideEnvelope_OnChanged():
	Cloth_Control_ParameterChanged(PPG.Inspected(0),"SelfCollideEnvelope","sySelfCollide","Envelope")
	
def Cloth_Control_GetActiveCollideName(prop):
	id = prop.CollideObjectChooser.Value
	if id == 0:
		return "syCollide[*]"
	
	collideName = "syCollide"
	if id>1:
		collideName +="["+(str(id-1))+"]"
	
	return collideName
		
def Cloth_Control_ParameterChanged(prop,param,opname,opparam):
	cloth = Cloth_Control_GetClothObjects(prop)
	indexed = False
	search = ""
	all = False
	
	if opname.find("[")>-1:
		split = opname.split("[")
		opname = split[0]
		search = "["+split[1]
		indexed = True
		if split[1] == "*]":
			all = True
		
	xsi.LogMessage(all)
	for c in cloth:
		#xsi.LogMessage("Update Cloth Object ["+c.Name+"]"+opname+"."+opparam+" ---> "+str(PPG.Inspected(0).Parameters(param).Value))
		syflex = c.ActivePrimitive.ConstructionHistory.Find("syCloth")
		op = syflex
		if opname:
			if not indexed:
				op = syflex.NestedObjects(opname)
				op = xsi.Dictionary.GetObject(op)
			elif not all:
				for n in syflex.NestedObjects:
					if n.Name == opname and n.FullName.find(search)>-1:
						op = xsi.Dictionary.GetObject(n)
			else:
				for n in syflex.NestedObjects:
					if n.Name == opname:
						op = xsi.Dictionary.GetObject(n)
						xsi.LogMessage(op)
						try:
							op.Parameters(opparam).Value = PPG.Inspected(0).Parameters(param).Value
						except:
							xsi.LogMessage("[Cloth_Control] Can't find Parameter "+opparam+" under "+op.FullName+" !",constants.siWarning)
		try:
			op.Parameters(opparam).Value = PPG.Inspected(0).Parameters(param).Value
		except:
			xsi.LogMessage("[Cloth_Control] Can't find Parameter "+opparam+" under "+op.FullName+" !",constants.siWarning)

def Cloth_Control_OutputFolderSelection_OnChanged():
	outputfolder = PPG.OutputFolderSelection.Value
	if outputfolder == 0:
		PPG.OutputFolder.SetCapabilityFlag(constants.siNotInspectable,True)
		PPG.ScenePath.SetCapabilityFlag(constants.siNotInspectable,false)
	
	else:
		PPG.OutputFolder.SetCapabilityFlag(constants.siNotInspectable,False);
		PPG.ScenePath.SetCapabilityFlag(constants.siNotInspectable,True);
			
def Cloth_Control_SelectGravityIcon_OnClicked():
	model = PPG.Inspected(0).Parent3DObject
	icon = model.FindChild("Cloth_Gravity")
	if icon:
		xsi.SelectObj(icon)
		
def Cloth_Control_SelectWindIcon_OnClicked():
	model = PPG.Inspected(0).Parent3DObject
	icon = model.FindChild("Cloth_Wind")
	if icon:
		xsi.SelectObj(icon)
		
		
#----------------------------------------------------
# Cache
#--------------------------------------------
def Cloth_Control_SetSyflexCache_OnClicked():
	scene = xsi.ActiveProject2.ActiveScene
	
	folder = xsi.InstallationPath( constants.siProjectPath )
	scene = xsi.ActiveProject.ActiveScene
	folder = XSIUtils.BuildPath(folder,"Caches",scene)
	
	prop = PPG.Inspected(0)
	cloth = Cloth_Control_GetClothObjects(prop)
	for c in cloth:
		syflex = c.ActivePrimitive.ConstructionHistory.Find( "syCloth" )
		modelfolder = XSIUtils.BuildPath(folder,prop.Parent3DObject.FullName)
		XSIUtils.EnsureFolderExists(modelfolder,False)
		
		path = XSIUtils.BuildPath(modelfolder,c.Name+".")
		syflex.Parameters("Cache").Value = path
		xsi.LogMessage("--------------------------------------------------")
		xsi.LogMessage("[Cloth_Control] Set Syflex Cache Path done for "+c.FullName+" : ")
		xsi.LogMessage(path)
		
def Cloth_Control_ClearSyflexCache_OnClicked():
	scene = xsi.ActiveProject2.ActiveScene
	
	folder = xsi.InstallationPath( constants.siProjectPath )
	scene = xsi.ActiveProject.ActiveScene
	folder = XSIUtils.BuildPath(folder,"Caches",scene)
	
	prop = PPG.Inspected(0)
	cloth = Cloth_Control_GetClothObjects(prop)
	model = cloth[0].Model
	
	# Reset to First Frame
	remote = xsi.Dictionary.GetObject( "PlayControl" )
	v = remote.Parameters("In").Value
	remote.Parameters( "Current" ).Value = prop.Parameters("FirstFrame").Value
	
	#deactivate delta storing state
	previousdelta = uti.DeactivateDelta(model)
	
	for c in cloth:
		syflex = c.ActivePrimitive.ConstructionHistory.Find( "syCloth" )
		#clear all cache.
		xsi.LogMessage("{Cloth_Control] Delete All Syflex Cache File for "+c.FullName)
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
	
def Cloth_Control_SetDampingPercentage_OnClicked():
	prop = PPG.Inspected(0)
	
	tmp = xsi.ActiveSceneRoot.AddProperty("CustomProperty",false, "Cloth_Control_SetDampingPercentage")
	p = tmp.AddParameter3( "Value", constants.siFloat, 100, 1, 1000, False, False )
	layout = tmp.PPGLayout
	layout.Clear()
	layout.AddGroup("Set Damping : Stiffness divided by ")
	layout.AddItem( "Value")
	layout.EndGroup()
	
	bCancelled  = xsi.InspectObj(tmp,None,"Set Syflex Damping",constants.siModal,False)
	if not bCancelled:
		prop.StretchDamping.Value = prop.StretchStiffness.Value / p.Value
		prop.ShearDamping.Value = prop.ShearStiffness.Value / p.Value
		prop.BendDamping.Value = prop.BendStiffness.Value / p.Value
		Cloth_Control_StretchDamping_OnChanged()
		Cloth_Control_ShearDamping_OnChanged()
		Cloth_Control_BendDamping_OnChanged()

		xsi.DeleteObj(tmp)

	else:
		xsi.DeleteObj(tmp)
		xsi.LogMessage("[Cloth_Control]Set Syflex Damping Aborted..")

#----------------------------------------------------
# Corrective Shape Callbacks
#--------------------------------------------
def Cloth_Control_StoreCorrectiveShape_OnClicked():
	prop = PPG.Inspected(0)
	layout = prop.PPGLayout
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes) == 1:
		xsi.Cloth_CorrectiveShape(clothes[0])
		
		smoothlist = Cloth_Control_GetCorrectiveList(prop ,"Shape")
		pro.UpdateListItemsUI( layout.Item("CorrectiveShapeList"), prop.Parameters("CorrectiveShapeList"), smoothlist)
		PPG.Refresh()
		
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)


def Cloth_Control_InspectCorrectiveShape_OnClicked():
	prop = PPG.Inspected(0)
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes) == 1:
		tree = clothes[0].ActivePrimitive.ICETrees("CorrectiveShape")
		if(tree):
			compound = tree.Nodes("CorrectiveShape");
			if(compound):
				xsi.InspectObj(compound)
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)

def Cloth_Control_DeleteCorrectiveShape_OnClicked():
	prop = PPG.Inspected(0)
	layout = prop.PPGLayout
	clothes = Cloth_Control_GetClothObjects(prop)
	
	if len(clothes)==1:
		selectedShape = prop.CorrectiveShapeList.Value;
		selectedFrame = selectedShape.replace("Frame","")
		Cloth_DeleteCorrectiveShape(clothes[0],selectedFrame )
		
		smoothlist = Cloth_Control_GetCorrectiveList(prop,"Shape")
		pro.UpdateListItemsUI( layout.Item("CorrectiveShapeList"), prop.Parameters("CorrectiveShapeList"), smoothlist)
		PPG.Refresh()
		
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)

def Cloth_Control_DeleteAllCorrectiveShape_OnClicked():
	prop = PPG.Inspected(0)
	layout = prop.PPGLayout
	clothes = Cloth_Control_GetClothObjects(prop)
	
	if len(clothes) == 1:
		delete = XSIUIToolkit.Msgbox( "Do you realy want to delete ALL corrective shapes", constants.siMsgYesNo | constants.siMsgQuestion, "Delete Corrective Shape" )

		if delete == constants.siMsgYes:
			tree = clothes[0].ActivePrimitive.ICETrees("CorrectiveShape")
			if tree:
				xsi.DeleteObj(tree)
			cls = clothes[0].ActivePrimitive.Geometry.Clusters("CorrectiveShapeCls")
			if cls:
				xsi.DeleteObj(cls)
		
		shapelist = []
		pro.UpdateListItemsUI( layout.Item("CorrectiveShapeList"), prop.Parameters("CorrectiveShapeList"), shapelist)
		PPG.Refresh()
		
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)

#--------------------------------------------
# Corrective Smooth Callbacks
#--------------------------------------------
def Cloth_Control_StoreCorrectiveSmooth_OnClicked():
	prop = PPG.Inspected(0)
	layout = prop.PPGLayout
	
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes)==1:
		xsi.Cloth_CorrectiveSmooth(clothes[0])
		smoothlist = Cloth_Control_GetCorrectiveList(prop,"Smooth")
		pro.UpdateListItemsUI(layout.Item("CorrectiveSmoothList"), prop.Parameters("CorrectiveSmoothList"), smoothlist)
		PPG.Refresh()
		
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)

def Cloth_Control_InspectCorrectiveSmooth_OnClicked():
	prop = PPG.Inspected(0)
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes)==1:	
		tree = clothes[0].ActivePrimitive.ICETrees("CorrectiveSmooth")
		if(tree):
			compound = tree.Nodes("CorrectiveSmooth")
			if compound:
				xsi.InspectObj(compound)
				
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)

def Cloth_Control_DeleteCorrectiveSmooth_OnClicked():
	prop = PPG.Inspected(0)
	layout = prop.PPGLayout
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes)==1:	
		selectedSmooth = prop.Parameters("CorrectiveSmoothList").Value
		selectedFrame = selectedSmooth.replace("Frame","")
		Cloth_DeleteCorrectiveSmooth(clothes[0],selectedFrame)
		
		smoothlist = Cloth_Control_GetCorrectiveList(prop,"Smooth")
		pro.UpdateListItemsUI( layout.Item("CorrectiveSmoothList"), prop.Parameters("CorrectiveSmoothList"), smoothlist)
		PPG.Refresh()
		
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)

def Cloth_Control_DeleteAllCorrectiveSmooth_OnClicked():
	prop = PPG.Inspected(0)
	layout = prop.PPGLayout
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes)==1:
		toolkit = Dispatch("XSI.UIToolkit")
		delete = toolkit.Msgbox( "Do you realy want to delete ALL corrective smooth maps", constants.siMsgYesNo | constants.siMsgQuestion, "Delete Corrective Smooth" ) ;

		if delete == constants.siMsgYes:
			tree = clothes[0].ActivePrimitive.ICETrees("CorrectiveSmooth")
			if tree:
				xsi.DeleteObj(tree)
			cls = clothes[0].ActivePrimitive.Geometry.Clusters("CorrectiveSmoothCls")
			if cls:
				xsi.DeleteObj(cls)
				
		smoothlist = []
		pro.UpdateListItemsUI( layout.Item("CorrectiveSmoothList"), prop.Parameters("CorrectiveSmoothList"), smoothlist);
		PPG.Refresh()
		
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)

def Cloth_Control_PaintSmooth_OnClicked():
	prop = PPG.Inspected(0)
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes)==1:
		selectedSmooth = prop.CorrectiveSmoothList.value;
		cls = clothes[0].ActivePrimitive.Geometry.Clusters("CorrectiveSmoothCls")
		xsi.SelectObj(cls.Properties(oSelectedSmooth))
		xsi.PaintTool()
		
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)

#--------------------------------------------
# Corrective Push Callbacks
#--------------------------------------------
def Cloth_Control_StoreCorrectivePush_OnClicked():
	prop = PPG.Inspected(0)
	layout = prop.PPGLayout
	
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes)==1:
		xsi.Cloth_CorrectivePush(clothes[0])
		
		pushlist = Cloth_Control_GetCorrectiveList(prop,"Push")
		pro.UpdateListItemsUI( layout.Item("CorrectivePushList"), prop.Parameters("CorrectivePushList"), pushlist)
		PPG.Refresh()
		
	else:
		xsi.LogMessage("[Cloth_Control] Store CorrectiveShape only works in Single item Mode!!", constants.siError)

def Cloth_Control_InspectCorrectivePush_OnClicked():
	prop = PPG.Inspected(0)
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes) == 1:
		tree = clothes[0].ActivePrimitive.ICETrees("CorrectivePush")
		if tree:
			compound = tree.Nodes("CorrectivePush")
			if compound:
				xsi.InspectObj(compound)
				
	else:
		xsi.LogMessage("[Cloth_Control] Inspect Corrective Push only works in Single Item Mode!!", constants.siError)

def Cloth_Control_DeleteCorrectivePush_OnClicked():
	prop = PPG.Inspected(0)
	layout = prop.PPGLayout
	
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes) == 1:
		selectedpush = prop.CorrectivePushList.value
		selectedframe = selectedpush.replace("Frame","")
		Cloth_DeleteCorrectivePush(clothes[0],selectedframe )
		
		pushlist = Cloth_Control_GetCorrectiveList(prop,"Push")
		pro.UpdateListItemsUI( layout.Item("CorrectivePushList"), prop.Parameters("CorrectivePushList"), pushlist)
		PPG.Refresh()
		
	else:
		xsi.LogMessage("[Cloth_Control] Delete Corrective Push only works in Single Item Mode!!", constants.siError)

def Cloth_Control_DeleteAllCorrectivePush_OnClicked():
	prop = PPG.Inspected(0)
	layout = prop.PPGLayout
	clothes = Cloth_Control_GetClothObjects(prop)
	
	if len(clothes) == 1:
		delete = XSIUIToolkit.Msgbox( "Do you realy want to delete ALL corrective push maps", constants.siMsgYesNo | constants.siMsgQuestion, "Delete Corrective Push" ) 

		if delete == constants.siMsgYes:
			tree = clothes[0].ActivePrimitive.ICETrees("CorrectivePush")
			if tree:
				xsi.DeleteObj(tree)
			cls = clothes[0].ActivePrimitive.Geometry.Clusters("CorrectivePushCls")
			if cls:
				xsi.DeleteObj(cls)
		
		pushlist = []
		pro.UpdateListItemsUI( layout.Item("CorrectivePushList"), prop.Parameters("CorrectivePushList"), pushlist);
		PPG.Refresh()
		
	else:
		xsi.LogMessage("[Cloth_Control] Delete ALL Corrective Push only works in Single Item Mode!!", constants.siError)

#--------------------------------------------
# Paint Maps
#--------------------------------------------
def Cloth_Control_PaintPush_OnClicked():
	prop = PPG.Inspected(0)
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes) == 1:
		selectedpush = prop.CorrectivePushList.value
		cls = clothes[0].ActivePrimitive.Geometry.Clusters("CorrectivePushCls")
		xsi.SelectObj(cls.Properties(selectedpush))
		xsi.PaintTool()
	else:
		xsi.LogMessage("[Cloth_Control] Paint Corrective Push only works in Single Item Mode!!", constants.siError)

def Cloth_Control_PaintSoftMimic_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	clothes = Cloth_Control_GetClothObjects(prop)
	
	if len(clothes)==1:
		cls = clothes[0].ActivePrimitive.Geometry.Clusters("SoftMimicCls");
		map = cls.Properties("SoftMimicMap")
		
		Cloth_Control_PaintMap(map)
		
	else:
		xsi.LogMessage("[Cloth_Control] Paint Mimic Map only works in Single Item Mode!!", constants.siError)


def Cloth_Control_PaintRestoreMap_OnClicked():
	prop = PPG.Inspected(0)
	clothes = Cloth_Control_GetClothObjects(prop)
	
	if len(clothes)==1:
		cls = clothes[0].ActivePrimitive.Geometry.Clusters("ClothCls");
		if not cls:
			return
		
		map = cls.Properties("RestoreMap")
		if map:
			Cloth_Control_PaintMap(map)
		else:
			xsi.LogMessage("[Cloth_Control]Can't find RestoreMap on "+clothes[0].FullName)
		
	else:
		xsi.LogMessage("[Cloth_Control] Paint Restore Map only works in Single Item Mode!!", constants.siError)
	
def Cloth_Control_PaintMap(map):
	object = map.Parent3DObject
	object.Properties("Visibility").Parameters("ViewVis").Value = True
	xsi.SelectObj(map)
	view = xsi.OpenView("Object View",True)
	view.SetAttributeValue("lockstatus",True)
	xsi.PaintTool()
	
def Cloth_Control_InspectRestoreNode_OnClicked():
	xsi.LogMessage("hey...")
	prop = PPG.Inspected(0)
	clothes = Cloth_Control_GetClothObjects(prop)
	inspect = []
	for c in clothes:
		tree = c.ActivePrimitive.ICETrees.Find("ClothRestore")
		if tree:
			node = tree.CompoundNodes("ClothRestoreShape")
			if node:
				inspect.append(node)
	xsi.InspectObj(inspect)		

# Get Corrective List
#---------------------------------------------
def Cloth_Control_GetCorrectiveList(prop,correctivetype):
	clothes = Cloth_Control_GetClothObjects(prop)
	if len(clothes) ==1:
		outstr = ""
		outlist = []
		
		proptype = None
		if correctivetype == "Shape":
			proptype = "clskey"
		elif correctivetype == "Smooth":
			proptype = "wtmap"
		elif correctivetype == "Push":
			proptype = "wtmap"
		
		if prop.Parameters("Corrective"+correctivetype+"List"):
			cls = clothes[0].ActivePrimitive.Geometry.Clusters("Corrective"+correctivetype+"Cls");
			if cls:
				props = cls.Properties
				for p in props:
					if p.Type == proptype:
						outstr+=p.Name+";"
						outlist.append(p.Name)
						outlist.append(p.Name)

			prop.Parameters("Corrective"+correctivetype+"List").Value = outstr

		return outlist
		
	else:
		xsi.LogMessage("[Cloth_Control] Get Corrective List only works in Single Item Mode!!", constants.siError)
		
#	Cloth_CorrectivePush
#------------------------------------------------
def Cloth_CorrectivePush_Init(ctxt):
	cmd = ctxt.Source
	cmd.Description = "Create Secondary Animated Push"
	
	args = cmd.Arguments;
	args.Add("obj", constants.siArgumentInput)


def Cloth_CorrectivePush_Execute(obj):
	correct.Correct(obj,"Push")

def Cloth_DeleteCorrectivePush(obj,frame):
	correct.Delete(obj,"Push",frame)
	
#	Cloth_CorrectiveSmooth
#------------------------------------------------	
def Cloth_CorrectiveSmooth_Init(ctxt):
	cmd = ctxt.Source
	cmd.Description = "Create Secondary Animated Smooth"
	
	args = cmd.Arguments;
	args.Add("obj", constants.siArgumentInput)


def Cloth_CorrectiveSmooth_Execute(obj):
	xsi.LogMessage("Cloth_CorrectiveSmooth called...")
	correct.Correct(obj,"Smooth")
	
def Cloth_DeleteCorrectiveSmooth(obj,frame):
	correct.Delete(obj,"Smooth",frame)
	
#	Cloth_CorrectiveShape
#------------------------------------------------
def Cloth_CorrectiveShape_Init(ctxt):
	cmd = ctxt.Source
	cmd.Description = "Create Secondary Shape"
	
	args = cmd.Arguments
	args.Add("inObject", constants.siArgumentInput)
	
def Cloth_CorrectiveShape_Execute(obj):
	xsi.LogMessage("Cloth_CorrectiveShape called...")
	correct.Correct(obj,"Shape")
	
def Cloth_DeleteCorrectiveShape(obj,frame):
	correct.Delete(obj,"Shape",frame)		
	