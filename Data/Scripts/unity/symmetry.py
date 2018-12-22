from Utils2 import *
xsi.SelectChildNodes()

		
model = xsi.Selection(0).Model
todelete = []
for s in xsi.Selection:
	sym = model.FindChild(s.Name.replace("R_","L_"))
	if sym:
		cns = s.Kinematics.AddConstraint("Symmetry",sym)
		todelete.append(cns)
	
xsi.DeleteObj(todelete)