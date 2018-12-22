// MirrorAnim
//--------------------------------------------------
function XSILoadPlugin( in_reg )
{
	in_reg.Author = "benmalartre";
	in_reg.Name = "MirrorAnimPlugin";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterCommand("MirrorAnim","MirrorAnim");

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.");
	return true;
}

function MirrorAnim_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "mirror of pose or animation";
	oCmd.ReturnValue = true;

	var oArgs;
	oArgs = oCmd.Arguments;
	oArgs.Add("inActionType",siArgumentInput);
	oArgs.Add("inSelectType",siArgumentInput);	
	oArgs.Add("inOffset",siArgumentInput);
	oArgs.Add("inConsiderTime",siArgumentInput);
	oArgs.Add("inFrameIn",siArgumentInput);
	oArgs.Add("inFrameOut",siArgumentInput);
	return true;
}

function MirrorAnim_Execute(inActionType, inSelectType, inOffset, inConsiderTime, inFrameIn, inFrameOut)
{
	if (Selection.count == 0){logmessage ("select at least one controller!!");return;}

	mirror = new Object();
	mirror.sel = Selection;
	if(mirror.sel(0).type == "#model")mirror.model = mirror.sel(0);
	else mirror.model = mirror.sel(0).model;
	
	mirror.globalSRT = mirror.model.FindChild("GlobalSRT");
	
	// getUI
	if (!inActionType){mirror = getUI(mirror);}
	else 
	{
		mirror.actionType 		= inActionType;
		mirror.selectType 		= inSelectType;
		mirror.offset		 	= inOffset;	
		mirror.considerTime		= inConsiderTime;
		mirror.frameIn		 	= inFrameIn;	
		mirror.frameOut		 	= inFrameOut;
	}
	
	// ==== ACTIONS =======================================
	
	if (mirror.actionType != null)
	{
		oRun = 1;
		mirror = getControlers(mirror, mirror.selectType);
		mirror = getDisplayInfo(mirror);
		mirror = getParameters(mirror);
		if (mirror.actionType == 4){mirror = deleteMap(mirror);oRun = 0;}
		mirror = createMirrorCnxMap(mirror);
		mirror = createValueCnxMap(mirror);
		
		if(oRun == 1)
		{
			mirror = getControlers(mirror, mirror.selectType);
			if (mirror.actionType == 1){mirror = mirrorPose(mirror,0);}
			else if (mirror.actionType == 2){mirror = mirrorPose(mirror,1);}
			else if (mirror.actionType == 3){mirror = mirrorAnim(mirror);}
		}
		
		mirror = muteMirror(mirror);	
	}
	
	logmessage ("eMirrorAnim complete");
}


// deleteMap
// This function delete the templates connexion mapping between Right and Left
//-------------------------------------------------------------------------------
function deleteMap(inMirror)
{
	if(!inMirror.model.mixer)inMirror.model.addMixer();
	inMirror.cnxMap = inMirror.model.Mixer.NestedObjects("Templates").NestedObjects("Connection Mapping").NestedObjects("MirrorCnxMap")	
	if (inMirror.cnxMap)
	{DeleteObj(inMirror.cnxMap);}
	
	inMirror.invValMap = inMirror.model.Mixer.NestedObjects("Templates").NestedObjects("Value Mapping").NestedObjects("InverseValMap");
	if (inMirror.invValMap)
	{DeleteObj(inMirror.invValMap);}
	
	return inMirror;
}

// createMirrorCnxMap
// This function create the templates connexion mapping between Right and Left
//-------------------------------------------------------------------------------
function createMirrorCnxMap(inMirror)
{
	inMirror.cnxMap = inMirror.model.Mixer.NestedObjects("Templates").NestedObjects("Connection Mapping").NestedObjects("MirrorCnxMap")

	if (!inMirror.cnxMap)
	{
		inMirror.objects = XSIFactory.CreateActiveXObject("XSI.Collection");
		var oGrp = inMirror.model.Groups.Item("Controls");
		if(oGrp)inMirror.objects.AddItems(oGrp.members);
		
		inMirror.cnxMap = CreateEmptyConnectionMap(inMirror.model)(0);
		inMirror.cnxMap.Name = "MirrorCnxMap";

		for (i=0;i<inMirror.objects.count;i++)
		{
			oldItemName = inMirror.objects(i).name;
			LogMessage(oldItemName);
			var s1 = oldItemName.charAt(1);
			var oldItemLow = oldItemName.toLowerCase();
			var s2 = oldItemLow.charAt(1);
			if(oldItemName.substring(0,1) == "L" && (s1 != s2  || s1 == "_"))newItemName = oldItemName.replace("L", "R");
			else if(oldItemName.substring(0,1) == "R" && (s1 != s2 || s1 == "_"))newItemName = oldItemName.replace("R", "L");
			else if(oldItemName.indexOf("_L_")!=-1)newItemName = oldItemName.replace("_L_", "_R_");
			else if(oldItemName.indexOf("_R_")!=-1)newItemName = oldItemName.replace("_R_", "_L_");
			else newItemName = oldItemName;

			AddMappingRule (inMirror.cnxMap, oldItemName, newItemName, GetNumMappingRules(inMirror.cnxMap)+1);
		}
		
		for(i=0;i<inMirror.DisplayInfos.count;i++)
		{
			var targetParams = inMirror.DisplayInfos(i).Parameters;
			var sourceInfoName;
			var side;
			if( inMirror.DisplayInfos(i).name.indexOf("_L")!=-1 )
			{
				sourceInfoName = inMirror.DisplayInfos(i).FullName.replace("_L","_R");
				side = -1;
			}
			else if( inMirror.DisplayInfos(i).name.indexOf("_R")!=-1)
			{
				sourceInfoName = inMirror.DisplayInfos(i).FullName.replace("_R","_L");
				side = 1;
			}
			else 
			{
				sourceInfoName = inMirror.DisplayInfos(i).FullName;
				side = 0;
			}
			
			var sourceInfos = Dictionary.GetObject(sourceInfoName);
			if(sourceInfos)var sourceParams = sourceInfos.Parameters;
			
			
			if(sourceParams)
			{
				for(a=0;a<targetParams.count;a++)
				{
					var oldItemParam = targetParams(a).fullName;
					oldItemParam = oldItemParam.replace(inMirror.model.name+".","");
					if(side == -1)var newItemParam = oldItemParam.replace("Left","Right");
					else if(side == 1)var newItemParam = oldItemParam.replace("Right","Left")
					else
					{
						if(oldItemParam.indexOf("Right")!=-1)var newItemParam = oldItemParam.replace("Right","Left");
						else if(oldItemParam.indexOf("Left")!=-1)var newItemParam = oldItemParam.replace("Left","Right");
					}

					for(b=0;b<sourceParams.count;b++)
					{
						if(sourceParams(b).fullName.indexOf(newItemParam)!=-1)
						AddMappingRule (inMirror.cnxMap, oldItemParam, newItemParam, GetNumMappingRules(inMirror.cnxMap)+1);
					}
				}
			}
		}		
		logmessage ("Mirror Connexion Mapping has been generated. You should check it");
	}
	
	inMirror.board = new Array();
	inMirror.selfObject = 0;
	LogMessage("Num Mapping Rules : "+GetNumMappingRules(inMirror.cnxMap))
	for (i=0;i<GetNumMappingRules(inMirror.cnxMap);i++)
	{		
		inMirror.board[i] = new Array(GetMappingRule(inMirror.cnxMap, i+1)(1), GetMappingRule(inMirror.cnxMap, i+1)(3));
		//logmessage (inMirror.board[i][0]+" >>> "+inMirror.board[i][1]);
		
		if (inMirror.board[i][0] == inMirror.board[i][1])
		{inMirror.selfObject = inMirror.selfObject+1;}
	}
	return inMirror;

}

// Create Value Connexion Template
//----------------------------------------------------
function createValueCnxMap(inMirror)
{
	inMirror.invValMap = inMirror.model.Mixer.NestedObjects("Templates").NestedObjects("Value Mapping").NestedObjects("InverseValMap");
	
	if (!inMirror.invValMap)
	{
		inMirror.invValMap = CreateEmptyValueMap(inMirror.model)(0);
		inMirror.invValMap.Name = "InverseValMap";

		// ==== NAMES OF THE CONTROLERS & INVERSED PARAMETERS ===
		// The Array are [Controler Name, Prefix Length, Parameter1, Parameter2 ....]
		inMirror.ctrl = new Array();
		var id = 0;
		//Spine
		if(inMirror.model.findChild("UpperBody")){inMirror.ctrl[id] = new Array("UpperBody", 0, "posx", "roty", "rotz");id++;}
		if(inMirror.model.findChild("Hip")){inMirror.ctrl[id] = new Array("Hip", 0, "posx", "roty", "rotz");id++;}
		if(inMirror.model.findChild("Chest")){inMirror.ctrl[id] = new Array("Chest", 0, "posx", "roty", "rotz");id++;}
		if(inMirror.model.findChild("Spine1_FK")){inMirror.ctrl[id] = new Array("Spine1_FK", 0, "rotx", "roty");id++;}
		if(inMirror.model.findChild("Spine2_FK")){inMirror.ctrl[id] = new Array("Spine2_FK", 0, "rotx", "roty");id++;}
		
		//Shoulders
		if(inMirror.model.findChild("L_Shoulder")){inMirror.ctrl[id] = new Array("L_Shoulder", 0, "rotx","roty");id++;}
		if(inMirror.model.findChild("R_Shoulder")){inMirror.ctrl[id] = new Array("R_Shoulder", 0, "rotx",  "roty");id++;}

		//Head
		if(inMirror.model.findChild("Neck")){inMirror.ctrl[id] = new Array("Neck", 0, "rotx", "roty");id++;}
		if(inMirror.model.findChild("Head")){inMirror.ctrl[id] = new Array("Head", 0, "rotx", "roty");id++;}
	
		//Left Arm
		if(inMirror.model.findChild("L_Bicep")){inMirror.ctrl[id] = new Array("L_Bicep", 1, "rotx","roty");id++;}
		if(inMirror.model.findChild("L_Hand")){inMirror.ctrl[id] = new Array("L_Hand", 1, "rotx","roty");id++;}
		if(inMirror.model.findChild("L_Hand_IK")){inMirror.ctrl[id] = new Array("L_Hand_IK", 1, "posx","rotx","roty","sclx","scly","sclz");id++;}
		
		//Left Hand
		if(inMirror.model.findChild("L_Thumb_1")){inMirror.ctrl[id] = new Array("L_Thumb_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Thumb_2")){inMirror.ctrl[id] = new Array("L_Thumb_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Thumb_3")){inMirror.ctrl[id] = new Array("L_Thumb_3", 1,"rotx","roty");id++;}
		
		if(inMirror.model.findChild("L_Index_1")){inMirror.ctrl[id] = new Array("L_Index_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Index_2")){inMirror.ctrl[id] = new Array("L_Index_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Index_3")){inMirror.ctrl[id] = new Array("L_Index_3", 1,"rotx","roty");id++;}
		
		if(inMirror.model.findChild("L_Major_1")){inMirror.ctrl[id] = new Array("L_Major_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Major_2")){inMirror.ctrl[id] = new Array("L_Major_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Major_3")){inMirror.ctrl[id] = new Array("L_Major_3", 1,"rotx","roty");id++;}
		
		if(inMirror.model.findChild("L_Ring_1")){inMirror.ctrl[id] = new Array("L_Ring_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Ring_2")){inMirror.ctrl[id] = new Array("L_Ring_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Ring_3")){inMirror.ctrl[id] = new Array("L_Ring_3", 1,"rotx","roty");id++;}
		
		if(inMirror.model.findChild("L_Kinky_1")){inMirror.ctrl[id] = new Array("L_Kinky_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Kinky_2")){inMirror.ctrl[id] = new Array("L_Kinky_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("L_Kinky_3")){inMirror.ctrl[id] = new Array("L_Kinky_3", 1,"rotx","roty");id++;}
		
		//Left Leg
		if(inMirror.model.findChild("L_THigh")){inMirror.ctrl[id] = new Array("L_THigh", 1, "rotx", "roty");id++;}
		if(inMirror.model.findChild("L_Knee")){inMirror.ctrl[id] = new Array("L_Knee", 1, "rotx", "roty");id++;}
		if(inMirror.model.findChild("L_Foot")){inMirror.ctrl[id] = new Array("L_Foot", 1, "rotx", "roty");id++;}
		if(inMirror.model.findChild("L_Toe")){inMirror.ctrl[id] = new Array("L_Toe", 1, "rotx", "roty");id++;}

		//Left Foot
		if(inMirror.model.findChild("L_Foot_Ctrl")){inMirror.ctrl[id] = new Array("L_Foot_Ctrl", 1, "posx", "roty", "rotz");id++;}
		if(inMirror.model.findChild("L_Foot_Roll1")){inMirror.ctrl[id] = new Array("L_Foot_Roll1", 1, "roty", "rotz");id++;}
		if(inMirror.model.findChild("L_Foot_Roll2")){inMirror.ctrl[id] = new Array("L_Foot_Roll2", 1, "roty", "rotz");id++;}
		if(inMirror.model.findChild("L_Foot_Roll4")){inMirror.ctrl[id] = new Array("L_Foot_Roll4", 1, "roty", "rotz");id++;}
		if(inMirror.model.findChild("L_Foot_Roll5")){inMirror.ctrl[id] = new Array("L_Foot_Roll5", 1, "roty", "rotz");id++;}
		
		//Right Arm 
		if(inMirror.model.findChild("R_Bicep")){inMirror.ctrl[id] = new Array("R_Bicep", 1, "rotx","roty");id++;}
		if(inMirror.model.findChild("R_Hand")){inMirror.ctrl[id] = new Array("R_Hand", 1, "rotx","roty");id++;}
		if(inMirror.model.findChild("R_Hand_IK")){inMirror.ctrl[id] = new Array("R_Hand_IK", 1, "posx","rotx","roty","sclx","scly","sclz");id++;}
		
		//Right Hand
		if(inMirror.model.findChild("R_Thumb_1")){inMirror.ctrl[id] = new Array("R_Thumb_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Thumb_2")){inMirror.ctrl[id] = new Array("R_Thumb_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Thumb_3")){inMirror.ctrl[id] = new Array("R_Thumb_3", 1,"rotx","roty");id++;}
		
		if(inMirror.model.findChild("R_Index_1")){inMirror.ctrl[id] = new Array("R_Index_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Index_2")){inMirror.ctrl[id] = new Array("R_Index_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Index_3")){inMirror.ctrl[id] = new Array("R_Index_3", 1,"rotx","roty");id++;}
		
		if(inMirror.model.findChild("R_Major_1")){inMirror.ctrl[id] = new Array("R_Major_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Major_2")){inMirror.ctrl[id] = new Array("R_Major_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Major_3")){inMirror.ctrl[id] = new Array("R_Major_3", 1,"rotx","roty");id++;}
		
		if(inMirror.model.findChild("R_Ring_1")){inMirror.ctrl[id] = new Array("R_Ring_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Ring_2")){inMirror.ctrl[id] = new Array("R_Ring_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Ring_3")){inMirror.ctrl[id] = new Array("R_Ring_3", 1,"rotx","roty");id++;}
		
		if(inMirror.model.findChild("R_Kinky_1")){inMirror.ctrl[id] = new Array("R_Kinky_1", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Kinky_2")){inMirror.ctrl[id] = new Array("R_Kinky_2", 1,"rotx","roty");id++;}
		if(inMirror.model.findChild("R_Kinky_3")){inMirror.ctrl[id] = new Array("R_Kinky_3", 1,"rotx","roty");id++;}
	
		//Right Leg
		if(inMirror.model.findChild("R_THigh")){inMirror.ctrl[id] = new Array("R_THigh", 1, "rotx", "roty");id++;}
		if(inMirror.model.findChild("R_Knee")){inMirror.ctrl[id] = new Array("R_Knee", 1, "rotx", "roty");id++;}
		if(inMirror.model.findChild("R_Foot")){inMirror.ctrl[id] = new Array("R_Foot", 1, "rotx", "roty");id++;}
		if(inMirror.model.findChild("R_Toe")){inMirror.ctrl[id] = new Array("R_Toe", 1, "rotx", "roty");id++;}

		//Right Foot
		if(inMirror.model.findChild("R_Foot_Ctrl")){inMirror.ctrl[id] = new Array("R_Foot_Ctrl", 1, "posx", "roty", "rotz");id++;}
		if(inMirror.model.findChild("R_Foot_Roll1")){inMirror.ctrl[id] = new Array("R_Foot_Roll1", 1, "roty", "rotz");id++;}
		if(inMirror.model.findChild("R_Foot_Roll2")){inMirror.ctrl[id] = new Array("R_Foot_Roll2", 1, "roty", "rotz");id++;}
		if(inMirror.model.findChild("R_Foot_Roll4")){inMirror.ctrl[id] = new Array("R_Foot_Roll4", 1, "roty", "rotz");id++;}
		if(inMirror.model.findChild("R_Foot_Roll5")){inMirror.ctrl[id] = new Array("R_Foot_Roll5", 1, "roty", "rotz");id++;}
		
		/*
		//Face
		var fCtrlsGrp = inMirror.model.groups.item("FaceAnims");
		if(fCtrlsGrp)
		{
			var fCtrls = fCtrlsGrp.Members;
			for(a=0;a<fCtrls.count;a++)
			{
				//if(fCtrls(a).name.indexOf("L_")!=-1 || fCtrls(a).name.indexOf("R_")!=-1){}
				
				inMirror.ctrl[id] = new Array(fCtrls(a).name, 1, "posx", "roty", "rotz");id++;

			}
		}
		*/

		for (i=0;i<inMirror.ctrl.length;i++)
		{
			for (d=2;d<inMirror.ctrl[i].length;d++)
			{
				AddMappingRule (inMirror.invValMap, inMirror.ctrl[i][0]+".kine.local."+inMirror.ctrl[i][d], "", GetNumMappingRules(inMirror.invValMap)+1);
			}
		}
	}
	
	inMirror.ctrl = new Array();
	for (i=0;i<GetNumMappingRules(inMirror.invValMap);i++)
	{
		inMirror.ctrl[i] = GetMappingRule(inMirror.invValMap, i+1)(1);
	}
	
	return inMirror
}

// Get Controlers
//------------------------------------------------------------
function getControlers(inMirror, inSeltype)
{
	inMirror.objects = XSIFactory.CreateActiveXObject("XSI.Collection");

	if (inSeltype == 1 || inMirror.actionType == 4)
	{
		var oGrp = inMirror.model.Groups.Item("Controls");
		inMirror.objects.AddItems(oGrp.members);
	}
	else
	{
		SelectChildNodes();	
		for (i=0;i<Selection.count;i++)
		{
			var oGrp = inMirror.model.Groups.Item("Controls");
			if(oGrp){if(oGrp.IsMember( Selection(i)))inMirror.objects.Add(Selection(i))};
		}
	}

	return inMirror;
}

// Get Parameters
//------------------------------------------------------------
function getParameters(inMirror)
{
	LogMessage("Get parameters!!!");
	LogMessage(inMirror.objects.GetAsText())
	inMirror.parameters = inMirror.objects.FindObjectsByMarkingAndCapabilities( "", siKeyable ); 
	return inMirror;
}

// Get Mirror Objects
//------------------------------------------------------------
function getMirrorObjects(inMirror)
{
	inMirror.mirrorObjects = XSIFactory.CreateActiveXObject("XSI.Collection");
	for(a=0;a<inMirror.objects.count;a++)
	{
		var iName = inMirror.objects(a).name;
		var tName = "";
		
		if(iName.substring(0,2) == "L_")tName = iName.replace("L_","R_");
		else if(iName.substring(0,2) == "R_")tName = iName.replace("R_","L_");
		else if(iName.indexOf("_L_")!=-1)tName = iName.replace("_L_","_R_");
		else if(iName.indexOf("_R_")!=-1)tName = iName.replace("_R_","_L_");
		
		else 
		{
			var c1 = iName.charAt(1);
			var lName = iName.toLowerCase();
			var c2 = lName.charAt(1);
		
			if(c1 != c2)
			{
				if(iName.substring(0,1) == "L" )tName = "R"+iName.substring(1,iName.length);
				else if(iName.substring(0,1) == "R" )tName = "L"+iName.substring(1,iName.length);
			}
		}
		
		if(tName!="")var mirrorObj = inMirror.model.FindChild(tName);
		if(mirrorObj)
		{
			inMirror.mirrorObjects.Add(mirrorObj);
			LogMessage(inMirror.objects(a).Name+" ---> "+mirrorObj.Name)
		}
	} 
	
	return inMirror;
}

// Key Mirror Parameters
//------------------------------------------------------------
function keyMirrorParameters(inMirror)
{
	SaveKeyOnKeyable(inMirror.mirrorObjects);
/*
	inMirror.mirrorParameters = inMirror.mirrorObjects.FindObjectsByMarkingAndCapabilities( "", siKeyable ); 
	SetMArking(inMirror.mirrorParameters);
	SetKey();
*/
	return inMirror;
}

// get Display Info
//------------------------------------------------------------
function getDisplayInfo(inMirror)
{
	inMirror.DisplayInfos = XSIFactory.CreateActiveXObject("XSI.Collection");
	for(a=0;a<inMirror.objects.count;a++)
	{
		var oEnum = new Enumerator(inMirror.objects(a).Properties);
		for(;!oEnum.atEnd();oEnum.moveNext())
		{
			if(oEnum.item().name.indexOf("DisplayInfo") != -1)inMirror.DisplayInfos.Add(oEnum.item());
		}
	}	
	return inMirror;
}

// Mirror pose 
//-----------------------------------------------------------
function mirrorPose(inMirror,inKeys)
{
	LogMessage(inMirror.parameters);
	// Store the action in static mode
	inMirror.action = StoreAction(inMirror.model, inMirror.parameters, 1, "Temp", 0, null, null, 0, 0, false, 0);
	inMirror.items = inMirror.action.SourceItems;
	
	// Edit the stored Action
	for (b=0;b<inMirror.items.count;b++)
	{
		// get the name of the object
		target = inMirror.items(b).target;
		if(target.indexOf("DisplayInfo")==-1)
		{
			pointPos = target.indexOf(".");
			obj = target.substring(0,pointPos);
		}
		
		else
		{
			obj = target;
		}
		
		// get the index in the board of correspondance
		for (c=0;c<inMirror.board.length;c++){if (obj == inMirror.board[c][0]){var index = c;}}
		
		// change the target to mirror action
		inMirror.items(b).target = target.replace(inMirror.board[index][0], inMirror.board[index][1]);
		
		//inverse the value of fcurve if necessary
		for (a=0; a<inMirror.ctrl.length; a++)
		{
			if (inMirror.ctrl[a] == inMirror.items(b).target)
			{
				inverseSource(inMirror.items(b).source);
				if(inMirror.ctrl[a].indexOf("HandIK.kine.local.rotx")!=-1)
				{
					correctHandIK(inMirror.items(b).source);
				}
			}
		}
	}
	
	ApplyAction(inMirror.action, inMirror.model, false, 0, 0, false);
	DeleteObj(mirror.action);
	
	if(inKeys == 1)
	{
		getMirrorObjects(inMirror);
		keyMirrorParameters(inMirror);
	}
		
	return inMirror;
}


// Mirror animation
//------------------------------------------------------------
function mirrorAnim(inMirror)
{
	// Store the Action
	inMirror.action = StoreAction(inMirror.model, inMirror.parameters, 6, "Temp", 0, inMirror.frameIn, inMirror.frameOut, inMirror.considerTime, 0, false, 0);
	inMirror.items = inMirror.action.SourceItems;
		
	// Edit the stored Action
	for (b=0;b<inMirror.items.count;b++)
	{
		// get the name of the object
		target = inMirror.items(b).target;
		if(target.indexOf("DisplayInfo")==-1)
		{
			pointPos = target.indexOf(".");
			obj = target.substring(0,pointPos);
		}
		
		else
		{
			obj = target;
		}
		// get the index in the board of correspondance
		for (c=0;c<inMirror.board.length;c++){if (obj == inMirror.board[c][0]){var index = c;}}
		
		// change the target to mirror action
		inMirror.items(b).target = target.replace(inMirror.board[index][0], inMirror.board[index][1]);
		
		// Set the static source to fcurve
		if (inMirror.items(b).type == "StaticValueAnimItem")
		{
			if (inMirror.considerTime == 0)
			{
				var fIn = getValue("PlayControl.In");
				var fOut = getValue("PlayControl.Out");
			}
			else
			{
				var fIn = inMirror.frameIn;
				var fOut = inMirror.frameOut;
			}
			inMirror.items(b).SetAsFcurve(Array(fIn,inMirror.items(b).source.value,0,0,0,0,fOut,inMirror.items(b).source.value,0,0,0,0));
		}
		
		//inverse the value of fcurve if necessary
		for (a=0; a<inMirror.ctrl.length; a++)
		{
			if (inMirror.ctrl[a] == inMirror.items(b).target)
			{
				inverseSource(inMirror.items(b).source);
				if(inMirror.ctrl[a].indexOf("HandIK.kine.local.rotx")!=-1)
				{
					correctHandIK(inMirror.items(b).source);
				}
			}
		}
		
		//now the offset
		if (inMirror.items(b).type != "ExpressionAnimItem")
		{addOffset (inMirror.items(b).source, inMirror.offset, inMirror.frameIn, inMirror.frameOut, inMirror.considerTime);}
	}

	ApplyAction(inMirror.action, inMirror.model, true, inMirror.frameIn, inMirror.frameOut, false);
	//DeleteObj(mirror.action);
	
	return inMirror;
}

// Remove animation
// This function only remove animation on keyable parameters
//------------------------------------------------------------
function removeAnim(inMirror, inColl)
{
	if (inMirror.considerTime == 0)	
	{
	for (i=0;i<inMirror.objects.count;i++)
		{
			Params = inColl(i).Properties("MarkingSet").Parameters;
			for (k=0;k<Params.count;k++)
			{
				fCurve = Params(k).source;
				if (fCurve && Params(k).source.type != "Expression"){fCurve.RemoveKeys();}
			}
		}	
	}
	else
	{
		for (i=0;i<inMirror.objects.count;i++)
		{
			Params = inColl(i).Properties("MarkingSet").Parameters;
			for (k=0;k<Params.count;k++)
			{
				fCurve = Params(k).source;
				if (fCurve && Params(k).source.type != "Expression")
				{
					fIn = inMirror.frameIn;
					fOut = inMirror.frameOut;
					fCurve.RemoveKeys(fIn, fOut);
				}
			}
		}	
	}
	
	return inMirror;
}

// Inverse Source
//-----------------------------------------------
function inverseSource(inSource)
{
	if (inSource.type == 20)
	{
		KeysColl = inSource.Keys;
		var reset = false;
		var current = inSource.Extrapolation;
		if( current > siConstantExtrapolation){
			reset = true;
			inSource.Extrapolation = siConstantExtrapolation;
		}
		for (j=0;j<KeysColl.count;j++)
		{
			KeysColl(j).value = - KeysColl(j).value;
		}
		if(reset){
			inSource.Extrapolation = current;
		}
	}
	else if (inSource.type == "StaticSource")
	{
		inSource.Value = - inSource.Value;
	}
}

// CorrectHandIK
//-----------------------------------------------
function correctHandIK(inSource)
{
	if (inSource.type == 20)
	{
		KeysColl = inSource.Keys;
		for (j=0;j<KeysColl.count;j++)
		{
			KeysColl(j).value = 180 - KeysColl(j).value;
		}
	}
	else if (inSource.type == "StaticSource")
	{
		inSource.value = (180-inSource.value);
	}
	
}


// Add Offset
//-----------------------------------------------
function addOffset(inFCurve, inOffset, inFIn, inFOut, inConsiderTime)
{
	KeysColl = inFCurve.Keys;

	if (inOffset>0)
	{
		for (j=KeysColl.count-1;j>=0;j--)
		{
			if ((inConsiderTime == 0) || (KeysColl(j).time >= inFIn && KeysColl(j).time <= inFOut))
			{KeysColl(j).time = KeysColl(j).time + inOffset;}
		}
	}
	else if (inOffset<0)
	{
		for (j=0;j<KeysColl.count;j++)
		{
			if ((inConsiderTime == 0) || (KeysColl(j).time >= inFIn && KeysColl(j).time <= inFOut))
			{KeysColl(j).time = KeysColl(j).time + inOffset;}
		}
	}

}


// Search character in a string
//------------------------------------------------------------
function searchChar(inString, inChar)
{
	for (i=inString.length-1;i>=0;i--)
	{
		charac = inString.charAt(i);
		if (charac == inChar){ nb = i; }
	}
	
	return nb;
}

// MuteMirror
//------------------------------------------------------------
function muteMirror(inMirror)
{
	inMirror.cnxMap.Active = false; 
	inMirror.invValMap.Active = false; 
	
	return inMirror;
}



// Get UI 
//------------------------------------------------------------
function getUI(inMirror)
{
	var remote = Dictionary.GetObject("PlayControl");
	var SceneIn = remote.In.value;
	var SceneOut = remote.Out.value;

	var UI = ActiveSceneRoot.AddProperty("Custom_parameter_list",0,"MirrorAnim");

	var SelectType = UI.AddParameter2("SelectType",siInt4 ,0,0,2,0,1,8,16,"Type","Type");

	var ActionType = UI.AddParameter2("ActionType",siInt4 ,1,1,5,0,1,8,16,"Type","Type");
	var Offset = UI.AddParameter2("Offset",siInt4 ,0,null,null,-15,15,8,16,"Offset","Offset");
	var ConsiderTime = UI.AddParameter2("ConsiderTime",siBool,0,0,1,0,1,8,16,"Consider Time","Active");	
	var FrameIn = UI.AddParameter2("FrameIn",siInt4 ,SceneIn ,null,null,SceneIn ,SceneOut ,8,16,"Frame In","Frame In");
	var FrameOut = UI.AddParameter2("FrameOut",siInt4 ,SceneOut ,null,null,SceneIn ,SceneOut ,8,16,"Frame Out","Frame Out");
	
	var layout = UI.PPGLayout;
	layout.Language = "JScript" ;
	
	layout.Logic =  OnInit.toString()
					+ ActionType_OnChanged.toString()
					+ ConsiderTime_OnChanged.toString()
					+ rebuildLayout.toString();

	rebuildLayout(UI);

	try{
		inspectobj( UI, "", "MirrorAnim", siModal);
	}
	catch(e){
		deleteobj(UI);
		logMessage ("Mirror Animation Canceled");
		return inMirror;
	}
	
	inMirror.selectType 		= SelectType.value;
	inMirror.actionType 		= ActionType.value;
	inMirror.offset		 		= Offset.value;	
	inMirror.considerTime		= ConsiderTime.value;
	inMirror.frameIn		 	= FrameIn.value;	
	inMirror.frameOut		 	= FrameOut.value;
	
	if(SelectType.value == 	1)inMirror.group = "ANIMs";
	else if(SelectType.value == 	2)inMirror.group = "FaceAnims";
	
	deleteobj(UI);

	return inMirror;
}

function rebuildLayout(inPPG)
{
	var SelectTypeItems = Array("On Selection Only",0,"All Body",1,"All Face",2);
	var ActionTypeItems = Array("Mirror Pose (no Keys)",1,"Mirror Pose(Keys)",2,"Mirror Animation",3, "Generate the templates (delete the current)",4);
	var Info = "                The mirror is apply folowing the y/z plane of the global SRT.";
	
	var layout = inPPG.PPGLayout;
	layout.Clear();
		
	layout.AddGroup("Selection Type");
	var item = layout.AddEnumControl("SelectType",SelectTypeItems,"Type",siControlRadio );
	layout.EndGroup();
	layout.AddGroup("Action");
	var item = layout.AddEnumControl("ActionType",ActionTypeItems,"Type",siControlCombo);
	if(inPPG.ActionType.value == 3)
	{
		layout.AddGroup("Offset");
		var item = layout.AddItem("Offset");
		layout.EndGroup();
		
		layout.AddGroup("In / Out");
		var item = layout.AddItem("ConsiderTime");
		var item = layout.AddItem("FrameIn");
		var item = layout.AddItem("FrameOut");
		layout.EndGroup();
	}
	layout.EndGroup();
	
	layout.AddGroup("Info");
	var item = layout.AddStaticText (Info);
	layout.EndGroup();
}

function OnInit()
{
	ConsiderTime_OnChanged();
}

function ActionType_OnChanged()
{
	rebuildLayout(PPG);
	PPG.Refresh() ;
}

function ConsiderTime_OnChanged()
{	
	PPG.FrameIn.Enable( PPG.ConsiderTime.value == 1);
	PPG.FrameOut.Enable( PPG.ConsiderTime.value == 1);
}