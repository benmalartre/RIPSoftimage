// Get Time datas
//---------------------------------
var fso = XSIFactory.CreateActiveXObject( "Scripting.FileSystemObject" );

var oRemoteControl = Dictionary.GetObject( "PlayControl" );
var start = oRemoteControl.Parameters("In").Value;
var end = oRemoteControl.Parameters("Out").Value;

var oProp = ActiveSceneRoot.AddCustomProperty("umapCopySyflexCacheFrames");

var oStart = oProp.AddParameter3("StartFrame",siInt4, start, -1000, 10000,0, 0);
var oEnd = oProp.AddParameter3("EndFrame",siInt4, end, -1000, 10000,0, 0);
var oCacheFile = oProp.AddParameter3("CacheFile",siString, "", "", "");

var oLayout = oProp.PPGLayout;
oLayout.Clear();
oLayout.AddGroup("Reference Frame");
oLayout.AddItem( "CacheFile", "Cache File", siControlFilePath );
oLayout.EndGroup();
oLayout.AddGroup("Target Frames");
oLayout.AddRow();
oLayout.AddItem("StartFrame","From Frame");
oLayout.AddItem("EndFrame","To Frame");
oLayout.EndRow();
oLayout.EndGroup();

var bCancelled = InspectObj(oProp,"","Copy Syflex Cache Frames",siModal);
if ( !bCancelled )
{
	var oFileName = oProp.CacheFile.Value;
   	var oPath = oFileName.split(".")[0];
	var start = oProp.StartFrame.value;
	var end = oProp.EndFrame.value;
	
	var endPadding = getPadding(end);
	var oFile = fso.GetFile(oProp.CacheFile.Value);

	var oProgressBar = XSIUIToolkit.ProgressBar;

	oProgressBar.Maximum = end - start;
	oProgressBar.Caption = "Copying Syflex Cache Files"
	oProgressBar.Visible = true

	for(a=start;a<end;a++)
	{
		var padding = getPadding(a);
		oFile.Copy ( oPath+"."+padding,true )
	
		if (oProgressBar.CancelPressed)
		{
	       logmessage("Progress bar cancelled at " + oProgressBar.Increment);
	       break;
	    }
 
		oProgressBar.StatusText = "Frame "+ padding;
		oProgressBar.Increment();
	
	}

	LogMessage("Copy Syflex Cache File Done...");
	oProgressBar.Visible = false;
}

DeleteObj(oProp);


function getPadding(iPadding)
{
	// check for negative padding
	var negate = false;
	var Padding = "";
	var tArray;
	if(iPadding<0)
	{
		negate = true;
		Padding = iPadding+"";
		tArray = Padding.split("-");
		Padding = tArray[0];	
	}
	else 
	{
		negate = false;
		Padding = iPadding+"";
	}

	// padding
	if(Padding.length==1)Padding = "000"+Padding;
	else if(Padding.length==2)Padding = "00"+Padding;
	else if(Padding.length==3)Padding = "0"+Padding;

	// negative padding
	if(negate == true)Padding = "-"+Padding;
	
	return Padding;
}

