from Utils2 import *


def AddBuffer(slave, master):
	null = AddNull(master, 0, 0, slave.Name+"_Buf")
	MatchTransform(null, slave)
	slave.Kinematics.AddConstraint("Pose", null)
	GroupSetup(null.Model, [null], "Buffers")


def AddControlNull(parent, bone, grp):
	n = AddNull(parent, 2, 0.05, bone.Name)
	MatchPosition(n, bone)
	MatchRotation(n, bone)
	AddBuffer(bone, n)
	for c in bone.Children:
		AddControlNull(n, c, grp)
		
		
model = xsi.ActiveSceneRoot.Addmodel(None, "Rig")
group = GroupSetup(model, None, "Controls")

roots = []
for s in xsi.Selection:
	roots.append(s)
parent = PickElement()	
for r in roots:
	AddControlNull(parent, r, group)
