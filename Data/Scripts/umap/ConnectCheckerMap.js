var sCheckerFile = "P:\\longs-metrages\\umap\\wip\\_rigging\\maps\\Damier.png";
var oImageClip = CreateImageClip2( sCheckerFile , "Checker" )(0);

for(s=0;s<Selection.count;s++)
{
	var oMaterial = Selection(s).Material;
	ConnectCheckerMap(oMaterial, oImageClip);
}


function ConnectCheckerMap(inMaterial,inImageClip)
{
	
	var oShader = oMaterial.Parameters("Surface").Source;
	var oDiffuse = oShader.Parameters("Diffuse");
	var oAmbient = oShader.Parameters("Ambient");
	
	oDiffuse.Connect(inImageClip);
	oAmbient.Connect(inImageClip);
}
