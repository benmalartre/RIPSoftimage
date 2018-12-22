var oObj = Selection(0);
var oSyflexOp = oObj.ActivePrimitive.ConstructionHistory.Find("syCloth");

var oNested = oSyflexOp.NestedObjects;
var oCollide = XSIFactory.CreateActiveXObject("XSI.Collection");

for(a=0;a<oNested.count;a++)
{
	if(oNested(a).Type == "syCollide")
	{
		oCollide.Add(oNested(a));
		var oInputs = oNested(a).InputPorts;
		for(b=0;b<oInputs.count;b++)
		{
			LogMessage(oInputs(b).FullName);
		}
	}
}



