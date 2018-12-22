// Clean Weights
//-------------------------------------------
// Add a push method to the JScript Array Object
// (Array.Push was added in Jscript 5.5 but we cannot rely on this)
var push = function(item){return this[this.length++] = item;}
Array.prototype.push = push;

function XSILoadPlugin( in_reg )
{
	in_reg.Author = "benmalartre";
	in_reg.Name = "bCleanWeightsPlugin";
	in_reg.Email = "";
	in_reg.URL = "";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterCommand("bCleanWeights","bCleanWeights");
	in_reg.RegisterMenu( siMenuTbAnimateDeformEnvelopeID, "bEnvelopeMenu", false, false);

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	var strPluginName;
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.",siVerbose);
	return true;
}

// bEnvelopeMenu_Init
//------------------------------------------------
function  bEnvelopeMenu_Init( in_ctxt )
{
   var oMenu = in_ctxt.source;
   var oSubMenu = oMenu.AddItem("bEnvelope", siMenuItemSubmenu);
   
   oSubMenu.AddCommandItem("&Clean Weights","bCleanWeights");
}

// bCleanWeights
//--------------------------------
function bCleanWeights_Init( in_ctxt )
{
	var oCmd;
	oCmd = in_ctxt.Source;
	oCmd.Description = "";
	oCmd.ReturnValue = true;

	var oArgs;
	oArgs = oCmd.Arguments;
	oArgs.AddWithHandler("inMeshs","Collection");
	oArgs.Add("inThreshold",siArgumentInput);
	return true;
}

function bCleanWeights_Execute(inMeshs,inThreshold)
{
	logmessage(inMeshs.count);
	if(inMeshs.count > 0)
	{
		if(!inThreshold)inThreshold = getThreshold();
		if(inThreshold>-1)
		{
			for(b=0;b<inMeshs.count;b++)
			{
				cleanWeightsForMesh(inMeshs(b),inThreshold);
			}
		}
	}
}

function cleanWeightsForMesh(inMesh,inThreshold)
{
	
	var oPrimitive = inMesh.ActivePrimitive;
	var oConstructionHistory = oPrimitive.ConstructionHistory;
	var oEnvelopeOp = oConstructionHistory.Filter("envelopop");

	if(oEnvelopeOp.count == 0)
	{
		logmessage(" No Envelope on "+ inMesh);
	}
	else
	{
		//applyBindPose(inMesh);
		var oEnvelope = oEnvelopeOp(0);	

		var oPort = oEnvelope.PortAt( 4, 0, 0 );
		var oEnvelopeProp = oPort.Target2;
		FreezeObj(oEnvelopeProp);

		var oDeformers = oEnvelope.Deformers;
		var oNbDeformers = oDeformers.count;
		var oPoints = inMesh.ActivePrimitive.Geometry.Points;
		var oCheckedDeformers = createCheckedDeformerArray(oNbDeformers);

		var oNewDeformers = XSIFactory.CreateActiveXObject("XSI.Collection");

		var oTooSmall = 5;
		var oCheck = 1;

		var oWeights = oEnvelope.Weights.Array.toArray();

		var oProgressBar = XSIUIToolkit.ProgressBar;
		oProgressBar.step = 1;
		oProgressBar.visible = true;
		oProgressBar.maximum = oPoints.count;
		oProgressBar.caption = "Cleaning weights for "+ inMesh +"..."

		while( oProgressBar.Value < oProgressBar.maximum )
		{
			var i = oProgressBar.Value

			var totalWeight = 0;
			var nbDeformer = 0;
	
			for(d=0;d<oNbDeformers;d++)
			{
				var oW = oWeights[oNbDeformers*i+d];
		
				if(oW<oTooSmall)
				{
					oWeights[oNbDeformers*i+d] = 0;
					oW = 0;
				}

				else
				{
					oCheckedDeformers[d] = 1;
					nbDeformer += 1;
				}
		
				totalWeight += oW;
		
			}
	 
			if(Math.abs(100 - totalWeight)>0.001)
			{
				//logmessage("Points #"+i+" is not normalized");
				if(totalWeight == 0)
				{
					//getClosestWeight();
					//need to implement a function to do this... héhéhé
				}
				else
				{
					normalizeWeight(oWeights,oNbDeformers*i,oNbDeformers*i+oNbDeformers,totalWeight,nbDeformer);
				}
			}
	
			if ( oProgressBar.CancelPressed )
			{	
				oCheck = 0;
		    	break ;
		    }

		   oProgressBar.Increment() ;
	
		}

		if(oCheck == 1)
		{
			for(j=0;j<oCheckedDeformers.length;j++)
			{
				if(oCheckedDeformers[j] == 1)
				{
					oNewDeformers.add(oDeformers(j));
				}
			}
	
			oEnvelope.Weights.Array = oWeights;
			FreezeObj(oEnvelopeProp);

			buildNewEnvelope(inMesh,oEnvelope,oNewDeformers)
		}
		
		oProgressBar.Visible = false ;
	}

}

function applyBindPose(inMesh)
{
	var Model = inMesh.Model;
	if(Model+".Mixer.BindPose")ApplyAction(Model+".Mixer.BindPose");
	else 
	resetActor(inMesh);
}

function getThreshold()
{
	var UI = ActiveSceneRoot.AddProperty("Custom_parameter_list",0,"eCleanWeights");
	
	var threshold = UI.AddParameter2("Threshold",siFloat,1,0,100,0,100,8,16,"Threshold","Threshold");

	try
	{
		inspectobj( UI, "", "bCleanWeights", siModal );
		var outThreshold = threshold.value;
		deleteObj(UI);
		return outThreshold;
	}
	
	catch(e)
	{
		deleteobj(UI);
		return -1;
	}
}

function createCheckedDeformerArray(inNbDeformers)
{
	var outArray = new Array();
	for(n=0;n<inNbDeformers;n++)
	{
		outArray[n] = 0;
	}
	return outArray;
} 

function normalizeWeight(inOutWeights,inStartIndex,inEndIndex,inTotalWeight,inNbDeformer)
{
	var oDiff = 100 - inTotalWeight;
	var t = 0;
	for(a=inStartIndex;a<inEndIndex;a++)
	{
		if(inOutWeights[a]>0)
		{
			var oMult = inOutWeights[a]/inTotalWeight;
			inOutWeights[a] += oDiff*oMult;
			t += inOutWeights[a];
		}
	}
	
	return inOutWeights;
}

function buildNewEnvelope(inMesh,inEnvelope,inNewDeformers)
{
	var oWeights = inEnvelope.PortAt( 4, 0, 0 ).Target2;
	var oObj = oWeights.Parent3DObject;
	var oDeformers = inEnvelope.Deformers;

	var fso = XSIFactory.CreateActiveXObject("Scripting.FileSystemObject");
	var oFolder = Application.ActiveProject.Path+"\\DSPresets";
	
	if(!fso.FolderExists(oFolder))
	{
		oFolder = fso.CreateFolder(oFolder);
	}
	
	var oPresetName = oObj.name +"_Weights";
	SavePreset( oWeights, oPresetName, oFolder, null, 1,null,null );
	RemoveFlexEnv(oObj);
	
	var oEnvelope = inMesh.ApplyEnvelope( inNewDeformers);
	var oWeights = oEnvelope.PortAt( 4, 0, 0 ).Target2;		
	LoadPreset( oFolder +"\\"+oPresetName, oWeights );
	
	return oEnvelope;

}


