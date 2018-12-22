var oSel = Selection;
var oClothObjects = XSIFactory.CreateActiveXObject("XSI.Collection");
for(a=0;a<oSel.count;a++)oClothObjects.Add(oSel(a));

TransferClothObjectUVs(oClothObjects);


function TransferClothObjectUVs(inClothObjects)
{
	var oCmdLog = Application.Preferences.GetPreferenceValue("scripting.cmdlog");
	Application.Preferences.SetPreferenceValue("scripting.cmdlog", false);
	
	var oModel = inClothObjects(0).Model;
	var aSplit = oModel.Name.split("_");

	var sModelType = aSplit[0];
	var sModelName = aSplit[1];

	var sLibraryBranch = "";
	var sModelSubFolder = "";

	if(sModelType == "ch")sLibraryBranch = "Characters";
	else if(sModelType == "pr")sLibraryBranch = "Props";

	var sModelFullName = sModelType+"_"+sModelName+"_shd";

	var sEdamLibrary = "P:\\longs-metrages\\umap\\umap_edam\\LIBRARY";
	var sShadingModelPath = XSIUtils.BuildPath(sEdamLibrary,sLibraryBranch,sModelName,"Models","shading",sModelFullName,sModelFullName+"_Ref.emdl");
	var oImageClip;

	var fso = XSIFactory.CreateActiveXObject( "Scripting.FileSystemObject" );
	if(fso.FileExists(sShadingModelPath))
	{
		var oShadingModel = ImportModel( sShadingModelPath)(1);
		var sCheckerFile = "P:\\longs-metrages\\umap\\wip\\_rigging\\maps\\Damier.png";
		oImageClip = CreateImageClip2( sCheckerFile , "Checker" )(0);
	}
	else
	{
		LogMessage("Can't find corresponding shading model for "+oModel+" ---> Transfer UV aborted...");
		return;
	}

	for(var a=0;a<inClothObjects.count;a++)
	{
		var oClothObject = inClothObjects(a);
		var sClothObjectName = oClothObject.Name;
		var oClothShadingObject = oShadingModel.FindChild(sClothObjectName);
		var oShadingUVs = oClothShadingObject.Material.CurrentUV;
		
		CreateTextureSupport( oClothObject, siTxtUV, siTxtDefaultSpherical, "Texture_Support" );
		
		var oMaterial = oClothObject.Material;
		ConnectCheckerMap(oMaterial, oImageClip);

		var oClothUVs = oClothObject.Material.CurrentUV;
		CopyUVW( oShadingUVs );
		PasteUVW( oClothUVs );
		FreezeObj(oClothUVs);
	}
	
	DeleteObj("B:"+oShadingModel);
	Application.Preferences.SetPreferenceValue("scripting.cmdlog", oCmdLog);
}

function ConnectCheckerMap(inMaterial,inImageClip)
{
	var oShader = inMaterial.Parameters("Surface").Source;
	var oDiffuse = oShader.Parameters("Diffuse");
	var oAmbient = oShader.Parameters("Ambient");
	
	oDiffuse.Connect(inImageClip);
	oAmbient.Connect(inImageClip);
}