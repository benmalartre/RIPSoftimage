from Utils2 import *
model = Dispatch(xsi.Selection(0))

delta = Application.Dictionary.getObject(model.FullName+".Delta")

fcurves = delta.ActionDeltas("StoredFCurves");
for i in fcurves.Items:	
	if i.Name.find("_N")>-1:
		i.Name = i.Name.replace("_N","")