var oFrameNumber = "Frame86";

var oTree = Selection(0).ActivePrimitive.ICETrees("CorrectiveShape");
var oGetDataNodes = oTree.Nodes.Filter("SceneReferenceNode");

//LogMessage(oGetDataNodes(0).Reference.Value);

var oCompound = oTree.Nodes("umapCorrectiveShape");
var oCorrectiveShapeCompound = oTree.CompoundNodes("umapCorrectiveShape");

for(n=0;n<oGetDataNodes.count;n++)
{
	if(oGetDataNodes(n).Reference.Value.indexOf(oFrameNumber)!=-1)
	{		
		DeleteObj(oGetDataNodes(n));
	}
}

// Delete Corresponding MultiplyByScalarNode
//------------------------------------------
var oMultNodes = oCompound.Nodes.Filter("MultiplyByScalarNode");
for(n=0;n<oMultNodes.count;n++)
{
	var oInputs = oMultNodes(n).InputPorts;
	
	if(!oInputs(0).IsConnected)
	{		
		DeleteObj(oMultNodes(n));
	}
}

// Delete unused AddNode input port
//------------------------------------------
var oAddNode = oCompound.Nodes.Filter("AddNode")(0);
var oInputs = oAddNode.InputPorts;
for(i=0;i<oInputs.count;i++)
{
	if(!oInputs(i).IsConnected)
	{
		RemovePortFromICENode(oInputs(i).FullName);
	}
}


