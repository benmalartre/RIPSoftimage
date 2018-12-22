// SynopticPlugin
//------------------------------------------------
function XSILoadPlugin( in_reg )
{
	in_reg.Author = "benmalartre";
	in_reg.Name = "SynopticFunctionsPlugin";
	in_reg.Email = "";
	in_reg.URL = "";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterCommand("SynopticFunctions","SynopticFunctions");
	in_reg.RegisterCommand("SynopticSelect","SynopticSelect");

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.");
	return true;
}

function eSynopticFunctions_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "Synoptic Functions";
	oCmd.ReturnValue = true;

	var oArgs;
	oArgs = oCmd.Arguments;
	oArgs.Add("inObj",siArgumentInput);
	oArgs.Add("inFunction",siArgumentInput);
	return true;
}

function eSynopticFunctions_Execute( inModel,inFunction )
{	
	switch (inFunction) 
	{
		case "Sel_All" :	// Select all controllers find in Control Group
			if(inModel.Groups.Item("Controls"))
			{
				SelectObj(inModel.Groups.Item("Controls").Members);	
			}
			break;	
			
		case "Key_Sel" :	// Will key Selected object using keyable parameters
			if(Selection.count>0)
			{
				SaveKeyOnKeyable( Selection );
			}
			break;	
			
		case "Key_All" :	// Will key All controls object using character key sets
			var oSel = GetValue("SelectionList");
			var oKeySet = inModel.properties("CharacterKeySet");
			if(oKeySet)
			{
				var oSubKeySets = oKeySet.NestedObjects;
				for(i=0;i<oSubKeySets.count;i++)
				{
					if(oSubKeySets(i).name.indexOf("Face") == -1)
					{
						SetCurrentCharacterKeySet( oSubKeySets(i));
						SaveKeyOnCharacterKeySet();
					}
				}
			}
			
			else if(inModel.Groups.Item("Controls"))
			{
				SelectObj(inModel.Groups.Item("Controls").Members);	
				SaveKeyOnKeyable();
			}
			
			else
			{
				logmessage("Not valid model: no character key set, no control group...\n"+
							"Operation aborted...");
			}
			SelectObj(oSel);
			
			break;
			
		case "Pose_Sel" : // Will pose Selected object using keyable parameters
			eSynopticFunctions_Pose_Sel(inModel);
			break;	
			
		case "Pose_All" : // Will pose All controls objects using keyable parameters
			eSynopticFunctions_Pose_All(inModel,"Controls");
			break;	
			
		case "Store_Sel" : // Will store Selected object using keyable parameters
			eSynopticFunctions_Store_Sel(inModel);
			break;	
			
		case "Store_All" : // Will store All controls objects using keyable parameters
			eSynopticFunctions_Store_All(inModel,"Controls");
			break;	
			
		case "Remove_Sel" : // Will remove Animation on selected object using keyable parameters
			eSynopticFunctions_Remove_Sel(inModel);
			break;	
		
		case "Remove_All" : // Will remove Animation on all control objects using keyable parameters
			eSynopticFunctions_Remove_All(inModel,"Controls");
			break;	
			
		case "Reset_Sel" : // Will reset selected object using keyable parameters
			eSynopticFunctions_Reset_Sel(inModel);
			break;	
		
		case "Reset_All" : // Will reset all control objects using keyable parameters
			eSynopticFunctions_Reset_All(inModel,"DefaultPose","Controls");
			break;
			
		case "Sel_All_Face" : // Will select all controls in Face Control Group
			if(inModel.Groups.Item("FaceAnims"))
			{
				SelectObj(inModel.Groups.Item("FaceAnims").Members);	
			}
			break; 
			
		case "Key_All_Face" :	// Will key controls object using character key set "FacialKeySet"...
			var oSel = GetValue("SelectionList");
			
			var oKeySet = GetThisCharacterKeySet(inModel,"Facial");
			
			if(oKeySet )
			{
				SetCurrentCharacterKeySet( oKeySet);
				SaveKeyOnCharacterKeySet();
			}
			
			
			else if(inModel.Groups.Item("FaceAnims"))
			{
				SelectObj(inModel.Groups.Item("FaceAnims").Members);	
				SaveKeyOnKeyable();
			}
			
			else
			{
				logmessage("Not valid model: no facial key set, no facecontrol group...\n"+
							"Operation aborted...");
			}
			SelectObj(oSel);
			
			break;
			
		case "Pose_All_Face" : // Will pose All face controls objects using keyable parameters
			eSynopticFunctions_Pose_All(inModel, "FaceAnims");
			break;	
			
		case "Store_All_Face" : // Will store facecontrol  objects using keyable parameters
			eSynopticFunctions_Store_All_Face(inModel);
			break;	
			
		case "Remove_All_Face" : // Will remove Animation on facecontrol objects using keyable parameters
			eSynopticFunctions_Remove_All(inModel,"FaceAnims");
			break;	
		
		case "Reset_All_Face" : // Will reset facecontrol objects using keyable parameters
			eSynopticFunctions_Reset_All(inModel,"FaceDefaultPose","FaceAnims");
			break;
			
		case "Load_Pose" : // Will open an explorer and load a pose in the model's mixer
			eSynopticFunctions_Load_Pose(inModel);
			break;
		
		/*	
		case "inspectFaceControl" : // Will open face control in an object view..
			var oFaceControl =  inModel.FindChild("Face_Control_Root");
			SelectObj( "B:"+oFaceControl);
			
			
			if(oFaceControl)
			{
				var oFaceView = desktop.activelayout.createview("Object View", inModel.name +"_FaceControl_View" )
				oFaceView.setattributevalue ("lockstatus", "true");
			}
			break;
		*/
			
		
			
		
	}
	return true;
}


// Synoptic Select
//---------------------------------------------
function eSynopticSelect_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "Synoptic Functions";
	oCmd.ReturnValue = true;

	var oArgs;
	oArgs = oCmd.Arguments;
	oArgs.Add("inObj",siArgumentInput);
	oArgs.Add("inNameArray",siArgumentInput,0,siDispatch);
	return true;
}

function eSynopticSelect_Execute(inModel, inNameArray)
{
	var toSel = XSIFactory.CreateActiveXObject("XSI.Collection");
	
	for(a=0;a<inNameArray.length;a++)
	{
		var oItem = inModel.FindChild(inNameArray[a]);
		if(oItem)toSel.Add(oItem);
	}
	
	var rtn = GetKeyboardState();
	var mod;

	KeyPressed = rtn(1);

	switch (KeyPressed) 
		{
		case 0 :	// No Key or No Valid Key
			SelectObj (toSel);
			break;	
		case 1 :	// Shift	check the object isn't already selected
			for (i=0; i<toSel.Count; i++)
			{if (toSel(i).Selected(3) == 0)	{AddToSelection (toSel(i));}}
			break;
		case 2 : 	// Ctrl
			ToggleSelection (toSel);
			break;	
		case 3 :	// Shift+Ctrl	
			RemoveFromSelection (toSel);				
			break;
		case 4 :	// Alt
			SelectObj (toSel, "BRANCH", true);
			break;
		case 5 : 	// Alt+Shift      check the object isn't already selected
			for (i=0; i<toSel.Count; i++)
			{if (toSel(i).Selected(3) == 0)	{AddToSelection (toSel(i), "BRANCH", true);}}
			break;
		case 6 :  	// Alt+Ctrl
			ToggleSelection (toSel, "BRANCH", true);
			break;
		case 7 :   // Alt+Shift+Ctrl
			RemoveFromSelection (toSel);
			break;
		}
}

//-----------------------------------------------------
function eSynopticFunctions_Pose_Sel(inModel)
{
	var cFrame = getFrame("Current");
	var oSel = GetValue("SelectionList");
	
	if (Selection.Count > 0)
	{	
		var pose = new Object();
		pose.Model = inModel;
		
		SelectChildNodes();	
		pose = get_poseUI(pose);
		
		var oColl = XSIFactory.CreateActiveXObject("XSI.Collection");
		oColl.AddItems(Selection);
		var oKeyable = oColl.FindObjectsByMarkingAndCapabilities( "", siKeyable );

		if (!pose.Name)
		{
			logmessage ("Pose Action Cancelled");
			return;
		}
		else
		{
			StoreAction( pose.Model, oKeyable, 1, pose.Name, 0, null, null, 0, 0, 0, 0 );
			DeselectAll();
			if (pose.Export == 1)
			{
				ExportAction(pose.Model+".Mixer."+pose.Name, pose.Folder+"\\"+pose.Name+".eani");
			}
		}
	}
	else
	{
		XSIUIToolkit.Msgbox( "No Selected Objet", siMsgExclamation, "Pose Sel Info" ) ;
	}
	setFrame("Current",cFrame);
	SelectObj(oSel);

}

//-----------------------------------------------------
function eSynopticFunctions_Pose_All(inModel,inCtrlGroup)
{
	var cFrame = getFrame("Current");
	var oSel = GetValue("SelectionList");
	
	var pose = new Object();
	pose.Model = inModel;
	var oKeyable;
	
	var oKeySet = GetThisCharacterKeySet(inModel,"Facial");
	if(oKeySet)
	{
		pose.Keyable = oKeySet.Parameters.GetAsText();
		pose = get_poseUI(pose);
	}
	
	/*
	else if(inModel.Groups.Item(inCtrlGroup))
	{
		var oCtrls = inModel.Groups.Item(inCtrlGroup).Members;	
		pose = get_poseUI(pose);
		
		var oColl = XSIFactory.CreateActiveXObject("XSI.Collection");
		oColl.AddItems(oCtrls);
		pose.Keyable = oColl.FindObjectsByMarkingAndCapabilities( "", siKeyable );
	}
	*/
	
	else
	{
		XSIUIToolkit.Msgbox( "No Facial Key Set : Operation aborted!!", siMsgExclamation, "Pose All Info" ) ;
		return;
	}
	
	if (!pose.Name)
	{
		logmessage ("Pose Action Cancelled");
		return;
	}
	else
	{
		StoreAction( pose.Model, pose.Keyable, 1, pose.Name, 0, null, null, 0, 0, 0, 0 );
		DeselectAll();
		if (pose.Export == 1)
		{
			ExportAction(pose.Model+".Mixer."+pose.Name, pose.Folder+"\\"+pose.Name+".eani");
		}
	}
		
	setFrame("Current",cFrame);
	SelectObj(oSel);

}


//-----------------------------------------------------
function eSynopticFunctions_Store_Sel(inModel)
{
	var cFrame = getFrame("Current");
	var oSel = GetValue("SelectionList");
	
	if (Selection.Count > 0)
	{	
		var store = new Object;
		store.Model = inModel;
		
		SelectChildNodes();	
		store= get_storeUI(store);
		
		var oColl = XSIFactory.CreateActiveXObject("XSI.Collection");
		oColl.AddItems(Selection);
		var oKeyable = oColl.FindObjectsByMarkingAndCapabilities( "", siKeyable );

		if (!store.Name)
		{
			logmessage ("Store Action Cancelled");
			return;
		}
		
		else
		{
			if (store.Remove == 1)
			{
				TempPose = StoreAction( store.Model, null, 1, "TempPose", 0, null, null, 0, 0, 0, 0 );
			}
			StoreAction( store.Model, null, 6, store.Name, store.Remove, store.In, store.Out, 0, 0, 0, 0 );
			DeselectAll();
			if (store.Export == 1)
			{
				ExportAction(store.Model+".Mixer."+store.Name, store.Folder+"\\"+store.Name+".eani");
			}
			if (store.Remove == 1)
			{
				ApplyAction(TempPose, null, false, 0, 0, false);
				DeleteObj(TempPose);
			}
		}
	}
	
	else
	{
		XSIUIToolkit.Msgbox( "No Selected Objet", siMsgExclamation, "Store Sel Info" );
	}
	
	setFrame("Current",cFrame);
	SelectObj(oSel);
}

//-----------------------------------------------------
function eSynopticFunctions_Store_All(inModel,inCtrlGroup)
{
	var cFrame = getFrame("Current");
	var oSel = GetValue("SelectionList");
	
	if(inModel.Groups.Item(inCtrlGroup))
	{
		var oCtrls = inModel.Groups.Item(inCtrlGroup).Members;	

		var store = new Object;
		store.Model = inModel;
		
		store= get_storeUI(store);
		
		var oColl = XSIFactory.CreateActiveXObject("XSI.Collection");
		oColl.AddItems(oCtrls);
		var oKeyable = oColl.FindObjectsByMarkingAndCapabilities( "", siKeyable );

		if (!store.Name)
		{
			logmessage ("Store Action Cancelled");
			return;
		}
		
		else
		{
			if (store.Remove == 1)
			{
				TempPose = StoreAction( store.Model, null, 1, "TempPose", 0, null, null, 0, 0, 0, 0 );
			}
			StoreAction( store.Model, null, 6, store.Name, store.Remove, store.In, store.Out, 0, 0, 0, 0 );
			DeselectAll();
			if (store.Export == 1)
			{
				ExportAction(store.Model+".Mixer."+store.Name, store.Folder+"\\"+store.Name+".eani");
			}
			if (store.Remove == 1)
			{
				ApplyAction(TempPose, null, false, 0, 0, false);
				DeleteObj(TempPose);
			}
		}
	}
	
	else
	{
		XSIUIToolkit.Msgbox( "No Control Group", siMsgExclamation, "Store All Info" );
	}
	
	setFrame("Current",cFrame);
	SelectObj(oSel);
}


//-----------------------------------------------------
function eSynopticFunctions_Store_All_Face(inModel)
{
	var cFrame = getFrame("Current");
	var oSel = GetValue("SelectionList");
	
	var store = new Object;
	store.Model = inModel;
	
	var oKeySet = GetThisCharacterKeySet(inModel,"Facial");
	if(oKeySet)
	{
		store.Keyable = oKeySet.Parameters.GetAsText();
		store= get_storeUI(store);
	}
	
	/*
	if(inModel.Groups.Item(inCtrlGroup))
	{
		var oCtrls = inModel.Groups.Item(inCtrlGroup).Members;	
		
		var oColl = XSIFactory.CreateActiveXObject("XSI.Collection");
		oColl.AddItems(oCtrls);
		store.Keyable = oColl.FindObjectsByMarkingAndCapabilities( "", siKeyable );
	}
	*/
	
	else
	{
		XSIUIToolkit.Msgbox( "No Facial Key Set :: Operation Aborted!!", siMsgExclamation, "Store All Info" );
		return;
	}
	
	if (!store.Name)
	{
		logmessage ("Store Action Cancelled");
		return;
	}
	
	else
	{
		if (store.Remove == 1)
		{
			TempPose = StoreAction( store.Model, store.Keyable, 1, "TempPose", 0, null, null, 0, 0, 0, 0 );
		}
		StoreAction( store.Model, store.Keyable, 6, store.Name, store.Remove, store.In, store.Out, 0, 0, 0, 0 );
		DeselectAll();
		if (store.Export == 1)
		{
			ExportAction(store.Model+".Mixer."+store.Name, store.Folder+"\\"+store.Name+".eani");
		}
		if (store.Remove == 1)
		{
			ApplyAction(TempPose, null, false, 0, 0, false);
			DeleteObj(TempPose);
		}
	}
		
	setFrame("Current",cFrame);
	SelectObj(oSel);
}
//-----------------------------------------------------
function eSynopticFunctions_Remove_Sel(inModel)
{
	var oSel = GetValue("SelectionList");
	
	if (Selection.Count > 0)
	{			
		var oMsg = "warning : this action will delete all keys on " + Selection.getAsText() +"\n"+
								"are you sure you want to do it?";
								
		var buttonPressed = XSIUIToolkit.Msgbox( oMsg, siMsgYesNo | siMsgQuestion, "Remove Sel Info");
	
		if ( buttonPressed == siMsgYes )
		{
			SelectChildNodes();	
		
			var oColl = XSIFactory.CreateActiveXObject("XSI.Collection");
			oColl.AddItems(Selection);
			var oKeyable = oColl.FindObjectsByMarkingAndCapabilities( "", siKeyable );
			removeAnimation(oKeyable);
		}
	}
	
	else
	{
		XSIUIToolkit.Msgbox( "No Selected Objet", siMsgExclamation, "Remove Sel Info" );
	}
	SelectObj(oSel);
}

//-----------------------------------------------------
function eSynopticFunctions_Remove_All(inModel,inCtrlGroup)
{
	var oSel = GetValue("SelectionList");
	var oMsg = "warning : this action will delete all keys on " + inModel.name +"\n"+
							"are you sure you want to do it?";
	
	var buttonPressed = XSIUIToolkit.Msgbox( oMsg, siMsgYesNo | siMsgQuestion, "Remove All Info");
         
	if ( buttonPressed == siMsgYes )
	{
		if(inModel.Groups.Item(inCtrlGroup))
		{
			var oCtrls = inModel.Groups.Item(inCtrlGroup).Members;	

			var oColl = XSIFactory.CreateActiveXObject("XSI.Collection");
			oColl.AddItems(oCtrls);
			var oKeyable = oColl.FindObjectsByMarkingAndCapabilities( "", siKeyable );

			removeAnimation(oKeyable);
		}
	
		else
		{
			XSIUIToolkit.Msgbox( "No Control Group!!", siMsgExclamation, "Remove All Info" );
		}
		SelectObj(oSel);
	}
}

//-----------------------------------------------------
function eSynopticFunctions_Reset_Sel(inModel)
{
	var oSel = GetValue("SelectionList");
	
	if (Selection.Count > 0)
	{			
		SelectChildNodes();
		resetTransform();
	}
	
	else
	{
		XSIUIToolkit.Msgbox( "No Selected Objet", siMsgExclamation, "Reset Sel Info" );
	}
	SelectObj(oSel);
}

//-----------------------------------------------------
function eSynopticFunctions_Reset_All(inModel,inPose,inGroup)
{
	var oSel = GetValue("SelectionList");
	
	// find default pose, if not use reset transform on objects find in Control group
	var oSources = new Enumerator(inModel.Sources);
	var oPose;
	for(;!oSources.atEnd();oSources.moveNext())
	{
		if(oSources.item().name == inPose)
		{
			oPose = oSources.item();
		}
	}
	if (oPose)ApplyAction(oPose);
	else
	{
		if(inModel.Groups.Item(inGroup))
		{
			var oCtrls = inModel.Groups.Item(inGroup).Members;
			resetTransform(oCtrls);
		}
	
		else
		{
			XSIUIToolkit.Msgbox( "No Control Group!!", siMsgExclamation, "Reset All Info" );
		}
	}
	SelectObj(oSel);
}

//-----------------------------------------------------
function getFrame(inFrameName)
{
	var oRemote = Dictionary.GetObject( "PlayControl" );
	return (oRemote.Parameters(inFrameName).value);
}
//-----------------------------------------------------
function setFrame(inFrameName,inValue)
{
	var oRemote = Dictionary.GetObject( "PlayControl" );
	oRemote.Parameters(inFrameName).value = inValue;
}

// Pose UI
//---------------------------------------------
function get_poseUI(inPose)
{

	var sliders = ActiveSceneRoot.AddProperty("Custom_parameter_list",0,"eStoreAction");

	var PoseName = sliders.AddParameter2("PoseName", siString, inPose.Model+"_NewPose", 0,1,0,1,8,16,"Action Name","Type");
	var PoseExport = sliders.AddParameter2("PoseExport",siBool,0,0,1,0,1,8,16,"Export Action","Active");
	var PoseFolder = sliders.AddParameter2("PoseFolder", siString, "", 0,1,0,1,8,16,"Export Folder","Type");
	
	var layout = sliders.PPGLayout;
	layout.Clear();
	layout.Language = "JScript" ;
	layout.Logic =   OnInit.toString()
			+ PoseExport_OnChanged.toString();

	//===============================================
	layout.AddGroup("New Source");
	var item = layout.AddItem("PoseName");
	layout.EndGroup();
	layout.AddGroup("Save Action As Well");
	var item = layout.AddItem("PoseExport");
	var item = layout.AddItem("PoseFolder", "Browse", siControlFolder);
	layout.EndGroup();

	try{
		inspectobj( sliders, "", "ls Pose to Action", siModal );
	}
	catch(e){
		deleteobj(sliders);
		logmessage ( e.description, siError);
		return inPose;
	}
	
	inPose.Name 		= PoseName.value;
	inPose.Export 		= PoseExport.value;
	inPose.Folder 		= PoseFolder.value;
	
	deleteobj(sliders);

	return inPose;
}
function OnInit(){	PoseExport_OnChanged();}
function PoseExport_OnChanged(){PPG.PoseFolder.Enable( PPG.PoseExport.value == 1);}

// Store UI
//---------------------------------------------
function get_storeUI(inStore)
{
	//Get Default Values
	var startKey = FirstKey(inStore.Keyable,null);
	var endKey = LastKey(inStore.Keyable,null);

	if (startKey > endKey)
	{
		XSIUIToolkit.Msgbox( "No key found on the model", siMsgExclamation, "Store All Info" ) ;
		return 0;
	}
	if (startKey == endKey)
	{
		XSIUIToolkit.Msgbox( "Only one key on the model. Please use the 'Pose' button", siMsgExclamation, "Store All Info" ) ;
		return 0;
	}

	var sliders = ActiveSceneRoot.AddProperty("Custom_parameter_list",0,"eStoreAction");

	var StoreName = sliders.AddParameter2("StoreName", siString, inStore.Model+"_NewAction", 0,1,0,1,8,16,"Action Name","Type");
	var In = sliders.AddParameter2("In",siInt4 ,startKey,startKey,endKey,startKey,endKey,8,16,"Frame In","Frame In");
	var Out = sliders.AddParameter2("Out",siInt4 ,endKey,startKey,endKey,startKey,endKey,8,16,"Frame Out","Frame Out");
	var RemoveAnim = sliders.AddParameter2("RemoveAnim",siBool,0,0,1,0,1,8,16,"Remove Original Animation","Remove");
	var StoreExport = sliders.AddParameter2("StoreExport",siBool,0,0,1,0,1,8,16,"Export Action","Active");
	var StoreFolder = sliders.AddParameter2("StoreFolder", siString, "", 0,1,0,1,8,16,"Export Folder","Type");
	
	var layout = sliders.PPGLayout;
	layout.Clear();
	layout.Language = "JScript" ;
	layout.Logic =   On2Init.toString()
		+ StoreExport_OnChanged.toString();

	//===============================================
	layout.AddGroup("New Source");
	var item = layout.AddItem("StoreName");
	var item = layout.AddItem("In");
	var item = layout.AddItem("Out");
	layout.EndGroup();
	layout.AddGroup("Once Done Creating The Source...");
	var item = layout.AddItem("RemoveAnim");
	layout.EndGroup();
	layout.AddGroup("Save Action As Well");
	var item = layout.AddItem("StoreExport");
	var item = layout.AddItem("StoreFolder", "Browse", siControlFolder);
	layout.EndGroup();

	try{
		inspectobj( sliders, "", "ls Store Action", siModal );
	}
	catch(e){
		deleteobj(sliders);
		//logmessage ( e.description, siError);
		return inStore;
	}
	
	inStore.Name 		= StoreName.value;
	inStore.In		 	= In.value;
	inStore.Out	 		= Out.value;
	inStore.Remove 		= RemoveAnim.value;
	inStore.Export 		= StoreExport.value;
	inStore.Folder 		= StoreFolder.value;
		
	deleteobj(sliders);

	return inStore;
}

function On2Init()
{	
	StoreExport_OnChanged();
}

function StoreExport_OnChanged()
{
	PPG.StoreFolder.Enable( PPG.StoreExport.value == 1);
}

// Load Pose UI
//---------------------------------------------
function eSynopticFunctions_Load_Pose(inModel)
{

}

// GetThisCharacterKeySet
//---------------------------------------------
function GetThisCharacterKeySet(inModel,inString)
{
	var oProps = inModel.properties;
	var oKeySets = GetAllCharacterKeySets();
	
	for(a=0;a<oProps.count;a++)
	{
		if(oProps(a).type == "customparamset")
		{
			if(oKeySets(""+oProps(a)+"."+inString))return(oKeySets(""+oProps(a)+"."+inString));
		}
	}
}