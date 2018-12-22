umapClothCorrectiveSmoothMap(Selection(0))

function umapClothCorrectiveSmoothMap(inObject)
{
	var oCmdLog = Application.Preferences.GetPreferenceValue("scripting.cmdlog");
	Application.Preferences.SetPreferenceValue("scripting.cmdlog", false);
	
	// Get Datas
	if(! inObject || inObject.type != "polymsh")
	{
		LogMessage("umapClothCorrectiveSmoothMap ---> invalid selection",siError);
		return;
	}
	
	var oPrim = inObject.ActivePrimitive;
	
	var oRemoteControl = Dictionary.GetObject("PlayControl");
	var cFrame = parseInt(oRemoteControl.Parameters( "Current" ).Value+0.1);

	// check if cluster exists
	var oCls = oPrim.Geometry.Clusters("CorrectiveSmoothCls");
	if(!oCls)
	{
		oCls = oPrim.Geometry.AddCluster(siVertexCluster,"CorrectiveSmoothCls")
	}

	// check if a smooth map already exists for this frame
	var SmoothMapExist = false;
	var oSmoothMap;
	if(oCls.Properties("Frame"+cFrame))
	{
		SmoothMapExist = true;
		var buttonPressed = XSIUIToolkit.MsgBox( "A corrective smooth map already exists for this frame...",
			siMsgOkOnly, "Corrective Smooth Map" );

		if ( buttonPressed == siMsgCancel )
		{
		   LogMessage( "Store Smooth Map cancelled by the user!!",siInfo ) ;
		   return;
		}
	}
	else
	{
		oSmoothMap = oCls.AddProperty("Weight Map Property",false,"Frame"+cFrame);
	}

	// check if tree exists
	var oTree = oPrim.ICETrees.Find( "CorrectiveSmooth" );
	if(!oTree)
	{
		oTree = ApplyOp("ICETree", inObject, siNode, null, null, siConstructionModeAnimation )(0);
		oTree.Name = "CorrectiveSmooth";
		
		var oSet = AddICENode("SetOneDataNode", oTree);
		SetValue(oSet+".Reference", "Self.PointPosition", null);
		var oIf = AddIceNode("IfNode",oTree);
		
		var oRepeat = AddICENode("RepeatNode", oTree);
		ConnectICENodes(oRepeat+".port", oSet+".value");
		SetValue(oRepeat+".iterations", 10);

		ConnectICENodes(oTree+".port1", oIf+".Result");
		ConnectICENodes(oIf+".IfFalse", oSet+".Value");
		ConnectICENodes(oIf+".IfFalse", oRepeat+".Execute");
		
		var oGet = AddICENode( "GetDataNode", oTree );
		SetValue( oGet+".Reference", "Self.PointPosition");
		var oGet1 = AddICENode( "GetDataNode", oTree );
		SetValue( oGet1+".Reference", "Self.PointNeighbors");
		var oGet2 = AddICENode( "GetDataNode", oTree );
		SetValue( oGet2+".Reference", "PointPosition");
		ConnectICENodes(oGet2+".Source", oGet1+".Value");
		
		var oAverage = AddICENode("GetArrayAverageNode",oTree);
		ConnectICENodes(oAverage+".Array", oGet2+".Value");
		
		var oInterp = AddIceNode("LinearInterpolateNode",oTree);
		
		ConnectICENodes(oInterp +".First", oGet+".Value");
		ConnectICENodes(oInterp+".Second", oAverage+".Result");
		ConnectIceNodes(oSet+".Source",oInterp+".Result");
		
		var oClamp = AddICENode("ClampNode", oTree);
		ConnectICENodes(oInterp+".Blend", oClamp+".Result");
		SetValue(oClamp+".Limit2", 1);
		
		var oAdd = AddICENode("AddNode", oTree);
		ConnectICENodes(oClamp+".Value", oAdd+".Result");
		
		var oCompound = CreateICECompoundNode(oGet+","+oSet+","+oGet1+","+oGet2+","+oAverage+","+oInterp+","+oRepeat+","+oIf+","+oClamp+","+oAdd,null);
		EditICECompoundProperties(oCompound, "umapCorrectiveSmooth", "", "", "", "", "", 1, 0, "", 4, 6732954);
		AddExposedParamToICECompoundNode(oIf+".Condition", oCompound, null, "Mute");
		AddExposedParamToICECompoundNode(oRepeat+".Iterations", oCompound, null, "Smooth");
	
	}
	
	if(!SmoothMapExist)
	{
		var oCompound = oTree.Nodes("umapCorrectiveSmooth");
		var oAdd = oCompound.Nodes.Filter("AddNode")(0);
		
		var oMult = AddIceNode("MultiplyByScalarNode",oCompound);
		
		if(oAdd.InputPorts.count == 2 && !oAdd.InputPorts(0).IsConnected)
		{
			ConnectIceNodes(oAdd+".Value1",oMult+".Result");
		}
		else if(oAdd.InputPorts.count == 2 && !oAdd.InputPorts(1).IsConnected)
		{
			ConnectIceNodes(oAdd+".Value2",oMult+".Result");
		}
		else
		{
			var oIndex = oAdd.InputPorts.count;
			AddPortToICENode( oAdd+".value"+oIndex, siNodePortDataInsertionLocationAfter);
			ConnectICENodes (oAdd+".value"+(oIndex+1),oMult+".result");
		}
		
		var oSmoothStr = "Self.cls.CorrectiveSmoothCls."+oSmoothMap.Name+".Weights";
		var oGetShape = AddIceNode("GetDataNode",oTree);
		
		SetValue(oGetShape+".Reference",oSmoothStr);
		var oInputs = oCompound.InputPorts;
		var oCount = 0;

		for(a=0;a<oInputs.count;a++)
		{
			if(oInputs(a).Name.indexOf("Position")!=-1)
			{
				oCount++;
			}
		}
		
		AddExposedParamToICECompoundNode(oMult+".value", oCompound, null, "Position");
		EditExposedParamInICECompoundNode(oCompound+".Position", "Position"+cFrame, "0", "0", null, null, 0, "");
		AddExposedParamToICECompoundNode(oMult+".Factor", oCompound, null, "Factor");
		EditExposedParamInICECompoundNode(oCompound+".Factor", "Frame"+cFrame, "0", "1", null, null, 0, "");	
		
		ConnectICENodes(oCompound+".Position"+cFrame, oGetShape+".value");
		
		InspectObj(oCompound,null,null,siLock);
	}

	Application.Preferences.SetPreferenceValue("scripting.cmdlog", oCmdLog);
}