umapClothCorrectiveShape(Selection(0));

function umapClothCorrectiveShape(inObject)
{
	var oCmdLog = Application.Preferences.GetPreferenceValue("scripting.cmdlog");
	Application.Preferences.SetPreferenceValue("scripting.cmdlog", false);
	
	// Get Datas
	if(! inObject || inObject.type != "polymsh")
	{
		LogMessage("umapClothCorrectiveShape ---> invalid selection",siError);
		return;
	}
	
	var oPrim = inObject.ActivePrimitive;
	
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
		var oBasePoints = oPrim.GetGeometry2( cFrame, siConstructionModeAnimation).Points;
		var oSecondPoints = oPrim.GetGeometry2( cFrame, siConstructionModeSecondaryShape).Points;
	}
	
	else
	{
		var oBasePoints = oPrim.GetGeometry2( cFrame, siConstructionModeAnimation).Points;
		var oSecondPoints = oPrim.GetGeometry2( cFrame, siConstructionModeSecondaryShape).Points;
	}
	
	var oBasePos = oBasePoints.PositionArray.toArray();
	var oSecondPos = oSecondPoints.PositionArray.toArray();

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
		oTree = ApplyOp("ICETree", inObject, siNode, null, null, siConstructionModeSecondaryShape )(0);
		oTree.Name = "CorrectiveShapeTree";
		
		var oSet = AddICENode("SetOneDataNode", oTree);
		SetValue(oSet+".Reference", "Self.PointPosition", null);
		var oIf = AddIceNode("IfNode",oTree);
		ConnectICENodes(oTree+".port1", oIf+".Result");
		ConnectICENodes(oIf+".IfFalse", oSet+".Value");
		
		var oGet = AddICENode( "GetDataNode", oTree );
		SetValue( oGet+".Reference", "Self.PointPosition");
		var oAdd = AddIceNode("AddNode",oTree);
		
		
		ConnectIceNodes(oAdd+".Value1",oGet+".Value");
		ConnectIceNodes(oSet+".Source",oAdd+".Result");
		
		var oCompound = CreateICECompoundNode(oGet+","+oSet+","+oAdd+","+oIf,null);
		EditICECompoundProperties(oCompound, "umapCorrectiveShape", "", "", "", "", "", 1, 0, "", 4, 6732954);
		AddExposedParamToICECompoundNode(oIf+".Condition", oCompound, null, "Mute");
	}
	
	if(!ShapeExist)
	{
		var oCompound = oTree.Nodes("umapCorrectiveShape");
		var oAdd = oCompound.Nodes.Filter("AddNode")(0);
		
		var oMult = AddIceNode("MultiplyByScalarNode",oCompound);
		
		if(oAdd.InputPorts.count == 2 && !oAdd.InputPorts(1).IsConnected)
		{
			ConnectIceNodes(oAdd+".Value2",oMult+".Result");
		}
		else
		{
			var oIndex = oAdd.InputPorts.count;
			AddPortToICENode( oAdd+".value"+oIndex, siNodePortDataInsertionLocationAfter);
			ConnectICENodes (oAdd+".value"+(oIndex+1),oMult+".result");
		}
		
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
		
		AddExposedParamToICECompoundNode(oMult+".value", oCompound, null, "Position");
		EditExposedParamInICECompoundNode(oCompound+".Position", "Position"+cFrame, "0", "0", null, null, 0, "");
		AddExposedParamToICECompoundNode(oMult+".Factor", oCompound, null, "Factor");
		EditExposedParamInICECompoundNode(oCompound+".Factor", "Frame"+cFrame, "0", "1", null, null, 0, "");	
		
		ConnectICENodes(oCompound+".Position"+cFrame, oGetShape+".value");
		
		InspectObj(oCompound,null,null,siLock);
	}

	Application.Preferences.SetPreferenceValue("scripting.cmdlog", oCmdLog);
}