#--------------------------------------------------------
# RigBuilder
#--------------------------------------------------------
from win32com.client import constants
from Globals import XSI
from Globals import XSIFactory
from Globals import Dispatch
from Constants import *
import Constants as CC
reload(CC)
import Utils as uti
reload(uti)
import Element as ele
reload(ele)
import Skeleton as ske
reload(ske)

import Geometry as geo
reload(geo)
import Muscle as msc
reload(msc)

import Icon as ico
reload(ico)
import ICETree as tre
reload(tre)
import Control as ctr
reload(ctr)
import Property as pro
reload(pro)


def XSILoadPlugin(in_reg):
	in_reg.Author = "benmalartre"
	in_reg.Name = "RigBuilder"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterProperty("RigBuilder")
	in_reg.RegisterCommand("GetRigBuilder", "GetRigBuilder")
	
	in_reg.RegisterProperty("RigElement")
	in_reg.RegisterProperty("MuscleElement")
	in_reg.RegisterProperty("GeometryElement")
	in_reg.RegisterProperty("RigElementPainter")
	
	in_reg.RegisterMenu(constants.siMenuTbAnimateCreateCharacterID, "ICERig", True, False)

	return True


def XSIUnloadPlugin(in_reg):
	strPluginName = in_reg.Name
	XSI.LogMessage(str(strPluginName) + str(" has been unloaded."), constants.siVerbose)
	return True


# -----------------------------------------------
# ICERig Menu
# -----------------------------------------------
def ICERig_Init(ctxt):
	oMenu = ctxt.Source
	oMenu.AddCommandItem("Get Rig Builder", "GetRigBuilder")
	return True


# -----------------------------------------------
# Apply RigBuilder
# -----------------------------------------------
def GetRigBuilder_Execute():
	model = XSI.ActiveSceneRoot.AddModel(None, "Rig")

	uti.GroupSetup(model, None, "FK_Rig")
	uti.GroupSetup(model, None, "IK_Rig")
	uti.GroupSetup(model, None, "Spring_Rig")

	prop = model.AddProperty("RigBuilder")
	XSI.InspectObj(prop, None, None, constants.siLock)
	
	guide = uti.AddNull(model, 0,.2, "Guides", 0, 0, 0)
	geo = uti.AddNull(model, 1, .2, "Geometries", 0, 0, 0)
	rig = uti.AddNull(model, 2, .2, "ICE_Rig", 0, 0, 0)
	uti.SetWireColor(rig, 1.0, 0.25, 0.5)
	
	tree = tre.CreateICETree(rig, "Init", constructionhistory=2)
	init = XSI.AddICECompoundNode("IRRigInit", tree)
	XSI.ConnectICENodes(str(tree) + ".port1", str(init) + ".Execute")
	
	simul = tre.CreateSimulatedICETree(rig, "Deform")
	deform = XSI.AddICECompoundNode("IRRigDeform", simul)
	XSI.ConnectICENodes(str(simul) + ".port1", str(deform) + ".Execute")
	
	
# -----------------------------------------------
# RigBuilder Property
# -----------------------------------------------
def RigBuilder_Define(in_ctxt):
	prop = in_ctxt.Source
	prop.AddParameter2("ElementType", constants.siInt4, ID_GEOMETRY, 0, ID_GEOMETRY, 0, ID_GEOMETRY)
	prop.AddParameter2("ElementName", constants.siString, "Hip")
	prop.AddParameter2("Divisions", constants.siInt4, 1, 1, 1000, 1, 12)
	prop.AddParameter2("Side", constants.siInt4, 2, 0, 2, 0, 2)
	prop.AddParameter2("Symmetrize", constants.siBool, False, False, True)
	prop.AddParameter2("SelfSymmetrize", constants.siBool, False, False, True)
	prop.AddParameter2("GlobalSize", constants.siFloat, 1, 0.001, 100)
	prop.AddParameter2("SymmetryAxis", constants.siInt4, 0, 0, 2, 0, 2)
	prop.AddParameter2("Radius", constants.siFloat, 1.0, 0.001, 100.0, 0.001, 100.0)
	prop.AddParameter2("CutoffDistances", constants.siFloat, 1.0, 0.001, 100.0, 0.001, 100.0)
	prop.AddParameter2("GeometryList", constants.siString, "", "", "")
	prop.AddParameter2("GeometryChooser", constants.siString)
	
	prop.AddParameter2("ControlType", constants.siInt4, 0, 0, 2, 0, 2)
	prop.AddParameter2("ControlIconType", constants.siInt4, 0, 0, 24, 0, 24)
	prop.AddParameter2("ControlList", constants.siString, "")
	prop.AddParameter2("ControlChooser", constants.siString)
	
	prop.AddParameter2("SkeletonType", constants.siInt4, 0, 0, 2, 0, 2)
	prop.AddParameter2("SkeletonControlType", constants.siInt4, 0, 0, 6, 0, 6)
	prop.AddParameter2("SkeletonList", constants.siString, "")
	prop.AddParameter2("SkeletonChooser", constants.siString)
	prop.AddParameter2("SkeletonSet", constants.siBool, False, False, True)
	
	prop.AddParameter2("MuscleList", constants.siString, "")
	prop.AddParameter2("MuscleChooser", constants.siString)
	
	
	prop.AddParameter2("R_ColorR_IK", constants.siFloat, 0.5, 0.0, 1.0)
	prop.AddParameter2("R_ColorG_IK", constants.siFloat, 0.0, 0.0, 1.0)
	prop.AddParameter2("R_ColorB_IK", constants.siFloat, 1.0, 0.0, 1.0)
	
	prop.AddParameter2("R_ColorR_FK", constants.siFloat, 0.75, 0.0, 1.0)
	prop.AddParameter2("R_ColorG_FK", constants.siFloat, 0.25, 0.0, 1.0)
	prop.AddParameter2("R_ColorB_FK", constants.siFloat, 1.0, 0.0, 1.0)
	
	prop.AddParameter2("M_ColorR_IK", constants.siFloat, 0.25, 0.0, 1.0)
	prop.AddParameter2("M_ColorG_IK", constants.siFloat, 1.0, 0.0, 1.0)
	prop.AddParameter2("M_ColorB_IK", constants.siFloat, 0.0, 0.0, 1.0)
	
	prop.AddParameter2("M_ColorR_FK", constants.siFloat, 0.75, 0.0, 1.0)
	prop.AddParameter2("M_ColorG_FK", constants.siFloat, 1.0, 0.0, 1.0)
	prop.AddParameter2("M_ColorB_FK", constants.siFloat, 0.25, 0.0, 1.0)
	
	prop.AddParameter2("L_ColorR_IK", constants.siFloat, 1.0, 0.0, 1.0)
	prop.AddParameter2("L_ColorG_IK", constants.siFloat, 0.5, 0.0, 1.0)
	prop.AddParameter2("L_ColorB_IK", constants.siFloat, 0.0, 0.0, 1.0)
	
	prop.AddParameter2("L_ColorR_FK", constants.siFloat, 1.0, 0.0, 1.0)
	prop.AddParameter2("L_ColorG_FK", constants.siFloat, 0.75, 0.0, 1.0)
	prop.AddParameter2("L_ColorB_FK", constants.siFloat, 0.25, 0.0, 1.0)
	
	prop.AddGridParameter("GeometryElements")
	
	#prop.AddParameter2("ShowAffectedPoints",constants.siBool,False,False,True)
	#prop.AddParameter2("ShowAffectedWeights",constants.siBool,False,False,True)
	
	return True

# -----------------------------------------------
# On Init
# -----------------------------------------------
def RigBuilder_OnInit():
	RigBuilder_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


# -----------------------------------------------
# Rebuild Layout
# -----------------------------------------------
def RigBuilder_RebuildLayout( PPG ):
	layout = PPG.PPGLayout
	layout.Clear()

	# globals
	layout.AddTab("Globals")
	
	layout.AddGroup("Geometry To Deform")
	ele.CheckRigElementList(PPG, ID_GEOMETRY)
	items = ele.BuildListFromString(PPG.Parameters("GeometryList").Value)
	item = layout.AddEnumControl("GeometryChooser", items, "", constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel, True)
	
	layout.AddRow()
	layout.AddButton("AddGeometry", "Pick/Add Objects")
	layout.AddButton("RemoveGeometry", "Remove Selected")
	layout.EndRow()
	layout.EndGroup()
	layout.AddGroup("Settings")
	layout.AddItem("GlobalSize", "Size")
	layout.EndGroup()
	layout.AddGroup("Symmetry")
	items = []
	items.append("XY")
	items.append(0)
	items.append("YZ")
	items.append(1)
	items.append("XZ")
	items.append(2)
	item = layout.AddEnumControl("SymmetryAxis", items, "", constants.siControlCombo)
	layout.EndGroup()
	
	layout.AddTab("Colors")
	layout.AddGroup("Color")
	layout.AddGroup("Right")
	layout.AddColor("R_ColorR_IK", "IK")
	layout.AddColor("R_ColorR_FK", "FK")
	layout.EndGroup()
	layout.AddGroup("Middle")
	layout.AddColor("M_ColorR_IK", "IK")
	layout.AddColor("M_ColorR_FK", "FK")
	layout.EndGroup()
	layout.AddGroup("Left")
	layout.AddColor("L_ColorR_IK", "IK")
	layout.AddColor("L_ColorR_FK", "FK")
	layout.EndGroup()
	layout.EndGroup()

	# skeleton
	layout.AddTab("Skeleton")
	ele.CheckRigElementList(PPG, ID_SKELETON)
	#RigBuilder_AddShowSkeletonElement(layout)
	layout.AddGroup("New Element")
	layout.AddItem("ElementName", "Name")
	layout.AddEnumControl("Side", RIG_SIDE_ITEM_ARRAY, "Side", constants.siControlCombo)
	layout.AddEnumControl("SkeletonType", RIG_SKELETON_ITEM_ARRAY, "Type", constants.siControlCombo)
	layout.AddEnumControl("SkeletonControlType", RIG_CONTROLS_ITEM_ARRAY, "Control", constants.siControlCombo)
	layout.AddItem("Divisions", "Division")
	layout.AddItem("Symmetrize", "Symetrize")
	layout.AddItem("SelfSymmetrize", "Self Symetrize")
	item = layout.AddButton("CreateNewElement", "Create")
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 50)
	
	layout.AddGroup("Existing Elements")
	layout.AddGroup()
	
	items = ele.BuildListFromString(PPG.Parameters("SkeletonList").Value)
	item = layout.AddEnumControl("SkeletonChooser", items, "", constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel, True)
	item.SetAttribute(constants.siUIMultiSelectionListBox, True)
	
	layout.EndGroup()
	layout.AddRow()
	item = layout.AddButton("InspectElement", "Inspect")
	item.SetAttribute(constants.siUICX, 95)
	item = layout.AddButton("RemoveElement", "Remove")
	item.SetAttribute(constants.siUICX, 95)
	item = layout.AddButton("SymmetrizeElement", "Symmetrize")
	item.SetAttribute(constants.siUICX, 95)
	layout.EndRow()
	
	layout.EndGroup()
	item = layout.AddButton("IRSetElement", "Set")
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 50)
	'''
	item = layout.AddItem("SkeletonSet","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "Set")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)
	item.SetAttribute(constants.siUIAlignCenter,1)
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,50)
	'''
	layout.EndGroup()
	
	layout.AddGroup("Misc")
	item = layout.AddButton("RebuildSkeletonList", "Rebuild List")
	item.SetAttribute(constants.siUICX, 0)
	layout.EndGroup()

	# muscles
	layout.AddTab("Muscles")
	ele.CheckRigElementList(PPG, ID_MUSCLE)
	layout.AddGroup("New Element")
	layout.AddItem("ElementName", "Name")
	layout.AddEnumControl("Side", RIG_SIDE_ITEM_ARRAY, "Side", constants.siControlCombo)
	layout.AddItem("Divisions", "Division")
	layout.AddItem("Symmetrize")
	item = layout.AddButton("CreateNewElement", "Create")
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 60)
	layout.EndGroup()
	
	layout.AddGroup("Existing Elements")
	layout.AddGroup()

	items = ele.BuildListFromString(PPG.Parameters("MuscleList").Value)
	item = layout.AddEnumControl("MuscleChooser", items, "", constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel, True)
	item.SetAttribute(constants.siUIMultiSelectionListBox, True)
	layout.EndGroup()
	layout.AddRow()
	item = layout.AddButton("InspectElement", "Inspect")
	item.SetAttribute(constants.siUICX, 100)
	item = layout.AddButton("RemoveElement", "Remove")
	item.SetAttribute(constants.siUICX, 100)
	item = layout.AddButton("SymmetrizeMuscle", "Symmetrize")
	item.SetAttribute(constants.siUICX, 100)
	layout.EndRow()
	
	layout.EndGroup()

	item = layout.AddButton("IRSetElement", "Set")
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 50)
	
	layout.AddGroup("Misc")
	item = layout.AddButton("RebuildMuscleList", "Rebuild List")
	item.SetAttribute(constants.siUICX, 0)
	'''
	item = layout.AddButton("MuscleLengthSelected","Muscle Length Selected")
	item.SetAttribute(constants.siUICX,300)
	item = layout.AddButton("MuscleLengthAll","Muscle Length ALL")
	item.SetAttribute(constants.siUICX,300)
	'''
	layout.EndGroup()

	# controls
	layout.AddTab("Controls")
	ele.CheckRigElementList(PPG, ID_CONTROL)
	layout.AddGroup("New Element")
	layout.AddItem("ElementName", "Name")
	layout.AddEnumControl("Side", RIG_SIDE_ITEM_ARRAY, "Side", constants.siControlCombo)
	layout.AddEnumControl("ControlIconType", RIG_CONTROL_ICON_ITEM_ARRAY, "Icon", constants.siControlCombo)
	layout.AddEnumControl("ControlType", RIG_CONTROLS_ITEM_ARRAY, "Type", constants.siControlCombo)

	layout.AddItem("Symmetrize")

	item = layout.AddButton("CreateNewElement", "Create")
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 60)
	layout.EndGroup()
	
	layout.AddGroup("Existing Elements")
	layout.AddGroup()

	items = ele.BuildListFromString(PPG.Parameters("ControlList").Value)
	item = layout.AddEnumControl("ControlChooser", items, "", constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel, True)
	layout.EndGroup()
	layout.AddRow()
	item = layout.AddButton("InspectElement", "Inspect")
	item.SetAttribute(constants.siUICX, 100)
	item = layout.AddButton("RemoveElement", "Remove")
	item.SetAttribute(constants.siUICX, 100)
	item = layout.AddButton("SymmetrizeElement", "Symmetrize")
	item.SetAttribute(constants.siUICX, 100)
	layout.EndRow()
	
	layout.AddRow()
	item = layout.AddButton("SelectControl", "Select")
	item.SetAttribute(constants.siUICX, 100)
	item = layout.AddButton("SetControlStaticKineState", "StaticKineState")
	item.SetAttribute(constants.siUICX, 100)
	item = layout.AddButton("SetControlNeutralPose", "NeutralPose")
	item.SetAttribute(constants.siUICX, 100)
	layout.EndRow()
	
	item = layout.AddButton("UpdateControlColor", "Update Colors")
	item.SetAttribute(constants.siUICX, 300)
	item.SetAttribute(constants.siUICY, 30)
	layout.EndGroup()
	
	layout.AddGroup("Functions")
	layout.AddRow()
	layout.AddButton("BuildIK2BRig", "IK2BoneRig")
	layout.AddButton("BuildIKSpineRig", "IKSpineRig")
	layout.AddButton("BuildFKRig", "FKRig")
	layout.AddButton("BuildFootRollRig", "FootRollRig")
	layout.EndRow()
	layout.EndGroup()
	
	# geometry
	layout.AddTab("Geometry")
	layout.AddRow()
	layout.AddGroup("Geometries")
	
	items = ele.BuildListFromString(PPG.Parameters("GeometryList").Value)
	item = layout.AddEnumControl("GeometryChooser", items, "", constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel, True)
	layout.EndGroup()
	
	layout.AddGroup("Skeleton Elements")
	items = ele.BuildListFromString(PPG.Parameters("SkeletonList").Value)
	item = layout.AddEnumControl("SkeletonChooser", items, "", constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel, True)
	layout.EndGroup()
	layout.EndRow()
	
	layout.AddGroup("Selected Points")
	layout.AddRow()
	item = layout.AddButton("ExclusiveAddPointsToSkeletonElement", "Exclusive Add")
	item.SetAttribute(constants.siUICX, 90)
	item = layout.AddButton("AddPointsToSkeletonElement", "Add")
	item.SetAttribute(constants.siUICX, 70)
	item = layout.AddButton("RemovePointsFromSkeletonElement", "Remove")
	item.SetAttribute(constants.siUICX, 70)
	item = layout.AddButton("MirrorPointsForSkeletonElement", "Mirror")
	item.SetAttribute(constants.siUICX, 70)
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("Geometry Elements")
	item = layout.AddItem("GeometryElements")
	item.SetAttribute(constants.siUINoLabel, True)
	layout.EndGroup()
	
	layout.AddGroup("")
	'''
	item = layout.AddButton("PrepareCopySkin","Prepare Copy Skin")
	item.SetAttribute(constants.siUIAlignCenter,1)
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,40)
	'''
	'''
	item = layout.AddButton("GatherSkinData","Gather Skin Data")
	item.SetAttribute(constants.siUIAlignCenter,1)
	item.SetAttribute(constants.siUICX,300)
	item.SetAttribute(constants.siUICY,40)
	'''
	item = layout.AddButton("RebuildSkeleton", "Rebuild Skeleton")
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("RebuildMuscles", "Rebuild Muscles")
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("AttachSkin", "Attach Skin")
	item.SetAttribute(constants.siUIAlignCenter, 1)
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 25)
	item = layout.AddButton("CreateEnvelope", "Create Envelope")
	item.SetAttribute(constants.siUIAlignCenter, 1)
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 25)
	layout.EndGroup()

'''	
def RigBuilder_AddShowSkeletonElement(PPGLayout):
	PPGLayout.AddGroup("Show Skeleton Elements")
	PPGLayout.AddRow()
	item = PPGLayout.AddItem("ShowGuidePoints","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "Points")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)

	item = PPGLayout.AddItem("ShowGuideShapes","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "Shapes")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)

	item = PPGLayout.AddItem("ShowSkeleton","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "Skeleton")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)

	PPGLayout.EndRow()
	PPGLayout.EndGroup()
'''

'''	
def RigBuilder_AddShowAffectedPoints(PPGLayout):
	PPGLayout.AddGroup("")
	item = PPGLayout.AddItem("ShowAffectedPoints","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "Show Element Points")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)
	
	item = PPGLayout.AddItem("ShowAffectedWeights","","dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "Show Weight Maps")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)
	PPGLayout.EndGroup()
'''


def RigBuilder_AddShowMuscleElement(PPGLayout):
	PPGLayout.AddGroup("Show Muscle Elements")
	PPGLayout.AddRow()
	item = PPGLayout.AddItem("ShowMusclePoints", "", "dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "Points")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)
	
	item = PPGLayout.AddItem("ShowMuscle", "", "dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "Muscles")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)

	item = PPGLayout.AddItem("ShowMuscleAnchors", "", "dscontrol")
	item.SetAttribute("class", "button")
	item.SetAttribute(constants.siUICaption, "Anchors")
	item.SetAttribute(constants.siUIStyle, 0x00001003)
	item.SetAttribute(constants.siUINoLabel, True)

	PPGLayout.EndRow()
	PPGLayout.EndGroup()


def RigBuilder_GeometryElements_GridData(prop):
	oGrid = prop.Parameters("GeometryElements").Value
	oGrid.BeginEdit()
	
	oGrid.RowCount = 6
	oGrid.ColumnCount = 4
	
	oGrid.SetRowValues(0, [0, 0.5, 0.75, 1.0])
	oGrid.SetRowValues(1, [0, 0.5, 0.75, 1.0])
	oGrid.SetRowValues(2, [0, 0.5, 0.75, 1.0])
	oGrid.SetRowValues(3, [0, 0.5, 0.75, 1.0])
	oGrid.SetRowValues(4, [0, 0.5, 0.75, 1.0])
	oGrid.SetRowValues(5, [0, 0.5, 0.75, 1.0])
	
	aCols = ["R","G","B","A"]
	for i in range(0,3):
		oGrid.SetRowLabel(i,"Vertex "+str(i))
		oGrid.SetColumnLabel(i,aCols[i])
		
	oGrid.EndEdit()


def RigBuilder_SkeletonChooser_OnChanged():
	sel = RigBuilder_GetSelectedSkeletonElements(PPG)
	XSI.SelectObj(sel)


def RigBuilder_UpdateElement_OnClicked():
	sel = XSI.Selection
	for s in sel:
		if s.Properties("RigElement"):
			cls = s.ActivePrimitive.Geometry.Clusters("ElementData")
			if cls:
				XSI.DeleteObj(cls)
			
			cls = uti.CreateAlwaysCompleteCluster(s, constants.siVertexCluster, "ElementData")
			cls = Dispatch(cls)
			depth = uti.GetWeightMap(s, "DepthMap", 10, 0, 100, cls)
			width = uti.GetWeightMap(s, "WidthMap", 10, 0, 100, cls)
			offset = cls.AddProperty("SimpleDeformShape",0,"Offset")


def RigBuilder_SymmetrizeElement_OnClicked():
	sel = None
	if XSI.Selection.Count == 0:
		sel = RigBuilder_GetSelectedSkeletonElements(PPG)#xsi.Selection
	else:
		sel = []
		for s in XSI.Selection:
			sel.append(s)
			
	for src in sel:
		prop = src.Properties("RigElement")
		if prop:
			if not prop.Parameters("Symmetrize").Value:
				prop.Parameters("Symmetrize").Value = True
				XSI.LogMessage("[RigBuilder] : Can't Symmetrize Element " + prop.Parameters("ElementName").Value, constants.siWarning)
				#return False
				
			cls = src.ActivePrimitive.Geometry.Clusters("ElementData")
			if not cls:
				continue
			
			#element = ele.IRElement(prop,src.Parent3DObject)
			skeleton = ske.IRSkeleton(src.Properties("RigElement"), src)
			skeleton.GetFromGuide(src)
			skeleton.Symmetrize()
			'''
			symname = element.symfullname
			dst = element.model.FindChild(element.symfullname+"_Crv")
			if not dst or not dst.ActivePrimitive.Geometry.Clusters("ElementData"):
				guide = ske.GetFromGuide(self,src) 
				return False
			
			uti.CopyWeightMap(src,dst,"DepthMap")
			uti.CopyWeightMap(src,dst,"WidthMap")
			'''
			#uti.CopyWeightMap(src,dst,"DepthMap")
			'''
			depth = uti.GetWeightMap(s,"DepthMap",10,0,100,cls)
			width = uti.GetWeightMap(s,"WidthMap",10,0,100,cls)
			offset = cls.AddProperty("SimpleDeformShape",0,"Offset")
			
			sdepth = uti.GetWeightMap(s,"DepthMap",10,0,100,cls)
			'''


def RigBuilder_SymmetrizeMuscle_OnClicked():
	sel = XSI.Selection
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	if sel.Count == 0:
		sel = RigBuilder_GetSelectedSkeletonElements(PPG)#xsi.Selection
		
	for src in sel:
		node = src.ActivePrimitive.ICETrees(0).Nodes(0)
		if node and node.Name == "Muscle Curve Guide":
			m = msc.IRMuscle(None, parent, name, side=MIDDLE, mirror=False, suffix="")
			m.GetFromGuide(src)
			m.Symmetrize()


def RigBuilder_AddGeometry_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	
	elem = geo.IRGeometry(prop, model, XSI.Selection(0).FullName)
	elem.AddToRig()
	PPG.Refresh()


def RigBuilder_RemoveGeometry_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject

	elem = geo.IRGeometry(prop, model)
	elem.RemoveFromRig()
	PPG.Refresh()


def RigBuilder_RebuildSkeletonList_OnClicked():
	prop = PPG.Inspected(0)
	param = prop.Parameters("SkeletonList")

	model = prop.Parent3DObject
	curves = XSIFactory.CreateActiveXObject("XSI.Collection")
	children = model.FindChildren()
	for c in children:
		if c.Properties("RigElement"):
			curves.Add(c)
	
	sList = ""
	for c in curves:
		sList += c.Name.replace("_Crv", "")+"|"
	
	prop.Parameters("SkeletonList").Value = sList
	items = ele.BuildListFromString(sList)
	prop.PPGLayout.Item("SkeletonChooser").UIItems = items 
	
	PPG.Refresh()


def RigBuilder_RebuildMuscleList_OnClicked():
	prop = PPG.Inspected(0)
	param = prop.Parameters("MuscleList")
	
	model = prop.Parent3DObject
	group = model.Groups("Muscle_Curves")
	members = group.Members
	if not members:
		XSI.DeleteOb(group)
	else:
		sList = ""
		for m in members:
			sList += m.Name.replace("_Crv", "")+"|"
		
		prop.Parameters("MuscleList").Value = sList
		items = ele.BuildListFromString(sList)
		prop.PPGLayout.Item("MuscleChooser").UIItems = items 
		PPG.Refresh()


def RigBuilder_MuscleDefoLength(prop,mname):
	model = prop.Parent3DObject
	start = model.FindChild(mname+"_Start")
	end = model.FindChild(mname+"_End")
	dist = uti.DistanceBetweenTwoObjects(start, end)
	XSI.LogMessage(mname + " ---> " + str(dist))
	prop = start.Properties("MuscleControl")
	if prop:
		prop.Parameters("DefoLength").Value = dist


def RigBuilder_MuscleLengthSelected_OnClicked():
	prop = PPG.Inspected(0)
	name = prop.Parameters("MuscleChooser").Value
	RigBuilder_MuscleDefoLength(prop, name)
	
	
def RigBuilder_MuscleLengthAll_OnClicked():
	prop = PPG.Inspected(0)
	RigBuilder_MuscleDefoLength(prop, "")


def RigBuilder_CreateNewElement_OnClicked():
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	root = model.FindChild("Guides")
	if not root:
		uti.MakeRigNull(model, 0, "Guides")
	
	elemType = RigBuilder_GetElementTypeFromTab(PPG)
	elem = RigBuilder_GetElementFromType(prop, elemType, root)
	if elem.PickPosition():
		elem.CreateGuide()
		elem.UpdateBuilder()

		if prop.Parameters("Symmetrize").Value:
			symelem = RigBuilder_GetSymmetrizedElement(elem)
			symelem.UpdateBuilder()
			#symelem.connect(elem)

		PPG.Refresh()


def RigBuilder_IRSetElement_OnClicked():
	sel = XSI.Selection(0)
	if sel:
		model = PPG.Inspected(0).Parent3DObject
		show_env = model.FindChild("ICE_Envelope", constants.siPointCloudPrimType)
		if show_env:
			t = show_env.ActivePrimitive.ICETrees(0)
			n = t.Nodes(0)
			name = sel.Name.replace("_Crv", "")
			XSI.SetValue(str(n) + ".Element_Name_string", name, "")

		XSI.IRSetElement()
	else:
		XSI.LogMessage("[ICERig Set Element] Invalid Selection : Operation aborted!!", constants.siWarning)


def RigBuilder_ShowGuidePoints_OnChanged():
	model = PPG.Inspected(0).Parent3DObject
	grp = model.Groups("Guide_Points")
	if grp:
		grp.Parameters("ViewVis").Value = PPG.ShowGuidePoints.Value


def RigBuilder_ShowGuideShapes_OnChanged():
	model = PPG.Inspected(0).Parent3DObject
	grp = model.Groups("Guide_Shapes")
	if grp:
		grp.Parameters("ViewVis").Value = PPG.ShowGuideShapes.Value


def RigBuilder_ShowSkeleton_OnChanged():
	model = PPG.Inspected(0).Parent3DObject
	skel = model.FindChild("ICE_Skeleton")
	if skel:
		skel.Properties("Visibility").Parameters("ViewVis").Value = PPG.ShowSkeleton.Value
		skel.Properties("Visibility").Parameters("RendVis").Value = PPG.ShowSkeleton.Value


def RigBuilder_ShowMuscle_OnChanged():
	model = PPG.Inspected(0).Parent3DObject
	muscles = model.FindChild("ICE_Muscles")
	if muscles:
		muscles.Properties("Visibility").Parameters("ViewVis").Value = PPG.ShowMuscle.Value


def RigBuilder_ShowMusclePoints_OnChanged():
	model = PPG.Inspected(0).Parent3DObject
	
	musclestart = model.Groups("Muscle_Start")
	if musclestart:
		musclestart.Parameters("ViewVis").Value = PPG.ShowMusclePoints.Value
		
	musclestartdepth = model.Groups("Muscle_StartDepth")
	if musclestartdepth:
		musclestartdepth.Parameters("ViewVis").Value = PPG.ShowMusclePoints.Value
		
	muscleenddepth = model.Groups("Muscle_EndDepth")
	if muscleenddepth:
		muscleenddepth.Parameters("ViewVis").Value = PPG.ShowMusclePoints.Value
		
	muscleend = model.Groups("Muscle_End")
	if muscleend:
		muscleend.Parameters("ViewVis").Value = PPG.ShowMusclePoints.Value


def RigBuilder_ShowMuscleAnchors_OnChanged():
	model = PPG.Inspected(0).Parent3DObject
	muscles = model.FindChild("ICE_Muscles")
	if muscles:
		tree = muscles.ActivePrimitive.ICETrees(0)
		node = tree.Nodes(0)
		node.InputPorts("Display_Anchors").Value = PPG.ShowMuscleAnchors.Value


def RigBuilder_Symmetrize_OnChanged():
	prop = PPG.Inspected(0)
	if prop.Parameters("Symmetrize").Value:
		prop.Parameters("Side").Value = 0
		prop.Parameters("Side").ReadOnly = True
	else:
		prop.Parameters("Side").Value = 2
		prop.Parameters("Side").ReadOnly = False


def RigBuilder_SetControlStaticKineState_OnClicked():
	uti.ResetStaticKinematicState(XSI.Selection)


def RigBuilder_SetNeutralPose_OnClicked():
	#uti.AddStaticKinematicState(xsi.Selection)
	XSI.LogMessage("Not Implemented!!!")


def RigBuilder_UpdateControlColor_OnClicked():
	ctrls = []
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	if XSI.Selection.Count > 0:
		for s in XSI.Selection:
			if s.name.find("_Ctrl")>-1:
				ctrl = ctr.IRControl(prop, model)
				ctrl.GetFromGuide(s)
				ctrls.append(ctrl)
	else:
		children = model.FindChildren("*_Ctrl*")
		for c in children:
			if c.Type == "crvlist":
				ctrl = ctr.IRControl(prop, model)
				ctrl.GetFromGuide(c)
				ctrls.append(ctrl)
				
	for c in ctrls:
		c.SetControlColor()


def RigBuilder_RebuildSkeleton_OnClicked():
	model = PPG.Inspected(0).Parent3DObject
	print model
	model = XSI.ActiveSceneRoot.Models(model.Name)
	print model.FindChildren2("", "", None, True)
	ske.DeleteSkeletonCloud(model)
	if model.Groups("Guide_Curves").Members.Count > 1:
		ele.ReorderCurveGroup(model, ID_SKELETON)
	ske.CreateSkeletonCloud(model)


def RigBuilder_PrepareCopySkin_OnClicked():
	model = PPG.Inspected(0).Parent3DObject
	skin = RigBuilder_GetSelectedSkinGeometry(PPG)
	if skin:
		geo.PrepareCopySkin(model, skin)
	else:
		XSI.LogMessage("[RigBuilder] : Fail Prepare Skin Geometry!")


# ----------------------------------------------
# Update Elements Clusters
# ----------------------------------------------
def RigBuilder_ExclusiveAddPointsToSkeletonElement_OnClicked():
	RigBuilder_UpdateClusterElement(PPG, 0)


def RigBuilder_AddPointsToSkeletonElement_OnClicked():
	RigBuilder_UpdateClusterElement(PPG, 1)


def RigBuilder_RemovePointsFromSkeletonElement_OnClicked():
	RigBuilder_UpdateClusterElement(PPG, 2)


def RigBuilder_UpdateClusterElement(PPG, method):
	# check selection
	sel = XSI.Selection(0)
	if sel and sel.Type == "pntSubComponent":
		sub =sel.SubComponent
		item = PPG.Inspected(0).PPGLayout.Item("SkeletonChooser")
		elemname = item.UIItems[PPG.Inspected(0).Parameters("SkeletonChooser").Value*2]
		clsname = elemname+"_Cls"
		parent = sub.Parent3DObject
		cls = parent.ActivePrimitive.Geometry.Clusters(clsname)
		
		if cls:
			uti.UpdateClusterComponent(cls, sub, method)
			wm = uti.GetWeightMap(parent, "WeightMap", 1, 0, 1, cls)
			elems_tuple = wm.Elements.Array
			elems = [1 for i in range(len(elems_tuple[0])) for j in range(len(elems_tuple))]
			wm.Elements.Array = elems
		else:
			XSI.LogMessage("[RigBuilder] : Can't find cluster named " + clsname + " under geometry " + parent.Name)


def RigBuilder_MirrorPointsForSkeletonElement_OnClicked():
	skin = RigBuilder_GetSelectedSkinGeometry(PPG)
	
	# get selected ui items
	item = PPG.Inspected(0).PPGLayout.Item("SkeletonChooser")
	elemname = item.UIItems[PPG.Inspected(0).Parameters("SkeletonChooser").Value*2]
	clsname = elemname+"_Cls"
	cls = skin.ActivePrimitive.Geometry.Clusters(clsname)
	
	if cls:
		uti.MirrorVertexCluster(cls, True, "WeightMap")


def RigBuilder_GatherSkinData_OnClicked():
	skin = RigBuilder_GetSelectedSkinGeometry(PPG)
	if not skin:
		return False

	geo.GatherSkinDataOnSelf(skin)


def RigBuilder_AttachSkin_OnClicked():
	skin = RigBuilder_GetSelectedSkinGeometry(PPG)
	model = PPG.Inspected(0).Parent3DObject
	if not skin:
		return False
	print model
	print skin
	geo.BindSkinToSkeleton(model, skin)


def RigBuilder_CreateEnvelope_OnClicked():
	skin = RigBuilder_GetSelectedSkinGeometry(PPG)
	model = PPG.Inspected(0).Parent3DObject
	if not skin:
		return False
		
	geo.CreateEnvelopeDuplicate(model, skin)
	
# ---------------------------------------------------
# Get Skin Geometry
# ---------------------------------------------------
def RigBuilder_GetSelectedSkinGeometry(PPG):
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	skinname = getSelectedUIItem(prop, "GeometryChooser")
	XSI.LogMessage(skinname)
	if skinname == None:
		return None

	skin = model.FindChild(skinname)
	if not skin:
		return False
	return skin

# a faire...
def RigBuilder_GetAllSkinGeometry(PPG):
	XSI.LogMessage("[ICERig] GetAllSkinGeometry not implemented...")
	return False
	'''
	model = PPG.Inspected(0).Parent3DObject
	prop = PPG.Inspected(0)
	skinname = getSelectedUIItem(prop,"GeometryChooser")

	skin = model.FindChild(skinname)
	if not skin:
		return False
	return skin
	'''
	
# ---------------------------------------------------
# Get Skeleton Element
#---------------------------------------------------	
def RigBuilder_GetSelectedSkeletonElement(PPG):
	prop = PPG.Inspected(0)

	uiitems = prop.PPGLayout.Item("SkeletonChooser").UIItems
	elemname = uiitems[prop.Parameters("SkeletonChooser").Value*2]
	
	return elemname


def RigBuilder_GetSelectedSkeletonElements(PPG):
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
		
	pvalue = prop.Parameters("SkeletonChooser").Value
	selitems = pvalue.split(";")
	sel = XSIFactory.CreateActiveXObject("XSI.Collection")
	for s in selitems:
		obj = model.FindChild(s+"_Crv")
		if obj:
			sel.Add(obj)

	return sel


# rebuild muscle
def RigBuilder_RebuildMuscles_OnClicked():
	model = PPG.Inspected(0).Parent3DObject
	ele.ReorderCurveGroup(model, ID_MUSCLE)
	msc.CreateMuscleMesh(model, True)


# inspect element
def RigBuilder_InspectElement_OnClicked():
	elems = RigBuilder_GetRigElement(PPG)
	if len(elems) > 0:
		for i in range(len(elems)):
			XSI.LogMessage("Element Name " + elems[i].fullname)
			elems[i].Inspect()


# remove element
def RigBuilder_RemoveElement_OnClicked():
	elems = RigBuilder_GetRigElement(PPG)
	if len(elems) > 0:
		for i in range(len(elems)-1):
			elems[i].RemoveFromRig()


# set element
def RigBuilder_GetRigElement(PPG):
	prop = PPG.Inspected(0)
	model = prop.Parent3DObject
	elemtype = RigBuilder_GetElementTypeFromTab(PPG)
	prefix = RigBuilder_GetElementTypePrefix(elemtype)
	suffix = RigBuilder_GetElementTypeSuffix(elemtype)
	proptype = RigBuilder_GetElementPropType(elemtype)
	
	items = prop.Parameters(prefix+"Chooser").Value
	items = items.split(";")

	elems = []
	
	for item in items:
		obj = model.FindChild(item + suffix)
		if obj:
			prop = obj.Properties(proptype)
			if prop:
				elem = RigBuilder_GetElementFromType(prop, elemtype, obj)
				elems.append(elem)
			else:
				XSI.LogMessage("[RigBuilder] Can't find element property for " + item)
		else:
			XSI.LogMessage("[RigBuilder] Can't find associated xsi object for " + item)
	return elems


# ---------------------------------------------------
def RigBuilder_GetElementFromType(prop, elemtype, parent):
	name = prop.Parameters("ElementName").Value
	division = prop.Parameters("Divisions").Value
	selfsymmetrize = prop.Parameters("SelfSymmetrize").Value
	side = prop.Parameters("Side").Value

	# skeleton element
	if elemtype == ID_SKELETON:
		if prop.Name == PROP_BUILDER:
			mode = prop.Parameters("SkeletonType").Value
			crv = None
		else:
			mode = prop.Parameters("ElementType").Value
			crv = prop.Parent3DObject
		elem = ske.IRSkeleton(crv, parent, name, mode, side, division, selfsymmetrize)

	# muscle element
	elif elemtype == ID_MUSCLE:
		if prop.Name == PROP_BUILDER:
			crv = None
		else:
			crv = prop.Parent3DObject
		elem = msc.IRMuscle(crv, parent, name, side, selfsymmetrize)

	# control element
	elif elemtype == ID_CONTROL:
		if prop.Name == PROP_BUILDER:
			icon = prop.Parameters("ControlIconType").Value
			mode = prop.Parameters("ControlType").Value
			crv = None
		else:
			icon = prop.Parameters("ElementIcon").Value
			mode = prop.Parameters("ElementType").Value
			crv = prop.Parent3DObject

		print "ICON : "+str(icon)
		elem = ctr.IRControl(crv, parent, name, icon, mode, side)

	# geometry element
	elif elemtype == ID_GEOMETRY:
		elem = geo.IRGeometry(prop, parent)
	
	return elem


# ---------------------------------------------------
def RigBuilder_GetSymmetrizedElement(elem):
	parent = elem.GetSymParent(elem.parent)

	if elem.type == ID_SKELETON:
		symelem = elem.Symmetrize()
	elif elem.type == ID_MUSCLE:
		symelem = elem.Symmetrize()
	elif elem.type == ID_CONTROL:
		symelem = elem.Symmetrize()
	elif elem.type == ID_GEOMETRY:
		print "SYMMETRIZE GEOM NOT IMPLEMENTED"
		return None
	return symelem


# ---------------------------------------------------
def RigBuilder_GetElementTypeFromTab(PPG):
	tab = PPG.CurrentTab
	if tab == "Globals":
		return -1
	elif tab == "Skeleton":
		return ID_SKELETON
	elif tab == "Muscles":
		return ID_MUSCLE
	elif tab == "Controls":
		return ID_CONTROL
	elif tab == "Geometry":
		return ID_GEOMETRY


# ---------------------------------------------------
def RigBuilder_GetElementTypePrefix(elemtype):
	if elemtype == ID_SKELETON:
		return "Skeleton"
	elif elemtype == ID_MUSCLE:
		return "Muscle"
	elif elemtype == ID_CONTROL:
		return "Control"
	elif elemtype == ID_GEOMETRY:
		return "Geometry"


# ---------------------------------------------------
def RigBuilder_GetElementTypeSuffix(elemtype):
	if elemtype == ID_SKELETON:
		return SUFFIX_SKELETON
	elif elemtype == ID_MUSCLE:
		return SUFFIX_MUSCLE
	elif elemtype == ID_CONTROL:
		return SUFFIX_CONTROL
	elif elemtype == ID_GEOMETRY:
		return SUFFIX_GEOMETRY


# ---------------------------------------------------
def RigBuilder_GetElementPropType(elemtype):
	if elemtype == ID_SKELETON:
		return PROP_SKELETON
	elif elemtype == ID_MUSCLE:
		return PROP_MUSCLE
	elif elemtype == ID_CONTROL:
		return PROP_CONTROL


# -----------------------------------------------
# RigElementPainter Property
# -----------------------------------------------
def RigElementPainter_Define(in_ctxt):
	prop = in_ctxt.Source
	prop.AddParameter2("ElementList", constants.siString, "", "", "", "", "", constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)

def RigElementPainter_OnInit():
	prop = PPG.Inspected(0)
	RigElementPainter_RebuildLayout(prop)
	PPG.Refresh()
	
def RigElementPainter_RebuildLayout(PPG):
	layout = PPG.PPGLayout
	layout.AddGroup("Elements")

	items = ele.BuildListFromString(PPG.Parameters("ElementList").Value)
	item = layout.AddEnumControl("ElementList",items,"",constants.siControlListBox)	
	item.SetAttribute(constants.siUINoLabel,True)
	'''
	layout.AddRow()
	item = layout.AddButton("InspectMuscleElement","Inspect")
	item.SetAttribute(constants.siUICX,100)
	item = layout.AddButton("SymmetrizeMuscleElement","Symmetrize")
	item.SetAttribute(constants.siUICX,100)
	item = layout.AddButton("RemoveMuscleElement","Remove")
	item.SetAttribute(constants.siUICX,100)
	layout.EndRow()
	'''
	layout.EndGroup()


# -----------------------------------------------
# RigElement Property
# -----------------------------------------------
def RigElement_Define( in_ctxt ):
	prop = in_ctxt.Source
	prop.AddParameter2("ElementType", constants.siInt4, 0, 0, 1000, 0, 100, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("ElementName", constants.siString, "", "", "", "", "")
	prop.AddParameter2("Divisions", constants.siInt4, 3, 1, 1000, 1, 12, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Side", constants.siInt4, 0, 0, 2, 0, 2, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Width", constants.siFloat, .5, 0.01, 100, 0, 2, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Height", constants.siFloat, .5, 0, 100, 0, 2, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Depth", constants.siFloat, .5, 0, 100, 0, 2, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Symmetrize", constants.siBool, False, False, True, False, True)
	prop.AddParameter2("SelfSymmetrize", constants.siBool, False, False, True, False, True)
	prop.AddParameter2("SymmetryAxis", constants.siInt4, 0, 0, 2, 0, 2)
	prop.AddParameter2("Radius", constants.siFloat, 1, 0, 100, -100, 100)
	prop.AddParameter2("Contract", constants.siFloat, 1, -10, 10, -10, 10)
	prop.AddParameter2("Slide", constants.siFloat, 0, -1, 1, -10, 10)
	
	prop.AddParameter2("ControlType", constants.siInt4, 0, 0, 6, 0, 6, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("IKFKBlend", constants.siFloat, 0, 0, 1, 0, 1, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("SpringActive", constants.siBool, True, False, True, False, True, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("SpringBlend", constants.siFloat, 1, 0, 1, 0, 1, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("AnimationMode", constants.siBool, False, False, True, 0, 1, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	
	prop.AddParameter2("Collide", constants.siBool, False, False, True)
	prop.AddParameter2("Stick", constants.siBool, False, False, True)
	prop.AddParameter2("UpVectorMode", constants.siInt4, 0, 0, 2)
	prop.AddParameter2("UpVectorX", constants.siFloat, 0.0)
	prop.AddParameter2("UpVectorY", constants.siFloat, 1.0)
	prop.AddParameter2("UpVectorZ", constants.siFloat, 0.0)
	prop.AddParameter2("UpVectorName", constants.siString, "")
	
	
	return True
	
def RigElement_OnInit():
	RigElement_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()
	
def RigElement_RebuildLayout(ppg):
	layout = ppg.PPGLayout
	layout.Clear()

	layout.AddTab("Element")
	layout.AddGroup("Element")
	
	layout.AddEnumControl("ElementType", RIG_SKELETON_ITEM_ARRAY, "Type", constants.siControlCombo)
	layout.AddItem("ElementName", "Name")

	layout.AddEnumControl("Side", RIG_SIDE_ITEM_ARRAY, "Side", constants.siControlCombo)
	layout.EndGroup()
	
	layout.AddGroup("Datas")
	layout.AddItem("Divisions")
	layout.AddItem("Width")
	layout.AddItem("Depth")
	if ppg.ElementType.Value == 0:
		layout.AddItem("Height")
		
	else:
		item = layout.AddButton("AddProfileControl", "Add Profile Control")
		item.SetAttribute(constants.siUIAlignCenter, 1)
		item.SetAttribute(constants.siUICX, 300)
		
	layout.AddItem("Radius")
	layout.AddItem("Contract")
	layout.AddItem("Slide")
	layout.EndGroup()

	item = layout.AddButton("IRSetElement", "Set")
	item.SetAttribute(constants.siUICX, 0)
	item.SetAttribute(constants.siUICY, 50)
	
	layout.AddTab("Control")
	layout.AddGroup("")
	layout.AddEnumControl("ControlType", RIG_CONTROLS_ITEM_ARRAY, "Type", constants.siControlCombo)
	v = ppg.ControlType.Value
	if v == CTRL_IKFK:
		layout.AddItem("IKFKBlend")
	elif v == CTRL_SPRING:
		layout.AddItem("SpringActive")
		layout.AddItem("SpringBlend")
	layout.EndGroup()
	
	item = layout.AddButton("BuildRigControl", "Build")
	item.SetAttribute("WidthPercentage", 100)

	
	layout.AddTab("Extra")
	layout.AddGroup("Up Vector")
	layout.AddRow()
	layout.AddItem("UpVectorX", "X")
	layout.AddItem("UpVectorY", "Y")
	layout.AddItem("UpVectorZ", "Z")
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("Collide")
	layout.AddItem("Collide")
	layout.AddEnumControl("UpVectorMode", RIG_UPVECTOR_ITEM_ARRAY, "Alignment", constants.siControlCombo)
	layout.AddItem("Stick")
	layout.EndGroup()


def RigElement_IRSetElement_OnClicked():
	crv = PPG.Inspected(0).Parent3DObject
	XSI.SelectObj(crv)
	XSI.IRSetElement()


def  RigElement_ElementName_OnChanged():
	prop = PPG.Inspected(0)
	obj = prop.Parent3DObject
	name = prop.Parameters("ElementName").Value
	prefix = "M_"
	if prop.Parameters("Side").Value == 0:
		prefix = "L_"
	elif prop.Parameters("Side").Value == 1:
		prefix = "R_"
	
	obj.Name = prefix+name+"_Crv"


# -----------------------------------------------
# MuscleControl Property
# -----------------------------------------------
def MuscleElement_Define( in_ctxt ):
	prop = in_ctxt.Source
	prop.AddParameter2("ElementType", constants.siInt4, 0, 0, 1000, 0, 100, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("ElementName", constants.siString, "", "", "", "", "")
	prop.AddParameter2("StartTangent", constants.siFloat, 0.25, 0.001, 1, 0.001, 1, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("EndTangent", constants.siFloat, 0.25, 0.001, 1, 0.001, 1, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("BulgeLimit", constants.siFloat, 2, 0, 4, 0, 4, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("StretchLimit", constants.siFloat, 2, 0, 4, 0, 4, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("DefoLength", constants.siFloat, 4, 0.001, 1000, 0.001, 10, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)

	prop.AddParameter2("SkeletonStart", constants.siInt4, -1, -1, 65535, -1, 65535, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("SkeletonEnd", constants.siFloat, -1, -1, 65535, -1, 65535, constants.siClassifUnknown, constants.siPersistable + constants.siAnimatable + constants.siKeyable)

	prop.AddParameter2("Width", constants.siFloat, 1, 0, 100, 0, 2, constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Height", constants.siFloat, 1, 0, 100, 0, 2, constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("DivisionsU", constants.siInt4,6,1,100,1,100, constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("DivisionsV", constants.siInt4,6,1,100,1,100,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Side", constants.siInt4,0,0,2,0,2,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)

	prop.AddParameter2("Symmetrize", constants.siBool,False,False,True,False,True)
	prop.AddParameter2("SelfSymmetrize", constants.siBool,False,False,True,False,True)
	prop.AddParameter2("SymmetryAxis", constants.siInt4,0,0,2,0,2)
	
	prop.AddParameter2("SpringActive", constants.siBool,True,False,True,False,True,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("SpringBlend", constants.siFloat,1,0,1,0,1,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	
	prop.AddParameter2("Collide", constants.siBool, False, False, True)
	prop.AddParameter2("Stick", constants.siBool, False, False, True)
	prop.AddParameter2("UpVectorMode", constants.siInt4, 0, 0, 2)
	prop.AddParameter2("UpVectorX", constants.siFloat, 0.0)
	prop.AddParameter2("UpVectorY", constants.siFloat, 1.0)
	prop.AddParameter2("UpVectorZ", constants.siFloat, 0.0)
	prop.AddParameter2("UpVectorName", constants.siString, "")
	return True


def MuscleElement_OnInit():
	MuscleElement_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def MuscleElement_RebuildLayout(ppg):
	layout = ppg.PPGLayout
	layout.Clear()

	layout.AddTab("Muscle")
	layout.AddGroup("Muscle")
	layout.AddItem("StartTangent")
	layout.AddItem("EndTangent")
	layout.AddItem("BulgeLimit")
	layout.AddItem("StretchLimit")
	layout.AddItem("DefoLength")
	layout.AddItem("Width")
	layout.AddItem("Height")
	layout.AddItem("DivisionsU")
	layout.AddItem("DivisionsV")
	layout.EndGroup()
	
	layout.AddTab("Element")
	layout.AddGroup("Element")
	
	layout.AddEnumControl("ElementType", RIG_SKELETON_ITEM_ARRAY, "Type", constants.siControlCombo)
	layout.AddItem("ElementName", "Name")

	layout.AddEnumControl("Side", RIG_SIDE_ITEM_ARRAY, "Side", constants.siControlCombo)
	layout.EndGroup()
	
	layout.AddGroup("Datas")
	layout.AddItem("DivisionsU")
	layout.AddItem("DivisionsV")
	
	layout.AddItem("Width")	
	layout.AddItem("Height")
	item = layout.AddButton("AddProfileControl", "Add Profile Control")
	item.SetAttribute(constants.siUIAlignCenter, 1)
	item.SetAttribute(constants.siUICX, 300)
	layout.EndGroup()
	
	layout.AddTab("Extra")
	layout.AddGroup("Up Vector")
	layout.AddRow()
	layout.AddItem("UpVectorX", "X")
	layout.AddItem("UpVectorY", "Y")
	layout.AddItem("UpVectorZ", "Z")
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("Collide")
	layout.AddItem("Collide")
	layout.AddEnumControl("UpVectorMode", RIG_UPVECTOR_ITEM_ARRAY, "Alignment", constants.siControlCombo)
	layout.AddItem("Stick")
	layout.EndGroup()
	return True


def MuscleElement_GetGuideNode():
	parent = PPG.Inspected(0).Parent3DObject
	tree = tre.GetICETreeByName(parent, "Muscle_Guide")
	if tree:
		node = tree.Nodes("Muscle Curve Guide")
		return node
	return None


def MuscleElement_Width_OnChanged():
	node = MuscleElement_GetGuideNode()
	if node:
		port = node.InputPorts("Width_Factor")
		port.Parameters(0).Value = PPG.Inspected(0).Parameters("Width").Value


def MuscleElement_Height_OnChanged():
	node = MuscleElement_GetGuideNode()
	if node:
		port = node.InputPorts("Depth_Factor")
		port.Parameters(0).Value = PPG.Inspected(0).Parameters("Height").Value


def MuscleElement_DivisionsU_OnChanged():
	node = MuscleElement_GetGuideNode()
	if node:
		port = node.InputPorts("U")
		port.Parameters(0).Value = PPG.Inspected(0).Parameters("DivisionsU").Value


def MuscleElement_DivisionsV_OnChanged():
	node = MuscleElement_GetGuideNode()
	if node:
		port = node.InputPorts("V")
		port.Parameters(0).Value = PPG.Inspected(0).Parameters("DivisionsV").Value


def RigElement_BuildRigControl_OnClicked():
	nb = PPG.Inspected.Count
	parent = XSI.Selection(0)
		
	if not parent or parent.Name.find("_Ctrl") == -1:
		XSI.LogMessage("Invalid Selection, Select a controller and try again")
		return
	
	model = parent.model
	ppg = model.Properties("RigBuilder")
	
	for n in range(nb):
		type = PPG.Inspected(n).Parameters("ControlType").Value
		crv = PPG.Inspected(n).Parent3DObject
		elem = crv.Properties("RigElement")
		simple = True
		if elem.Parameters("Divisions").Value>1:
			simple = False

		side = MIDDLE
		if crv.Name[:2] == "L_":
			side = LEFT
		elif crv.Name[:2] == "R_":
			side = RIGHT
		
		if type == CTRL_IK or type == CTRL_IKFK:
			ctr.BuildIK2BonesRig(parent, crv, ppg)
		if type == CTRL_FK or type == CTRL_IKFK:
			ctr.BuildFKRig(parent, crv, ppg, side, simple)
		if type == CTRL_SRT:
			ctr.BuildSRTRig(parent, crv, ppg)
		if type == CTRL_FOOTROLL:
			ctr.BuildFootRollRig(parent, crv, ppg)


def RigControl_OnInit():
	#RigControl_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def RigControl_ControlType_OnChanged():
	#RigControl_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def getSelectedUIItem(prop, itemName):
	return prop.Parameters(itemName).Value

# -----------------------------------------------
# GeometryElement Property
# -----------------------------------------------
def GeometryElement_Define( in_ctxt ):
	prop = in_ctxt.Source
	prop.AddGridParameter("SkeletonElements")
	prop.AddGridParameter("MuscleElements")

	'''
	prop.AddParameter2("ElementType",constants.siInt4,0,0,1000,0,100,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("ElementName",constants.siString,"","","","","",constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Divisions",constants.siInt4,3,1,1000,1,12,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("CompoundName",constants.siString,"","","","","",constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Side",constants.siInt4,0,0,2,0,2,constants.siKeyable|constants.siAnimatable)
	prop.AddParameter2("Width",constants.siFloat,.5,0.01,100,0,2,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Height",constants.siFloat,.5,0,100,0,2,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Depth",constants.siFloat,.5,0,100,0,2,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("Symmetrize",constants.siBool,False,False,True,False,True,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("ControlType",constants.siInt4,0,0,4,0,4,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	prop.AddParameter2("IKFKBlend",constants.siFloat,0,0,1,0,1)
	prop.AddParameter2("SpringActive",constants.siBool,true,false,true,false,True)
	prop.AddParameter2("SpringBlend",constants.siFloat,1,0,1,0,1)
	prop.AddParameter2("AnimationMode",constants.siBool,False,False,True,0,1)
	'''
	return True
	
