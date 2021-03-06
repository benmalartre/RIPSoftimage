// ClothIconOpPlugin
//--------------------------------------
function XSILoadPlugin( in_reg )
{
	in_reg.Author = "benmalartre";
	in_reg.Name = "ClothIconOp";
	in_reg.Email = "";
	in_reg.URL = "";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterOperator("SyGravityIconOp");
	in_reg.RegisterCommand("ApplySyGravityIconOp","ApplySyGravityIconOp");
	in_reg.RegisterOperator("SyWindIconOp");
	in_reg.RegisterCommand("ApplySyWindIconOp","ApplySyWindIconOp");
	
	in_reg.RegisterCommand("ConnectGravityIcon","ConnectGravityIcon");
	in_reg.RegisterCommand("ConnectWindIcon","ConnectWindIcon");
	in_reg.RegisterCommand("CreateGravityIcon","CreateGravityIcon");
	in_reg.RegisterCommand("CreateWindIcon","CreateWindIcon");
	in_reg.RegisterCommand("DisconnectGravityIcon","DisconnectGravityIcon");
	in_reg.RegisterCommand("DisconnectWindIcon","DisconnectWindIcon");
	in_reg.RegisterCommand("GravityIcon","GravityIcon");
	in_reg.RegisterCommand("WindIcon","WindIcon");

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	var strPluginName;
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.",siVerbose);
	return true;
}

function ApplySyGravityIconOp_Init( in_ctxt )
{
	var oCmd;
	oCmd = in_ctxt.Source;
	oCmd.Description = "Create an instance of SyGravityIconOp operator";
	oCmd.SetFlag(siNoLogging,false);
	
	var oArgs;
	oArgs = oCmd.Arguments;
	oArgs.Add("inGravityOp",siArgumentInput);
	oArgs.Add("inGravityIcon",siArgumentInput);

	return true;
}

function ApplySyGravityIconOp_Execute(inGravityOp, inGravityIcon)
{
	var newOp = XSIFactory.CreateObject("SyGravityIconOp");
	newOp.AddOutputPort(inGravityOp+".gx");
	newOp.AddOutputPort(inGravityOp+".gy");
	newOp.AddOutputPort(inGravityOp+".gz");
	newOp.AddInputPort(inGravityIcon+".kine.global");
	newOp.Connect();
	return newOp;
}

function SyGravityIconOp_Update( in_ctxt )
{
	var gT = in_ctxt.GetInputValue(0).Transform;
	var gA = gT.SclY;
	var gP = gT.Translation;

	var gF = XSIMath.CreateVector3(0,-1,0);
	gF.MulByTransformationInPlace(gT);
	
	gF.SubInPlace(gP);
	gF.NormalizeInPlace();
	gF.ScaleInPlace(gA*0.17);
	
	if(in_ctxt.OutputPort.name == "OutGx")in_ctxt.OutputPort.Value = gF.x;
	else if(in_ctxt.OutputPort.name == "OutGy")in_ctxt.OutputPort.Value = gF.y;
	else if(in_ctxt.OutputPort.name == "OutGz")in_ctxt.OutputPort.Value = gF.z;
	
	return true;
}

function ApplySyWindIconOp_Init( in_ctxt )
{
	var oCmd;
	oCmd = in_ctxt.Source;
	oCmd.Description = "Create an instance of SyWindIconOp operator";
	oCmd.SetFlag(siNoLogging,false);
	
	var oArgs;
	oArgs = oCmd.Arguments;
	oArgs.Add("inWindOp",siArgumentInput);
	oArgs.Add("inWindIcon",siArgumentInput);

	return true;
}

function ApplySyWindIconOp_Execute(inWindOp, inWindIcon)
{
	var newOp = XSIFactory.CreateObject("SyWindIconOp");
	newOp.AddOutputPort(inWindOp+".MainWindX");
	newOp.AddOutputPort(inWindOp+".MainWindY");
	newOp.AddOutputPort(inWindOp+".MainWindZ");
	newOp.AddOutputPort(inWindOp+".PerturbX");
	newOp.AddOutputPort(inWindOp+".PerturbY");
	newOp.AddOutputPort(inWindOp+".PerturbZ");
	newOp.AddOutputPort(inWindOp+".Intensity");
	newOp.AddInputPort(inWindIcon+".kine.global");
	newOp.Connect();
	return newOp;
}

function SyWindIconOp_Update( in_ctxt )
{
	var wT = in_ctxt.GetInputValue(0).Transform;
	var wA = wT.SclX;
	var wP = wT.Translation;
	var wS = wT.Scaling;

	var wF = XSIMath.CreateVector3(1,0,0);
	wF.MulByTransformationInPlace(wT);
	
	wF.SubInPlace(wP);

	if(in_ctxt.OutputPort.name == "OutMainWindX")in_ctxt.OutputPort.Value = wF.x;
	else if(in_ctxt.OutputPort.name == "OutMainWindY")in_ctxt.OutputPort.Value = wF.y;
	else if(in_ctxt.OutputPort.name == "OutMainWindZ")in_ctxt.OutputPort.Value = wF.z;
	else if(in_ctxt.OutputPort.name == "OutIntensity")in_ctxt.OutputPort.Value = wA/10;
	else if(in_ctxt.OutputPort.Name == "OutPerturbX")in_ctxt.OutputPort.Value = wS.x - 1;
	else if(in_ctxt.OutputPort.Name == "OutPerturbY")in_ctxt.OutputPort.Value = wS.y - 1;
	else if(in_ctxt.OutputPort.Name == "OutPerturbZ")in_ctxt.OutputPort.Value = wS.z - 1;

	return true;
}

// CreateGravityIcon
//------------------------------------------
function CreateGravityIcon_Execute()
{
	var oMesh = Selection(0);
	if(!oMesh || oMesh.type != "polymsh"|| !isCloth(oMesh) )oMesh = null;
	var oGravityOp = getClothOp("syGravity",oMesh);
	
	if(oGravityOp)
	{
		var oGravityIcon = GravityIcon();
		ApplySyGravityIconOp(oGravityOp,oGravityIcon);
	};
}

// ConnectGravityIcon
//------------------------------------------
function ConnectGravityIcon_Execute()
{
	var oMesh = Selection(0);
	if(!oMesh || oMesh.type != "polymsh" || !isCloth(oMesh))oMesh = null;
	var oGravityOp = getClothOp("syGravity",oMesh);
	
	if(oGravityOp)
	{
		var oGravityIcon = PickElement(siCurveFilter,"Pick Gravity Icon").value("PickedElement");
		if(oGravityIcon && oGravityIcon.name.indexOf("GravityIcon")!=-1)
		ApplySyGravityIconOp(oGravityOp,oGravityIcon);
	}
}

// DisconnectGravityIcon
//------------------------------------------
function DisconnectGravityIcon_Execute()
{
	var oMesh = Selection(0);
	if(!oMesh || oMesh.type != "polymsh"|| !isCloth(oMesh))oMesh = null;
	var oGravityOp = getClothOp("syGravity",oMesh);
	
	if(oGravityOp)
	{
		var oIconOp = oGravityOp.gx.source;
		if(oIconOp && oIconOp.Type == "SyGravityIconOp")oIconOp.Disconnect();
		else logmessage("No GravityIcon connected to this operator...");
	}
}

// CreateWindIcon
//------------------------------------------
function CreateWindIcon_Execute()
{
	var oMesh = Selection(0);
	if(!oMesh || oMesh.type != "polymsh"|| !isCloth(oMesh))oMesh = null;
	var oWindOp = getClothOp("syWind",oMesh);
	
	if(oWindOp)
	{
		var oWindIcon = WindIcon();
		ApplySyWindIconOp(oWindOp,oWindIcon);
	}
}

// ConnectWindIcon
//------------------------------------------
function ConnectWindIcon_Execute()
{
	var oMesh = Selection(0);
	if(!oMesh || oMesh.type != "polymsh"|| !isCloth(oMesh))oMesh = null;
	var oWindOp = getClothOp("syWind",oMesh);
	
	if(oWindOp)
	{
		var oWindIcon = PickElement(siCurveFilter,"Pick Wind Icon").value("PickedElement");
		if(oWindIcon && oWindIcon.name.indexOf("WindIcon")!=-1)
		ApplySyWindIconOp(oWindOp,oWindIcon);
	}
}

// DisconnectWindIcon
//------------------------------------------
function DisconnectWindIcon_Execute()
{
	var oMesh = Selection(0);
	if(!oMesh || !isCloth(oMesh)|| oMesh.type != "polymsh")oMesh = null;
	var oWindOp = getClothOp("syWind",oMesh);
	
	if(oWindOp)
	{
		var oIconOp = oWindOp.MainWindX.source;
		if(oIconOp && oIconOp.Type == "SyWindIconOp")oIconOp.Disconnect();
		else logmessage("No WindIcon connected to this operator...");
	}
}

//  isCloth
//---------------------------------------------
function isCloth(inObj)
{
	var oEnum = new Enumerator(inObj.ActivePrimitive.ConstructionHistory);
	for(;!oEnum.atEnd();oEnum.moveNext())
	{
		if(oEnum.item().type == "syCloth") return oEnum.item();
	}
	logmessage("No Cloth Found on "+ inObj);
	return 0;
}

// find Cloth meshs
//---------------------------------------------
function findClothMeshs()
{
	var ClothOps  = FindObjects(null,"{513A555C-13F2-4D77-82E7-C35D14B789C1}");
	var ClothMeshs = XSIFactory.CreateActiveXObject("XSI.Collection");

	for(var i=0; i<ClothOps .Count; i++)
	{
		var ClothMesh = ClothOps(i).OutputPorts(0).Target2.Parent;
		ClothMeshs.Add(ClothMesh);
	}

	return ClothMeshs;
}


//  get Cloth Operator
//---------------------------------------------
function getClothOp(inType,inObj)
{
	XSIUIToolkit.Msgbox( "Select a "+ inType +" operator", siMsgOkOnly, "eClothTools" ) ;
	if(!inObj)var oClothMesh = findClothMeshs();
	else var oClothMesh = inObj
	
	var ClothOp = OpenTransientExplorer( oClothMesh, siSEFilterGeometryOperators, 3 )(0);
	
	if(ClothOp && ClothOp.Type == inType)return ClothOp;
	else return 0;
}

/*--------------------------------
  WindIcon 
--------------------------------*/
function WindIcon_Execute()
{

	var Icon = ActiveSceneRoot.AddNurbsCurveList();
	var iCrv = Icon.ActivePrimitive.Geometry;
	var cCurves,cCrv, cDatas,cPos;

	var pos,knots,closed,degree,parameterization;
	
	pos = Array(0,0,0,1,1.8523366131254672,-0.0010717808411781076,0,1,1.9478207972817132,-0.000231414955498499,0,1,
				2.151371525663248,0.1838788376784602,0,1,1.84689961471569,0.3742691802722048,0,1,1.7870926322081342,0.2110774580489948,0,1);
	knots = Array(0,0,0,1,2,3,3,3);
	closed = false;
	degree = 3;
	parameterization = 1;
	iCrv.AddCurve(pos,knots,closed,degree,parameterization);
	
	pos = Array(0,0.028718769145895106,0,1,1.3892720478125613,0.06506588807366535,0,1,1.595759238588898,0.08937067707248432,0,
				1,1.638271307278143,0.3202661725612567,0,1,1.4500035745114838,0.49647589280268817,0,1,1.179069716157158,
				0.33141718259772573,0,1,1.389560437743813,0.14634775575197617,0,1);
	knots = Array(0,0,0,1,2,3,4,4,4);
	iCrv.AddCurve(pos,knots,closed,degree,parameterization);
	
	pos = Array(0,-0.020910155189063452,0,1,1.5697221881010643,-0.0719366423931406,0,1,1.7583800954805946,-0.06579255467137293,
				0,1,1.8319280100066215,-0.3201272356401506,0,1,1.6128280007664168,-0.4287435046331232,0,1,1.436443629578696,
				-0.35274860746677683,0,1,1.480516396719422,-0.1507659667681489,0,1);
	knots = Array(0,0,0,1,2,3,4,4,4);
	iCrv.AddCurve(pos,knots,closed,degree,parameterization);
	
	Icon.AddProperty("Display Property")
	Icon.properties("display").wirecol.value = 1000;
	
	Icon.name = "WindIcon";
	return Icon;
}

/*--------------------------------
  GravityIcon
--------------------------------*/
function GravityIcon_Execute()
{

	var Icon = ActiveSceneRoot.AddNurbsCurveList();
	var iCrv = Icon.ActivePrimitive.Geometry;
	var cCurves,cCrv, cDatas,cPos;

	var pos,knots,closed,degree,parameterization;
	
	pos = Array(-0.023767532151004324,-1.449265719979165,0,1,-0.05026697185675881,-1.4177790524100937,0,1,-0.06848533665446507,
				-1.3780127732945153,0,1,-0.36736176934207343,-1.3813268638379872,0,1,-0.3830319939339929,-1.6826771995266278,0,1,
				-0.1692826629645792,-2.0259174745433075,0,1,0.0004043234997093973,-2.000773693654818,0,1,0.015485712324803717,
				-1.9880061210902848,0,1,0.03520752848069617,-1.999613005239861,0,1,0.2528203697778078,-2.0408308819889313,0,1,
				0.3555056986376063,-1.5981678906680891,0,1,0.3637867735456547,-1.4277241314465967,0,1,0.1237312909028075,
				-1.3197178711891597,0,1,0.046134667349384865,-1.4446488723220518,0,1,-0.02499557090791944,-1.449110411398437,0,1);
	knots = Array(0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,12,12);
	closed = false;
	degree = 3;
	parameterization = 1;
	iCrv.AddCurve(pos,knots,closed,degree,parameterization);
	
	pos = Array(-0.020800407381699026,-1.4474631626934704,0,1,-0.03182973015521839,-1.3426693432163225,0,1,-0.06159048880753909,
				-1.2671871106980197,0,1,-0.06159048880753909,-1.2671871106980197,0,1,-0.008337497806451306,-1.2499176235771578,0,
				1,-0.008337497806451306,-1.2499176235771578,0,1,-0.0004181573166796055,-1.3332122572724496,0,1,-0.01610349294129609,-1.4478156375818867,0,1);
	knots = Array(0,0,0,1,2,3,4,5,5,5);
	degree = 3;
	iCrv.AddCurve(pos,knots,closed,degree,parameterization);
	
	pos = Array(0,-0.241347920443042,0,1,0,-1.1934850427067373,0,1);
	knots = Array(0,1);
	degree = 1;
	iCrv.AddCurve(pos,knots,closed,degree,parameterization);
	
	pos = Array(0.09734563878688053,0.02510886799627142,0,1,0.09734563878688053,-1.0380519161171377,0,1);
	iCrv.AddCurve(pos,knots,closed,degree,parameterization);
	
	pos = Array(-0.10971008905581111,-0.15252899096327074,0,1,-0.10971008905581111,-1.1490755779668517,0,1);
	iCrv.AddCurve(pos,knots,closed,degree,parameterization);
	
	Icon.AddProperty("Display Property")
	Icon.properties("display").wirecol.value = 62;
	
	Icon.name = "GravityIcon";
	return Icon;
}


