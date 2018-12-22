#---------------------------------------------------------------
#
#	bbppTransformation
#
#---------------------------------------------------------------
from Globals import xsi
from win32com.client import constants

null = None
false = 0
true = 1

def XSILoadPlugin( in_reg ):
	in_reg.Author = "benmalartre"
	in_reg.Name = "bbppTransformationPlugin"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterProperty("bbppTransformation")
	in_reg.RegisterCommand("bbppCreateTransformation","bbppCreateTransformation")
	in_reg.RegisterMenu( constants.siMenuMainTopLevelID, "bbpp", false, false);

	return true

def XSIUnloadPlugin( in_reg ):
	strPluginName = in_reg.Name
	xsi.LogMessage(str(strPluginName) + str(" has been unloaded."),constants.siVerbose)
	return true
	
def bbpp_Init(in_ctxt):
	oMenu = in_ctxt.source;
	oMenu.AddCommandItem("&Import Geometry","bImportGeometry")
	oMenu.AddCommandItem("&Export Geometry","bExportGeometry")
	oMenu.AddSeparatorItem()
	oMenu.AddCommandItem("&Create Transformation", "bbppCreateTransformation")
	
def bbppCreateTransformation_Execute():
	oModel = xsi.ActiveSCeneRoot.AddModel()
	oModel.Name = "Transformation"
	oProp = oModel.AddProperty("bbppTransformation")
	xsi.InspectObj(oProp,"","",constants.siLock)
	
	Groups = oModel.Groups
	StartGrp = Groups("Start")
	if not StartGrp:
		StartGrp = oModel.AddGroup()
		StartGrp.Name = "Start"
	EndGrp = Groups("End")
	if not EndGrp:
		EndGrp = oModel.AddGroup()
		EndGrp.Name = "End"

def bbppTransformation_Define( in_ctxt ):
	oCustomProperty = in_ctxt.Source
	oCustomProperty.AddParameter3("AnimatedTransformation",constants.siBool,0,0,1,0,0)
	oCustomProperty.AddParameter3("BarbatrucPicture",constants.siString,"","","")
	oCustomProperty.AddParameter2("StartGeometryStatic",constants.siString,"",null,null,null,null,constants.siClassifUnknown,constants.siPersistable)
	oCustomProperty.AddParameter2("EndGeometryStatic",constants.siString,"",null,null,null,null,constants.siClassifUnknown,constants.siPersistable)
	oCustomProperty.AddParameter2("StartGeometryAnimated",constants.siString,"",null,null,null,null,constants.siClassifUnknown,constants.siPersistable)
	oCustomProperty.AddParameter2("EndGeometryAnimated",constants.siString,"",null,null,null,null,constants.siClassifUnknown,constants.siPersistable)
	oCustomProperty.AddParameter2("StartFrame",constants.siInt4,1,-10000,10000,1,100,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	oCustomProperty.AddParameter2("EndFrame",constants.siInt4,24,-10000,10000,1,100,constants.siClassifUnknown,constants.siPersistable + constants.siAnimatable + constants.siKeyable)
	oCustomProperty.AddParameter3("ColorR",constants.siDouble,1,0,1,0,0)
	oCustomProperty.AddParameter3("ColorG",constants.siDouble,.5,0,1,0,0)
	oCustomProperty.AddParameter3("ColorB",constants.siDouble,1,0,1,0,0)
	oCustomProperty.AddParameter3("ColorA",constants.siDouble,1,0,1,0,0)
	oCustomProperty.AddParameter3("ColorVariance",constants.siDouble,0.5,0,1,0,0)
	return true
	
def bbppTransformation_OnInit():
	bbppTransformation_RebuildLayout( PPG.Inspected(0) )

def bbppTransformation_RebuildLayout( inPPG ):

	oPlugin = xsi.Plugins("bbppTransformationPlugin");
	oPluginPath = oPlugin.OriginPath;
	oTrunkedPath = oPluginPath.split("Application\\");
	oPicturePath = oTrunkedPath[0]+"Data\Pictures\Barbatruc.bmp";
	
	oAnimEnum = ["Static",0,"Animated",1]
	
	oLayout = inPPG.PPGLayout
	oLayout.Clear()
	
	oItem = oLayout.AddEnumControl( "BarbatrucPicture", null,"", constants.siControlBitmap )
	oItem.SetAttribute(constants.siUINoLabel,true);
	oItem.SetAttribute(constants.siUIFilePath,oPicturePath);
	
	oLayout.AddGroup("Transformation Type")
	oItem = oLayout.AddEnumControl("AnimatedTransformation",oAnimEnum,"",constants.siControlCombo)
	oItem.SetAttribute(constants.siUINoLabel,1)
	oLayout.EndGroup()
	
	oLayout.AddGroup("Static Geometry")
	oLayout.AddRow()
	oLayout.AddItem("StartGeometryStatic","Start")
	oLayout.AddButton("PickStartGeometryStatic","Pick")
	oLayout.EndRow()
	
	oLayout.AddRow()
	oLayout.AddItem("EndGeometryStatic","End")
	oLayout.AddButton("PickEndGeometryStatic","Pick")
	oLayout.EndRow()
	oLayout.EndGroup()
	
	if inPPG.Parameters("AnimatedTransformation").Value:
		oLayout.AddGroup("Animated Geometry")
		oLayout.AddRow()
		oLayout.AddItem("StartGeometryAnimated","Start")
		oLayout.AddButton("PickStartGeometryAnimated","Pick")
		oLayout.EndRow()
		
		oLayout.AddRow()
		oLayout.AddItem("EndGeometryAnimated","End")
		oLayout.AddButton("PickEndGeometryAnimated","Pick")
		oLayout.EndRow()
		oLayout.EndGroup()
	
	oLayout.AddGroup("Color")
	oItem = oLayout.AddColor( "ColorR", "",true )
	oItem.SetAttribute( "NoLabel", true )
	oLayout.AddItem("ColorVariance","Variance")
	oLayout.EndGroup()
	
	oLayout.AddGroup("Time Control")
	oLayout.AddItem("StartFrame","Start")
	oLayout.AddItem("EndFrame","End")
	oLayout.EndGroup()

	oLayout.AddGroup("Correspondance Points")
	oLayout.AddRow()
	oItem = oLayout.AddButton("CreateCorrespondancePoints","Create Points")
	oItem.SetAttribute(constants.siUICY,60)
	oItem.SetAttribute(constants.siUICX,150)
	oItem = oLayout.AddButton("DeleteCorrespondancePoints","Delete Points")
	oItem.SetAttribute(constants.siUICY,60)
	oItem.SetAttribute(constants.siUICX,150)
	oLayout.EndRow()
	oLayout.EndGroup()
	
	oLayout.AddGroup("Output Mesh")
	oItem = oLayout.AddButton("BuildOutputMesh","Build")
	oItem.SetAttribute(constants.siUIAlignCenter,1)
	oItem.SetAttribute(constants.siUICX,300)
	oItem.SetAttribute(constants.siUICY,60)
	oLayout.EndGroup()
	
def bbppTransformation_PickStartGeometryStatic_OnClicked( ):
	oStartMesh = bPickElement(constants.siPolyMeshFilter,"Pick Start Geometry")
	PPG.StartGeometryStatic.Value = oStartMesh.FullName
	oMap = bbppTransformation_GetCorrespondanceMap(oStartMesh)
	oGroup = PPG.Inspected(0).Parent3DObject.Groups("Start")
	bbppTransformation_CreateCorrespondanceIceTree(oStartMesh,oGroup,oMap)
	
def bbppTransformation_PickEndGeometryStatic_OnClicked( ):
	oEndMesh = bPickElement(constants.siPolyMeshFilter,"Pick End Geometry")
	PPG.EndGeometryStatic.Value = oEndMesh.FullName
	oMap = bbppTransformation_GetCorrespondanceMap(oEndMesh)
	oGroup = PPG.Inspected(0).Parent3DObject.Groups("End")
	bbppTransformation_CreateCorrespondanceIceTree(oEndMesh,oGroup,oMap)
	
def bbppTransformation_PickStartGeometryAnimated_OnClicked( ):
	oStartMesh = bPickElement(constants.siPolyMeshFilter,"Pick Start Geometry")
	PPG.StartGeometryAnimated.Value = oStartMesh.FullName
	
def bbppTransformation_PickEndGeometryAnimated_OnClicked( ):
	oEndMesh = bPickElement(constants.siPolyMeshFilter,"Pick End Geometry")
	PPG.EndGeometryAnimated.Value = oEndMesh.FullName
	
def bbppTransformation_AnimatedTransformation_OnChanged():
	bbppTransformation_RebuildLayout( PPG.Inspected(0) )
	PPG.Refresh()
			
def bbppTransformation_GetCorrespondanceMap(obj):
	maps = obj.ActivePrimitive.Geometry.VertexColors
	for map in maps:
		if map.Name == "CorrespondanceMap":
			return map	

	# CorrespondanceMap does not exist : we create it
	map = obj.ActivePrimitive.Geometry.AddVertexColor("CorrespondanceMap")
	return map
	
def replaceModelNameByThisModel(model,str):
	str = str.replace(model.Name,"this_model")
	return str

def replaceObjectNameBySelf(obj,str):
	str = str.replace(obj.FullName,"Self")
	return str
	
def bbppTransformation_CreateCorrespondanceIceTree(obj,group,map):
	oModel = PPG.Inspected(0).Parent3DObject
	oIceTree = xsi.ApplyOp("ICETree", obj, "siNode", "", "", 0)
	oCorrespondance = xsi.AddICECompoundNode("bbpp Correspondance Map", oIceTree)
	xsi.ConnectICENodes(str(oIceTree)+".port1", str(oCorrespondance)+".Execute")
	oGet1 = xsi.AddICENode("GetDataNode", oIceTree)
	oGet2 = xsi.AddICENode("GetDataNode", oIceTree)
	#sGroupName = replaceModelNameByThisModel(oModel,group.FullName)
	xsi.SetValue(str(oGet1)+".reference", group.FullName)
	xsi.ConnectICENodes(str(oCorrespondance)+".PointsGroup", str(oGet1)+".outname")
	sMapName = replaceObjectNameBySelf(obj,map.FullName)
	xsi.SetValue(str(oGet2)+".reference", sMapName+".Colors")
	xsi.ConnectICENodes(str(oCorrespondance)+".ColorMap", str(oGet2)+".outname")
	
def bbppTransformation_CreateCorrespondancePoints_OnClicked( ):
	Model = PPG.Inspected(0).Parent3DObject
	Groups = Model.Groups

	StartGrp = Groups("Start")
	if not StartGrp:
		StartGrp = Model.AddGroup()
		StartGrp.Name = "Start"
	EndGrp = Groups("End")
	if not EndGrp:
		EndGrp = Model.AddGroup()
		EndGrp.Name = "End"
		
	oStartRoot = bbppTransformation_GetPointRoot(PPG.Inspected(0),"Start")
	oEndRoot = bbppTransformation_GetPointRoot(PPG.Inspected(0),"End")
	oLinkRoot = bbppTransformation_GetPointRoot(PPG.Inspected(0),"Link")
	
	pid = StartGrp.Members.Count

	# create start null
	StartPos = bPickPosition("Pick Start Position",1)
	Start = bAddNull(oStartRoot,2,0.2,"Start1",1,.2,.5)
	bSetPosition(Start,StartPos)
	StartGrp.AddMember(Start)
	applyColorizePointTree(Model,Start,pid)

	# create end null
	EndPos = bPickPosition("Pick End Position",1)
	End = bAddNull(oEndRoot,2,0.2,"End1",.2,1,.5)
	bSetPosition(End,EndPos)
	EndGrp.AddMember(End)
	applyColorizePointTree(Model,End,pid)
	
	cObj = XSIFactory.CreateActiveXObject("XSI.Collection")
	cObj.Add(Start)
	cObj.Add(End)

	# create linking curve
	oCurve = bBuildCurveOnPoints(cObj,0,0,1)
	oLinkRoot.AddChild(oCurve)
	oCurve.Name = "Link1"
	bSetWireColor(oCurve,.8,.8,1);
	applyColorizePointTree(Model,oCurve,pid)
	
def applyColorizePointTree(model,obj,id):
	oIceTree = xsi.ApplyOp("ICETree", obj, "siNode", "", "", 0)
	oColorize = xsi.AddICECompoundNode("bbpp Colorize Point", oIceTree)
	xsi.ConnectICENodes(str(oIceTree)+".port1", str(oColorize)+".Execute")
	oGet1 = xsi.AddICENode("GetDataNode", oIceTree)
	xsi.SetValue(str(oGet1)+".reference", model.FullName)
	xsi.ConnectICENodes(str(oColorize)+".TransformationModel", str(oGet1)+".outname")
	xsi.SetValue(str(oColorize)+".PointID", id, "")
	
def bbppTransformation_DeleteCorrespondancePoints_OnClicked( ):
	oModel = PPG.Inspected(0).Parent3DObject
	if xsi.Selection.Count == 0:
		oElem = bPickElement(constants.siNullFilter,"Pick Point to Delete")
		
	else:
		oElem = xsi.Selection(0)
		
	if not oElem:
		xsi.LogMessage("bbppTransformation Delete Points : Operation Aborted !",constants.siInfo)
		return
		
	if oElem.Model.FullName == oModel.FullName and oElem.Type == "null":
		if oElem.Name.find("Start") != -1:
			sSplit = oElem.Name.split("Start")
			oOther = oModel.FindChild("End"+sSplit[1])
			oCurve = oModel.FindChild("Link"+sSplit[1])
			
		elif oElem.Name.find("End") != -1:
			sSplit = oElem.Name.split("End")
			oOther = oModel.FindChild("Start"+sSplit[1])
			oCurve = oModel.FindChild("Link"+sSplit[1])
			
		xsi.DeleteObj(str(oElem)+","+str(oOther)+","+str(oCurve))
		
	else:
		xsi.LogMessage("bbppTransformation Delete Points : Invalid Selection ---> Nothing Deleted!",constants.siError)
			
def bbppTransformation_BuildOutputMesh_OnClicked( ):
	oPPG = PPG.Inspected(0)
	oModel = oPPG.Parent3DObject
	oStartCloud = oModel.FindChild("StartCloud")
	oEndCloud = oModel.FindChild("EndCloud")
	
	oStartGrp = oModel.Groups("Start");
	oEndGrp = oModel.Groups("End");
	
	# does geometry exists?
	oStartGeomStatic =  bbppTransformation_StartGeometryStaticExist(oPPG)
	oEndGeomStatic =  bbppTransformation_EndGeometryStaticExist(oPPG)
	
	if not oStartGeomStatic or not oEndGeomStatic:
		xsi.LogMessage("bbppTransformation Geometry Does not exist ---> Can't build Output Mesh!",constants.siError)
		return
	
	if oPPG.AnimatedTransformation.Value:
		oStartGeomAnimated =  bbppTransformation_StartGeometryAnimatedExist(oPPG)
		oEndGeomAnimated =  bbppTransformation_EndGeometryAnimatedExist(oPPG)
		
		if not oStartGeomAnimated or not oEndGeomAnimated:
			xsi.LogMessage("bbppTransformation Geometry Does not exist ---> Can't build Output Mesh!",constants.siError)
			return
	
	# create start geometry point cloud
	if not oStartCloud:
		# create emission node
		oStartCloud = xsi.GetPrim("PointCloud", "StartCloud", oModel)
		oIceTree = xsi.ApplyOp("ICETree", oStartCloud, "siNode", "", "", 0)
		oEmit = xsi.AddICECompoundNode("bbpp Emit From Points", oIceTree)
		xsi.ConnectICENodes(str(oIceTree)+".port1", str(oEmit)+".add")
		oGet1 = xsi.AddICENode("GetDataNode", oIceTree)
		xsi.SetValue(str(oGet1)+".reference", oStartGeomStatic.FullName)
		xsi.ConnectICENodes(str(oEmit)+".Emiter", str(oGet1)+".outname")
		xsi.SetValue(str(oEmit)+".Color_red", 0.9)
		xsi.SetValue(str(oEmit)+".Color_green", 0.4)
		xsi.SetValue(str(oEmit)+".Color_blue", 0.1)
		
		# create blend node
		oBlend = xsi.AddICECompoundNode("bbpp Blend Cloud", oIceTree)
		xsi.AddPortToICENode(str(oIceTree)+".port1", "siNodePortDataInsertionLocationAfter")
		xsi.ConnectICENodes(str(oIceTree)+".port2", str(oBlend)+".Execute")
		oGet2 = xsi.AddICENode("GetDataNode",oIceTree)
		xsi.SetValue(str(oGet2)+".reference", str(oStartGrp))
		xsi.ConnectICENodes(str(oBlend)+".Start_Cloud", str(oGet2)+".outname")
		oGet3 = xsi.AddICENode("GetDataNode",oIceTree)
		xsi.SetValue(str(oGet3)+".reference", str(oEndGrp))
		xsi.ConnectICENodes(str(oBlend)+".End_Cloud", str(oGet3)+".outname")
		oGet4 = xsi.AddICENode("GetDataNode",oIceTree)
		xsi.SetValue(str(oGet4)+".reference", oEndGeomStatic.FullName)
		xsi.ConnectICENodes(str(oBlend)+".Target_Geometry", str(oGet4)+".value")
		
		# connect timer
		oTimer = xsi.AddICECompoundNode("bbpp Transformation Timer", oIceTree)
		xsi.ConnectICENodes(str(oBlend)+".Blend", str(oTimer)+".Blend")
		oGet5 = xsi.AddICENode("GetDataNode", oIceTree)
		xsi.SetValue(str(oGet5)+".reference", oPPG.StartFrame.FullName)
		oGet6 = xsi.AddICENode("GetDataNode", oIceTree)
		xsi.SetValue(str(oGet6)+".reference", oPPG.EndFrame.FullName)
		xsi.ConnectICENodes(str(oTimer)+".Start_Frame", str(oGet5)+".value")
		xsi.ConnectICENodes(str(oTimer)+".End_Frame", str(oGet6)+".value")

	# create end geometry point cloud
	if not oEndCloud:
		# create emission node
		oEndCloud = xsi.GetPrim("PointCloud", "EndCloud", oModel)
		oIceTree = xsi.ApplyOp("ICETree", oEndCloud, "siNode", "", "", 0)
		oEmit = xsi.AddICECompoundNode("bbpp Emit From Points", oIceTree)
		xsi.ConnectICENodes(str(oIceTree)+".port1", str(oEmit)+".add")
		oGet1 = xsi.AddICENode("GetDataNode", oIceTree)
		xsi.SetValue(str(oGet1)+".reference", oEndGeomStatic.FullName)
		xsi.ConnectICENodes(str(oEmit)+".Emiter", str(oGet1)+".outname")
		xsi.SetValue(str(oEmit)+".Color_red", 0.1)
		xsi.SetValue(str(oEmit)+".Color_green", 0.9)
		xsi.SetValue(str(oEmit)+".Color_blue", 0.4)
		
		# create blend node
		oBlend = xsi.AddICECompoundNode("bbpp Blend Cloud", oIceTree)
		xsi.AddPortToICENode(str(oIceTree)+".port1", "siNodePortDataInsertionLocationAfter")
		xsi.ConnectICENodes(str(oIceTree)+".port2", str(oBlend)+".Execute")
		oGet2 = xsi.AddICENode("GetDataNode",oIceTree)
		xsi.SetValue(str(oGet2)+".reference", str(oEndGrp))
		xsi.ConnectICENodes(str(oBlend)+".Start_Cloud", str(oGet2)+".outname")
		oGet3 = xsi.AddICENode("GetDataNode",oIceTree)
		xsi.SetValue(str(oGet3)+".reference", str(oStartGrp))
		xsi.ConnectICENodes(str(oBlend)+".End_Cloud", str(oGet3)+".outname")
		oGet4 = xsi.AddICENode("GetDataNode",oIceTree)
		xsi.SetValue(str(oGet4)+".reference", oStartGeomStatic.FullName)
		xsi.ConnectICENodes(str(oBlend)+".Target_Geometry", str(oGet4)+".value")
		
		# connect timer
		oTimer = xsi.AddICECompoundNode("bbpp Transformation Timer", oIceTree)
		xsi.ConnectICENodes(str(oBlend)+".Blend", str(oTimer)+".Blend")
		oGet5 = xsi.AddICENode("GetDataNode", oIceTree)
		xsi.SetValue(str(oGet5)+".reference", oPPG.StartFrame.FullName)
		oGet6 = xsi.AddICENode("GetDataNode", oIceTree)
		xsi.SetValue(str(oGet6)+".reference", oPPG.EndFrame.FullName)
		xsi.ConnectICENodes(str(oTimer)+".Start_Frame", str(oGet5)+".value")
		xsi.ConnectICENodes(str(oTimer)+".End_Frame", str(oGet6)+".value")
		xsi.SetValue(str(oTimer)+".Revert", True)
		
	# create Blob Mesh
	# delete it if already exists
	bbppTransformation_BlobGeometryExist(oPPG)
	xsi.SelectObj(str(oStartCloud)+","+str(oEndCloud))
	xsi.Create_Polygonizer_Polymesh()
	oBlob = xsi.Selection(0)
	oBlob.Name = "OutputMesh"
	oModel.AddChild(oBlob)
	oBlobOp = oBlob.ActivePrimitive.ConstructionHistory.Find("Polygonizer")
	
	
	# create expression for in-between
	#--------------------------------------
	# detail
	sStartFrame = oPPG.StartFrame.FullName
	sEndFrame = oPPG.EndFrame.FullName
	sTimelapse = sEndFrame+" - "+sStartFrame
	oExpr = "(1 - sin(cond(FC>"+sStartFrame+",cond(Fc<"+sEndFrame+",(Fc - "+sStartFrame+")/("+sTimelapse+"),1),0)*180)) *2 + 2"
	oBlobOp.Parameters("Detail").AddExpression(oExpr)
	
	# blur isofield
	oExpr = "sin(cond(FC>"+sStartFrame+",cond(Fc<"+sEndFrame+",(Fc - "+sStartFrame+")/("+sTimelapse+"),1),0)*180)/2"
	oBlobOp.Parameters("BlurIsofieldStrength").AddExpression(oExpr)
	
	# smooth mesh
	oExpr = "sin(cond(FC>"+sStartFrame+",cond(Fc<"+sEndFrame+",(Fc - "+sStartFrame+")/("+sTimelapse+"),1),0)*180)*4+2"
	oBlobOp.Parameters("SmoothMeshStrength").AddExpression(oExpr)
	
	# isolevel
	oBlobOp.Parameters("Isolevel").Value = 1
	
def bbppTransformation_StartGeometryStaticExist(inPPG):

		outObj = xsi.Dictionary.GetObject(inPPG.StartGeometryStatic.Value,false)
		if not outObj:
			inPPG.StartGeometryStatic.Value = ""
			return
		
		return outObj
	
def bbppTransformation_EndGeometryStaticExist(inPPG):
	outObj = xsi.Dictionary.GetObject(inPPG.EndGeometryStatic.Value,false)
	if not outObj:
		inPPG.EndGeometryStatic.Value = ""
		return
		
	return outObj
	
def bbppTransformation_StartGeometryAnimatedExist(inPPG):

		outObj = xsi.Dictionary.GetObject(inPPG.StartGeometryAnimated.Value,false)
		if not outObj:
			inPPG.StartGeometryAnimated.Value = ""
			return
		
		return outObj
	
def bbppTransformation_EndGeometryAnimatedExist(inPPG):
	outObj = xsi.Dictionary.GetObject(inPPG.EndGeometryAnimated.Value,false)
	if not outObj:
		inPPG.EndGeometryAnimated.Value = ""
		return
		
	return outObj
	
def bbppTransformation_BlobGeometryExist(inPPG):
	oOutputMesh = inPPG.Parent3DObject.FindChild("OutputMesh")
	if oOutputMesh:
		xsi.DeleteObj(oOutputMesh)

def  bbppTransformation_GetPointRoot(inPPG,inName):
	oModel = inPPG.Parent3DObject
	oRoot = oModel.FindChild(inName+"Root",constants.siNullPrimType)
	if not oRoot :
		oRoot = bAddNull(oModel,1,0.1,inName+"Root",0,0,0)
		if not inName == "Link":
			oGeom = xsi.ActiveSceneRoot.FindChild(inPPG.Parameters(inName+"GeometryStatic").Value)
			if oGeom:
				oRoot.Kinematics.AddConstraint("Pose",oGeom)
			
	return oRoot

