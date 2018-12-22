// Create shape
var A = Selection(0);
var B = Selection(1);
var aGeom = A.ActivePrimitive.Geometry;
var bGeom = B.ActivePrimitive.Geometry;

var aCls = aGeom.Clusters("ShapeCls");

var oBasePos = aGeom.Points.PositionArray.toArray();
var oSecondPos = bGeom.Points.PositionArray.toArray();

var l = oBasePos.length;

var oDelta = new Array(l);

for(a=0;a<l;a++)
{
	oDelta[a] = oSecondPos[a]-oBasePos[a];
}

var oShapeNode = aCls.AddProperty("SimpleDeformShape",0,Selection(1).Name);
oShapeNode.Elements.Array = oDelta;

