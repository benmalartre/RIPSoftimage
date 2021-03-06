// Secondary Shape Modeling
//--------------------------------------
umapStoreSecondaryShape();

function umapStoreSecondaryShape()
{
	var oCmdLog = Application.Preferences.GetPreferenceValue("scripting.cmdlog");
	Application.Preferences.SetPreferenceValue("scripting.cmdlog", false);
	
	// Get Datas
	var oObj = Selection(0);
	var oPrim = oObj.ActivePrimitive;
	
	var oRemoteControl = Dictionary.GetObject("PlayControl");
	var cFrame = parseInt(oRemoteControl.Parameters( "Current" ).Value+0.1);

	// check if cluster exists
	var oCls = oPrim.Geometry.Clusters("CorrectiveShapeCls");
	if(!oCls)
	{
		oCls = oPrim.Geometry.AddCluster(siVertexCluster,"CorrectiveShapeCls")
	}

	// check if a shape already exists for this frame
	var ShapeExist = false;
	if(oCls.Properties("Frame"+cFrame))
	{
		ShapeExist = true;
		var buttonPressed = XSIUIToolkit.MsgBox( "A corrective shape already exists for this frame\n Would you replace it??",
			siMsgOkCancel , "Corrective Shape" );

		if ( buttonPressed == siMsgCancel )
		{
		   LogMessage( "Store Secondary Shape cancelled by the user!!",siInfo ) ;
		   return;
		}
	}

	// Create shape
	var oTree = oPrim.ICETrees.Find( "CorrectiveShapeTree" );
	if(oTree)
	{
		var oCompound = oTree.Nodes("umapCorrectiveShape");
		var oMuteValue = oCompound.InputPorts("Mute").Value;
		oCompound.InputPorts("Mute").Value = true;
		var oBaseGeom = oPrim.GetGeometry2( cFrame, siConstructionModeAnimation);
		oCompound.InputPorts("Mute").Value = oMuteValue;
	}
	
	else
	{
		var oBaseGeom = oPrim.GetGeometry2( cFrame, siConstructionModeAnimation);
	}
	
	var oSecondGeom = oPrim.GetGeometry2( cFrame, siConstructionModeSecondaryShape);

	var oBasePos = oBaseGeom.Points.PositionArray.toArray();
	var oSecondPos = oSecondGeom.Points.PositionArray.toArray();

	var l = oBasePos.length;

	var oDelta = new Array(l);

	for(a=0;a<l;a++)
	{
		oDelta[a] = oSecondPos[a]-oBasePos[a];
	}
	
	if(!ShapeExist)
	{
		var oShapeNode = oCls.AddProperty("SimpleDeformShape",0,"Frame"+cFrame);
		oShapeNode.Elements.Array = oDelta;
	}
	else
	{
		var oShapeNode = oCls.Properties("Frame"+cFrame);
		/*
		var oElems = oShapeNode.Elements.Array.toArray();
		for(a=0;a<oElems.length;a++)
		{
			oDelta[a] -= oElems[a];
		}
		*/
		oShapeNode.Elements.Array = oDelta;
	}

	// check if tree exists
	var oTree = oPrim.ICETrees.Find( "CorrectiveShapeTree" );
	if(!oTree)
	{
		oTree = ApplyOp("ICETree", oObj, siNode, null, null, siConstructionModeAnimation )(0);
		oTree.Name = "CorrectiveShapeTree";
		
		var oSet = AddICENode("SetOneDataNode", oTree);
		SetValue(oSet+".Reference", "Self.PointPosition", null);
		var oIf = AddIceNode("IfNode",oTree);
		ConnectICENodes(oTree+".port1", oIf+".Result");
		ConnectICENodes(oIf+".IfFalse", oSet+".Value");
		
		var oGet = AddICENode( "GetDataNode", oTree );
		SetValue( oGet+".Reference", "Self.PointPosition");
		var oAdd = AddIceNode("AddNode",oTree);
		var oMult = AddIceNode("MultiplyByScalarNode",oTree);
		ConnectIceNodes(oAdd+".Value1",oGet+".Value");
		ConnectIceNodes(oAdd+".Value2",oMult+".Result");
		ConnectIceNodes(oSet+".Source",oAdd+".Result");
		
		var oCompound = CreateICECompoundNode(oMult+","+oGet+","+oSet+","+oAdd+","+oIf,null);
		EditICECompoundProperties(oCompound, "umapCorrectiveShape", "", "", "", "", "", 1, 0, "", 4, 6732954);
		AddExposedParamToICECompoundNode(oIf+".Condition", oCompound, null, "Mute");
		AddExposedParamToICECompoundNode(oMult+".value", oCompound, null, "Position");
		EditExposedParamInICECompoundNode(oCompound+".Position", "Position", "0", "0", null, null, 3, "");
		AddExposedParamToICECompoundNode(oMult+".Factor", oCompound, null, "Factor");
		EditExposedParamInICECompoundNode(oCompound+".Factor", "Factor", "0", "1", null, null, 3, "");	
	}
	
	if(!ShapeExist)
	{
		var oCompound = oTree.Nodes("umapCorrectiveShape");

		var oShapeStr = "Self.cls.CorrectiveShapeCls."+oShapeNode.Name+".positions";
		
		var oGetShape = AddIceNode("GetDataNode",oTree);
		
		SetValue(oGetShape+".Reference",oShapeStr);
		var oInputs = oCompound.InputPorts;
		var oCount = 0;

		for(a=0;a<oInputs.count;a++)
		{
			if(oInputs(a).Name.indexOf("Position")!=-1)
			{
				oCount++;
			}
		}
		var IsConnected = oCompound.InputPorts("Position").IsConnected;
		
		if(oCount == 1 && IsConnected == false)
		{
			ConnectICENodes(oCompound+".Position", oGetShape+".value");
		}
		
		else
		{
			if(oCount == 1)
			{
				AddPortToICENode(oCompound+".Position", siNodePortDataInsertionLocationAfter);
				ConnectICENodes(oCompound+".Position1", oGetShape+".value");
			}
			else
			{
				var oLast = oCount - 1;
				AddPortToICENode(oCompound+".Position"+oLast, siNodePortDataInsertionLocationAfter);
				ConnectICENodes(oCompound+".Position"+oCount, oGetShape+".value");
			}
		}
		
		oInputs = oCompound.InputPorts;
		var oLastFactorInput;
		
		for(a=0;a<oInputs.count;a++)
		{
			if(oInputs(a).Name.indexOf("Factor")!=-1)
			{
				oLastFactorInput = oInputs(a);
			}
		}
		
		// Get Property
		var oProp = oObj.Properties("CorrectiveShape");
		if(!oProp)oProp = oObj.AddCustomProperty("CorrectiveShape",false);
		
		//oProp.AddProxyParameter(oLastFactorInput, "Frame"+cFrame, "Frame"+cFrame );
		InspectObj(oCompound,null,null,siLock);
	}

	Application.Preferences.SetPreferenceValue("scripting.cmdlog", oCmdLog);
}

