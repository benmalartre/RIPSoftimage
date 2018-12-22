// Set Vertex Weight
//---------------------------------------
var oSubComponent = Selection(0).SubComponent;
var oSubCollection = oSubComponent.ComponentCollection;
var oMesh = oSubComponent.Parent3DObject;

var oCustomWeight = 1;
var oUI = setVertexWeightUI(oMesh)

var oWeightMap = oUI.WeightMap;
var oCustomWeight = oUI.CustomWeight;
var oParentCls = oWeightMap.Parent;
var oClsElements = oParentCls.Elements;

var oElements = oWeightMap.Elements;
oElementsArray = oElements.Array.toArray();

var oIndex, oItem;
for(x=0;x<oSubCollection.count;x++)
{
	oIndex = oSubCollection(x).Index;
	oItem = oClsElements.FindIndex(oIndex);
	oElementsArray[oItem] = oCustomWeight;
}

oElements.Array = oElementsArray;

// Set Vertex Weight UI
//--------------------------------
function setVertexWeightUI(inObj)
{
	var WeightMapList = getWeightMapList(inObj);
	
	var oUI = ActiveSceneRoot.AddProperty("Custom_parameter_list",0,"SetVertexWeight");
	var oWM = oUI.AddParameter3("WeightMaps",siString);
	var oWeight = oUI.AddParameter3("CustomWeight",siFloat,1,0,1);
	
	var oItem;
	var layout = oUI.PPGLayout;
	layout.Clear();
	
	layout.AddGroup("Select a WeightMap ");
		oItem = layout.AddEnumControl("WeightMaps",WeightMapList,"Weight Maps",siControlListBox);
		oItem.SetAttribute(siUINoLabel,true);
	layout.EndGroup();
	layout.AddItem( "CustomWeight");
		
	bCancelled  = InspectObj(oUI,null,"Set Vertex Weight",siModal,false);	

	if ( !bCancelled )
	{
		var outObj = new Object();
		outObj.WeightMap = Dictionary.GetObject(oWM.value);
		outObj.CustomWeight = oWeight.value;
		
		DeleteObj(oUI);
		return outObj;
	}
	else
	{
		DeleteObj(oUI);
		logmessage("Set Vertex Weight Aborted..");
	}

}

// Get Weight Map List
//--------------------------------
function getWeightMapList(inObj)
{
	var oCls = inObj.ActivePrimitive.Geometry.Clusters.Filter(siVertexCluster);
	var oWeightMapList = new Array();
	var oFullName, oSplittedName, oSmartName;
	var l;

	for(a=0;a<oCls.count;a++)
	{
		var oMaps = oCls(a).Properties.Filter(siWgtMapType)
		for(b=0;b<oMaps.count;b++)
		{
			oFullName = oMaps(b).FullName;
			oSplittedName = oFullName.split(".");
			l = oSplittedName.length;
			oSmartName = oSplittedName[l-2]+"."+oSplittedName[l-1];
			
			oWeightMapList.push(oSmartName);
			oWeightMapList.push(oMaps(b).FullName);
		}
	}
	return oWeightMapList;
}







