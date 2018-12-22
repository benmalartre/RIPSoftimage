from Utils2 import *

xsi.SelectChildNodes()
todelete = []

for s in xsi.Selection:
	for p in s.Properties:
		if p.Name.find("Custom_Parameter_Set")>-1:
			todelete.append(p)
		
xsi.DeleteObj(todelete)