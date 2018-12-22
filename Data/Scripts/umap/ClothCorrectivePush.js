umapClothCorrectivePushMap(Selection(0))

function umapClothCorrectivePushMap(inObject)
{
	var oCmdLog = Application.Preferences.GetPreferenceValue("scripting.cmdlog");
	Application.Preferences.SetPreferenceValue("scripting.cmdlog", false);
	
	// Get Datas
	if(! inObject || inObject.type != "polymsh")
	{
		LogMessage("umapClothCorrectivePushMap ---> invalid selection",siError);
		return;
	}
	
	var oPrim = inObject.ActivePrimitive;
	
	var oRemoteControl = Dictionary.GetObject("PlayControl");
	var cFrame = parseInt(oRemoteControl.Parameters( "Current" ).Value+0.1);

	// check if cluster exists
	var oCls = oPrim.Geometry.Clusters("CorrectivePushCls");
	if(!oCls)
	{
		oCls = oPrim.Geometry.AddCluster(siVertexCluster,"CorrectivePushCls")
	}

	// check if a push map already exists for this frame
	var PushMapExist = false;
	var oPushMap;
	if(oCls.Properties("Frame"+cFrame))
	{
		PushMapExist = true;
		var buttonPressed = XSIUIToolkit.MsgBox( "A corrective push map already exists for this frame...",
			siMsgOkOnly, "Corrective Push Map" );

		if ( buttonPressed == siMsgCancel )
		{
		   LogMessage( "Store Push Map cancelled by the user!!",siInfo ) ;
		   return;
		}
	}
	else
	{
		oPushMap = oCls.AddProperty("Weight Map Property",false,"Frame"+cFrame);
	}

	// check if tree exists
	var oTree = oPrim.ICETrees.Find( "CorrectivePush" );
	if(!oTree)
	{
		oTree = ApplyOp("ICETree", inObject, siNode, null, null, siConstructionModeAnimation )(0);
		oTree.Name = "CorrectivePush";
		
		var oSet = AddICENode("SetOneDataNode", oTree);
		SetValue(oSet+".Reference", "Self.PointPosition", null);
		var oIf = AddIceNode("IfNode",oTree);

		ConnectICENodes(oTree+".port1", oIf+".Result");
		ConnectICENodes(oIf+".IfFalse", oSet+".Value");
		
		var oGet = AddICENode( "GetDataNode", oTree );
		SetValue( oGet+".Reference", "Self.PointPosition");
		var oGet1 = AddICENode( "GetDataNode", oTree );
		SetValue( oGet1+".Reference", "Self.PointNormal");
		
		var oAdd = AddICENode("AddNode", oTree);
		ConnectICENodes(oAdd+".Value1", oGet+".Value");
		
		var oAdd1 = AddIceNode("AddNode",oTree);
		var oMult = AddIceNode("MultiplyByScalarNode",oTree);
		
		ConnectICENodes(oAdd+".Value2", oMult+".Result");
		ConnectICENodes(oMult+".Value", oGet1+".Value");
		ConnectICENodes(oMult+".Factor",oAdd1+".Result");
		ConnectICENodes(oSet+".Source", oAdd+".Result");

		
		
		var oCompound = CreateICECompoundNode(oGet+","+oGet1+","+oIf+","+oAdd+","+oAdd1+","+oSet,null);
		EditICECompoundProperties(oCompound, "umapCorrectivePush", "", "", "", "", "", 1, 0, "", 4, 6732954);
		AddExposedParamToICECompoundNode(oIf+".Condition", oCompound, null, "Mute");	
	}
	
	if(!PushMapExist)
	{
		var oCompound = oTree.Nodes("umapCorrectivePush");
		var oAdd = oCompound.Nodes.Filter("AddNode")(1);
		
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
		
		var oPushStr = "Self.cls.CorrectivePushCls."+oPushMap.Name+".Weights";
		var oGetPushMap = AddIceNode("GetDataNode",oTree);
		
		SetValue(oGetPushMap+".Reference",oPushStr);
		var oInputs = oCompound.InputPorts;
		var oCount = 0;

		for(a=0;a<oInputs.count;a++)
		{
			if(oInputs(a).Name.indexOf("Position")!=-1)
			{
				oCount++;
			}
		}
		
		AddExposedParamToICECompoundNode(oMult+".value", oCompound, null, "Push");
		EditExposedParamInICECompoundNode(oCompound+".Push", "Push"+cFrame, "0", "0", null, null, 0, "");
		AddExposedParamToICECompoundNode(oMult+".Factor", oCompound, null, "Factor");
		EditExposedParamInICECompoundNode(oCompound+".Factor", "Frame"+cFrame, "0", "1", null, null, 0, "");	
		
		ConnectICENodes(oCompound+".Push"+cFrame, oGetPushMap+".Value");
		
		InspectObj(oCompound,null,null,siLock);
		SelectObj(oPushMap);
		PaintTool();
	}

	Application.Preferences.SetPreferenceValue("scripting.cmdlog", oCmdLog);
}