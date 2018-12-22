// ApplyAction Only on Selected Controlers
//------------------------------------------------------------
function XSILoadPlugin( in_reg )
{
	in_reg.Author = "benmalartre";
	in_reg.Name = "ApplyAndPasteActionOnSelection";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterCommand("ApplyActionOnSelection","ApplyActionOnSelection");
	in_reg.RegisterCommand("PasteActionOnSelection","PasteActionOnSelection");

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.");
	return true;
}

// Apply Action On Selection
//---------------------------------------------
function ApplyActionOnSelection_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;

	var oArgs;
	oArgs = oCmd.Arguments;
	oArgs.Add("inAction",siArgumentInput);
	return true;
}

function ApplyActionOnSelection_Execute(inAction)
{
	ApplyAndPasteAction(0,inAction);
}


// Paste Action On Selection
//---------------------------------------------
function PasteActionOnSelection_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;

	var oArgs;
	oArgs = oCmd.Arguments;
	oArgs.Add("inAction",siArgumentInput);
	return true;
}
function PasteActionOnSelection_Execute(inAction)
{
	ApplyAndPasteAction(1,inAction);
}

// Apply And Paste Action 
//---------------------------------------------
function ApplyAndPasteAction(inMode,inAction)
{
	var oSel = Selection;
	var oCnt = oSel.count;

	if(oCnt>0)
	{
		var oAction;
		if(inAction==null)oAction = pickAction(oSel(0));
		else oAction = inAction;
		
		if(!oAction){logmessage ("eSelApplyAndPasteAction aborted ---> no or invalid action..."); return;}
		
		if(oAction.type == "Action")
		{
			var oState = getState(oAction);
			var oItems = oAction.SourceItems;
			for(a=0;a<oSel.count;a++)
			{
				checkItem(oSel(a),oItems);
			}
			if(inMode == 1)
			{
				UI = getPasteUI(oAction);
				if(UI)ApplyAction(oAction,"",1,UI.StartFrame,UI.EndFrame,UI.Ripple);
				else logmessage("Operation Canceled!!");
			}
			else
			{
				ApplyAction(oAction);
			}
			setState(oAction,oState);
		}
		else
		{
			logmessage("You have to pick an action...\n Operation aborted!!");
		}
	}
	else
	{
		logmessage("You have to select some controllers...\n Operation aborted!!");
	}
}

// get paste UI
//---------------------------------------------
function getPasteUI(inAction)
{
	var sourceStart = inAction.FrameStart.value;
	var sourceEnd = inAction.FrameEnd.value;

	var PPG = ActiveSceneRoot.AddProperty("Custom_parameter_list",0,"Paste Action On Selection");

	var StartFrame = PPG.AddParameter3( "StartFrame", siInt4, sourceStart);
	var EndFrame = PPG.AddParameter3( "EndFrame", siInt4, sourceEnd);
	var Ripple = PPG.AddParameter3("Ripple",siBool,0,0,1,0,0);

	var layout = PPG.PPGLayout;
	layout.clear();
	
	layout.addGroup("Options");
	layout.addItem("StartFrame");
	layout.addItem("EndFrame");
	layout.addItem("Ripple");

	try
	{
		inspectobj( PPG, "", "Paste Action On Selection", siModal);
	}
	catch(e)
	{
		deleteobj(PPG);
		logMessage ("Paste Action On Selection Canceled");
		return 0;
	}
	
	var oUI = new Object();
	oUI.StartFrame = PPG.StartFrame .value;
	oUI.EndFrame = PPG.EndFrame .value;
	oUI.Ripple = PPG.Ripple.value;

	deleteobj(PPG);
	return oUI;
}

// pick action
//---------------------------------------------
function pickAction(inObj)
{
	var oModel = inObj.model;
	var oActions = oModel.Sources;
	var oFiltered = XSIFactory.CreateActiveXObject("XSI.Collection");
	
	for(a=0;a<oActions.count;a++)
	{
		if(oActions(a).Type == "Action")oFiltered.Add(oActions(a));
	}
	var oPick = OpenTransientExplorer( oFiltered, siSEFilterAllNodesNoParams, 0 )(0);
	return oPick;
}

// check item
//---------------------------------------------
function checkItem(inObj,inItems)
{
	var target,pPos,obj;
	for(x=0;x<inItems.count;x++)
	{
		target = inItems(x).target;
		pPos = target.indexOf(".");
		obj = target.substring(0,pPos);
		if(obj == inObj.name)inItems(x).active = 1;
	}
}

// get State
//---------------------------------------------
function getState(inAction)
{
	var oItems = inAction.SourceItems;
	var oState = new Array(oItems.count);
	
	// get the active state for each item
	for (b=0;b<oItems.count;b++){oState[b] = oItems(b).active;oItems(b).active = 0;}
	return oState;
}

// set State
//---------------------------------------------
function setState(inAction,inState)
{
	var oItems = inAction.SourceItems;
	
	// set the active state for each item
	for (b=0;b<oItems.count;b++){oItems(b).active = inState[b];}
}
