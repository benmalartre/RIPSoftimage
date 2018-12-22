from Utils2 import *

roots = []
for s in xsi.Selection:
	roots.append(s)

parent = PickElement(constants.siGenericObjectFilter,"Pick Parent")
grp = GroupSetup(parent.Model,None,"Envelopes")
for r in roots:
	last = parent
	bones = r.Bones
	for b in bones:
		n = AddNull(last,1,1,b.Name+"_N")
		MatchTransform(n,b)
		n.Kinematics.AddConstraint("Pose",b)
		grp.AddMember(n)
		last = n
		
