from Utils2 import *

def AddBuffer(slave,master):
	buffer = AddNull(master,0,0,slave.Name+"_Buf")
	MatchTransform(buffer,slave)
	slave.Kinematics.AddConstraint("Pose",buffer)
	GroupSetup(buffer.Model,[buffer],"Buffers")
	
def AddControlNull(parent,bone,grp):
	n = AddNull(parent,2,0.05,bone.Name)
	MatchPosition(n,bone)
	MatchRotation(n,bone)
	AddBuffer(bone,n)
	for c in bone.Children:
		m = AddControlNull(n,c,grp)
		
		
model = xsi.ActiveSceneRoot.Addmodel(None,"Rig")
group  = GroupSetup(model,None,"Controls")

roots = []
for s in xsi.Selection:
	roots.append(s)
parent = PickElement()	
for r in roots:
	AddControlNull(parent,r,group)