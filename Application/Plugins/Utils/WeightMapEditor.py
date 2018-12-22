# ======================================================================================================================
# WeightMapEditorPlugin
# ======================================================================================================================
from Globals import xsi
from win32com.client import constants
import Utils as uti


def XSILoadPlugin( in_reg ):
	in_reg.Author = "benmalartre"
	in_reg.Name = "WeightMapEditorPlugin"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterProperty("WeightMapEditor")
	in_reg.RegisterMenu(constants.siMenu3DViewComponentContextID, "WeightMapEditorMenu", False)

	return True


def WeightMapEditorMenu_Init(in_ctxt):
	oMenu = in_ctxt.source
	oMenu.AddCallbackItem("WeightMapEditor", "OnWeightMapEditorMenu")


def OnWeightMapEditorMenu(in_ctxt):
	root = xsi.ActiveSceneRoot
	prop = root.Properties("WeightMapEditor")
	if not prop:
		prop = root.AddProperty("WeightMapEditor")
		
	xsi.InspectObj(prop, None, None, constants.siLock)
	obj = xsi.Selection(0).SubComponent.Parent3DObject

	prop.Parameters("TargetObject").Value = obj.FullName
	WeightMapEditor_RebuildLayout(prop)
	

def WeightMapEditor_Define( in_ctxt ):
	oCustomProperty = in_ctxt.Source
	oCustomProperty.AddParameter3("TargetObject", constants.siString, "", "", "", 0, 1)
	oCustomProperty.AddParameter2("WeightMapList", constants.siString, "")
	oCustomProperty.AddParameter2("WeightMapChooser", constants.siInt4, 0, 0, 100, 0, 100, constants.siClassifUnknown, constants.siPersistable)
	oCustomProperty.AddParameter2("Weight", constants.siFloat, 1, -100, 100, -1, 1, constants.siClassifUnknown, constants.siPersistable)
	return True


def WeightMapEditor_OnInit():
	WeightMapEditor_RebuildLayout(PPG.Inspected(0))
	PPG.Refresh()


def WeightMapEditor_OnClosed( ):
	xsi.LogMessage("ELIWeightMapEditor_OnClosed called", constants.siVerbose)


def WeightMapEditor_RebuildLayout(inPPG):
	wmlist = []
	if inPPG.Parameters("TargetObject").Value == "":
		sel = xsi.Selection(0)
		if not sel or not sel.IsClassOf(constants.siX3DObjectID):
			sel = uti.PickElement(constants.siGeometryFilter, "Pick Geometry Object")
		
		if sel:
			inPPG.Parameters("TargetObject").Value = sel.FullName
			wmlist = BuildListWeightMaps(sel, inPPG)
	else:
		obj = xsi.Dictionary.GetObject(inPPG.Parameters("TargetObject").Value)
		wmlist = BuildListWeightMaps(obj, inPPG)
		
	layout = inPPG.PPGLayout
	layout.Clear()
	layout.AddGroup("Target Object")
	layout.AddRow()
	item = layout.AddItem("TargetObject")
	item.SetAttribute(constants.siUINoLabel, True)
	layout.AddButton("ChangeTargetGeometry", "CHANGE")
	layout.EndRow()
	layout.EndGroup()
	
	layout.AddGroup("Weight Maps")
	if len(wmlist) == 0:
		layout.AddStaticText("This object contains no weightmap")
		return
		
	item = layout.AddEnumControl("WeightMapChooser", wmlist, "List", constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel, True)
	#layout.AddItem("WeightMapList")
	layout.EndGroup()
	
	layout.AddGroup("Set Weight On Selected Points")
	layout.AddRow()
	item = layout.AddItem("Weight")
	item.SetAttribute(constants.siUINoLabel, True)
	layout.AddButton("SetWeightOnSelectedPoints", "SET")
	layout.AddButton("AddWeightOnSelectedPoints", "ADD")
	layout.AddButton("SubWeightOnSelectedPoints", "SUB")
	layout.EndRow()
	layout.EndGroup()


def WeightMapEditor_WeightMapChooser_OnChanged():
	sel = xsi.Selection(0)
	wmname = GetUISelectedWeightMap(PPG.Inspected(0))
	wm = xsi.Dictionary.GetObject(wmname)
	xsi.SelectObj(wm)
	xsi.SelectObj(sel)


def WeightMapEditor_ChangeTargetGeometry_OnClicked():
	sel = xsi.Selection(0)
	if not sel or not sel.IsClassOf(constants.siX3DObjectID):
		sel = uti.PickElement(constants.siGeometryFilter, "Pick Geometry Object")
	
	if sel:
		PPG.Inspected(0).Parameters("TargetObject").Value = sel.FullName
		WeightMapEditor_RebuildLayout(PPG.Inspected(0))
		PPG.Refresh()


def WeightMapEditor_SetWeightOnSelectedPoints_OnClicked():
	sel = xsi.Selection(0)
	if not sel.Type == "pntSubComponent":
		xsi.LogMessage("Select some points on Target Object!!", constants.siWarning)
		return
	
	sub = sel.SubComponent
	obj = sub.Parent3DObject

	if not obj.FullName == PPG.Inspected(0).Parameters("TargetObject").Value:
		xsi.LogMessage("Selected Points are not on Target Geometry...", constants.siWarning)
		xsi.SelectObj(obj)
		WeightMapEditor_RebuildLayout(PPG.Inspected(0))
		return
	else:
		wmname = GetUISelectedWeightMap(PPG.Inspected(0))
		xsi.LogMessage(wmname)
		wm = xsi.Dictionary.GetObject(wmname)
		uti.SetWeightOnSelectedPoints(wm, PPG.Weight.Value, sub)


def WeightMapEditor_AddWeightOnSelectedPoints_OnClicked():
	sel = xsi.Selection(0)
	if not sel.Type == "pntSubComponent":
		xsi.LogMessage("Select some points on Target Object!!", constants.siWarning)
		return
	
	sub = sel.SubComponent
	obj = sub.Parent3DObject

	if not obj.FullName == PPG.Inspected(0).Parameters("TargetObject").Value:
		xsi.LogMessage("Selected Points are not on Target Geometry...", constants.siWarning)
		xsi.SelectObj(obj)
		WeightMapEditor_RebuildLayout(PPG.Inspected(0))
		return
	else:
		wmname = GetUISelectedWeightMap(PPG.Inspected(0))
		xsi.LogMessage(wmname)
		wm = xsi.Dictionary.GetObject(wmname)
		uti.AddWeightOnSelectedPoints(wm, PPG.Weight.Value, sub)


def ELIWeightMapEditor_SubWeightOnSelectedPoints_OnClicked():
	sel = xsi.Selection(0)
	if not sel.Type == "pntSubComponent":
		xsi.LogMessage("Select some points on Target Object!!", constants.siWarning)
		return
	
	sub = sel.SubComponent
	obj = sub.Parent3DObject

	if not obj.FullName == PPG.Inspected(0).Parameters("TargetObject").Value:
		xsi.LogMessage("Selected Points are not on Target Geometry...", constants.siWarning)
		xsi.SelectObj(obj)
		WeightMapEditor_RebuildLayout(PPG.Inspected(0))
		return
	else:
		wmname = GetUISelectedWeightMap(PPG.Inspected(0))
		xsi.LogMessage(wmname)
		wm = xsi.Dictionary.GetObject(wmname)
		uti.SubWeightOnSelectedPoints(wm, PPG.Weight.Value, sub)


# ---------------------------------------------------
# Get Selected Weight Map Name
# ---------------------------------------------------
def GetUISelectedWeightMap(inPPG):
	index = inPPG.Parameters("WeightMapChooser").Value
	items = inPPG.Parameters("WeightMapList").Value
	split = items.split("|")
	wm = split[index]
	return wm


# ---------------------------------------------------
# Build UIItems List from String
# ---------------------------------------------------
def BuildListWeightMaps(inObj, inPPG):
	geom = inObj.ActivePrimitive.Geometry
	items = []
	value = ""
	cnt = 0
	# check all cluster for weightmap already exists
	for cls in geom.Clusters.Filter("pnt"):
		for w in cls.LocalProperties.Filter("wtmap"):
			items.append(w.Name)
			value += w.FullName+"|"
			items.append(cnt)
			cnt += 1
	
	value = value[:-1]
	inPPG.Parameters("WeightMapList").Value = value
	return items
