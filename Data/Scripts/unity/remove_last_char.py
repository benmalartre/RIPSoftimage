from Utils2 import *

xsi.SelectChildNodes()
for s in xsi.Selection:
	#n = s.Name.replace("L_","R_")
	s.Name = s.Name[:-1]