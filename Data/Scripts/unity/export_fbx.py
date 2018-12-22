from Utils2 import *

model = xsi.ActiveSceneRoot.FindChild("Perso_Rig")
#path = XSIFactory.FilePathRequester
if model:
	
	FBXExportCameras(0)
	FBXExportLights(0)
	FBXExportGeometries(1)
	FBXExportSelection(1)
	FBXExport(path)


