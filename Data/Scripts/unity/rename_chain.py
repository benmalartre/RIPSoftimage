from Utils2 import *

def RenameBone(bone,name,id):
	bone.Name = name+"_"+str(id)
	for c in bone.Children:
		RenameBone(c,name,id+1)
		
s = xsi.Selection(0)
prop = xsi.ActiveSceneRoot.AddProperty("CustomProperty",0,"Rename")
prop.AddParameter3("NewName",constants.siString)

xsi.InspectObj(prop,"","Rename Selection",constants.siModal)
name = prop.Parameters("NewName").Value


i=1
RenameBone(xsi.Selection(0),name,1)
	
	
xsi.DeleteObj(prop)