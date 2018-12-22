#---------------------------------------------------------------
#
#	bGeoCache IO
#
#---------------------------------------------------------------

from win32com.client import constants

null = None
false = 0
true = 1

xsi = Application

def XSILoadPlugin( in_reg ):
	in_reg.Author = "benmalartre"
	in_reg.Name = "bGeoCacheIOPlugin"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterProperty("bImportGeometryPPG")
	in_reg.RegisterProperty("bExportGeometryPPG")
	in_reg.RegisterCommand("bImportGeometry","bbppImportGeometry")
	in_reg.RegisterCommand("bExportGeometry","bbppExportGeometry")
	in_reg.RegisterMenu( constants.siMenuTbAnimateToolsImportID , "bGeoCache", false, false);

	return true

def XSIUnloadPlugin( in_reg ):
	strPluginName = in_reg.Name
	xsi.LogMessage(str(strPluginName) + str(" has been unloaded."),constants.siVerbose)
	return true
	
def bGeoCache_Init(in_ctxt):
	oMenu = in_ctxt.source;
	oMenu.AddCommandItem("&Import Geometry","bImportGeometry")
	oMenu.AddCommandItem("&Export Geometry","bExportGeometry")
	
def bImportGeometry_Execute():
	oRoot = xsi.ActiveSceneRoot
	oUI = oRoot.Properties("bImportGeometry")
	
	if not oUI:
		oUI = oRoot.AddProperty("bImportGeometryPPG",false, "bImportGeometry")

	oPanel = xsi.OpenView("Property Panel",True) 
	oPanel.SetAttributeValue("targetcontent", str(oUI))
	oPanel.Resize(320,200)
	
def bImportGeometryPPG_Define( in_ctxt ):
	oCustomProperty = in_ctxt.Source
	oCustomProperty.AddParameter3("FileName", constants.siString, "", "", "", 0, 0 )
	oCustomProperty.AddParameter3("IsStatic",constants.siBool,0,0,1,0,0)
	oCustomProperty.AddParameter3("Frame",constants.siInt4,1,0,100,0,0)
	
	oCustomProperty.Parameters("Frame").Enable(0)
	return true
	
def bImportGeometryPPG_DefineLayout( in_ctxt ):
	oLayout = in_ctxt.Source
	oLayout.Clear()
	oGrp = oLayout.AddGroup()
	
	oItem = oLayout.AddItem("FileName","Cache File",constants.siControlFilePath)
	oItem.SetAttribute(constants.siUIFileMustExist,1)
	oItem.SetAttribute(constants.siUIFileFilter,"bGeoCache files(*.bgc)|*.bgc|All Files (*.*)|*.*||" )
	
	oEnum = ["Animated",0,"Static",1]
	oLayout.AddEnumControl("IsStatic",oEnum,"Geometry Type",constants.siControlCombo)
	
	oItem = oLayout.AddItem("Frame","Import Frame")

	oLayout.EndGroup()
	
	oItem = oLayout.AddButton("ImportGeometry","Import")
	oItem.SetAttribute(constants.siUICX,300)
	oItem.SetAttribute(constants.siUICY,50)

def bImportGeometryPPG_ImportGeometry_OnClicked():
	oFile = PPG.Inspected(0).Parameters("FileName").Value
	if oFile:
		isstatic = PPG.Inspected(0).Parameters("IsStatic").Value
		if not isstatic:
			obj = xsi.CreatePrim("Cube","MeshSurface","","")
			op = xsi.bbppApplyGeoCache(obj)
			xsi.LogMessage(op)
			op.Parameters("CacheFile").Value = oFile
			
		else:
			frame = PPG.Inspected(0).Parameters("Frame").Value
			xsi.bbppGeoCacheImport(oFile,frame)
	else:
		xsi.LogMessage("Invalid Cache File ---> Import Geometry Aborted...")
	
def bImportGeometryPPG_IsStatic_OnChanged():
	enable = PPG.Inspected(0).Parameters("IsStatic").Value
	PPG.Inspected(0).Parameters("Frame").Enable(enable)
	
def bExportGeometry_Execute():
	oRoot = xsi.ActiveSceneRoot
	oUI = oRoot.Properties("bExportGeometry")
	
	if not oUI:
		oUI = oRoot.AddProperty("bExportGeometryPPG",false, "bExportGeometry")

	oPanel = xsi.OpenView("Property Panel",True) 
	oPanel.SetAttributeValue("targetcontent", str(oUI))
	oPanel.Resize(320,200)
	
def bExportGeometryPPG_Define( in_ctxt ):
	oRemote = xsi.Dictionary.GetObject("PlayControl")
	startFrame = oRemote.Parameters("In").Value
	endFrame = oRemote.Parameters("Out").Value
	oCustomProperty = in_ctxt.Source
	oCustomProperty.AddParameter3("OutputFolder", constants.siString, "", "", "", 0, 0 )
	oCustomProperty.AddParameter3("StartFrame",constants.siInt4,startFrame,-10000,10000,0,0)
	oCustomProperty.AddParameter3("EndFrame",constants.siInt4,endFrame,-10000,10000,0,0)
	
	return true
	
def bExportGeometryPPG_DefineLayout( in_ctxt ):
	oLayout = in_ctxt.Source
	oLayout.Clear()
	oGrp = oLayout.AddGroup()
	
	oItem = oLayout.AddItem("OutputFolder","Output Folder",constants.siControlFolder)

	oLayout.AddRow()
	oItem = oLayout.AddItem("StartFrame")
	oItem.SetAttribute(constants.siUINoSlider,1)
	oItem = oLayout.AddItem("EndFrame")
	oItem.SetAttribute(constants.siUINoSlider,1)
	oLayout.EndRow()
	
	oLayout.EndGroup()
	
	oItem = oLayout.AddButton("ExportGeometry","Export")
	oItem.SetAttribute(constants.siUICX,300)
	oItem.SetAttribute(constants.siUICY,50)

def bExportGeometryPPG_ExportGeometry_OnClicked():
	objs = xsi.Selection
	if (objs.Count == 0):
		xsi.LogMessage("Nothing Selected ---> Export GeoCache Aborted...")
		return
	
	folder = PPG.Inspected(0).Parameters("OutputFolder").Value
	if(folder==""):
		xsi.LogMessage("Invalid Output Folder ---> Export GeoCache Aborted...")
		return
		
	startFrame = PPG.Inspected(0).Parameters("StartFrame").Value
	endFrame = PPG.Inspected(0).Parameters("EndFrame").Value
	
	for o in objs:
		xsi.bbppGeoCacheRecord(o,folder,startFrame,endFrame,1)
	
def bImportGeometryPPG_IsStatic_OnChanged():
	enable = PPG.Inspected(0).Parameters("IsStatic").Value
	PPG.Inspected(0).Parameters("Frame").Enable(enable)


