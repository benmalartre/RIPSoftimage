from Utils2 import *

slaves = xsi.Selection
master = PickElement(constants.siGenericObjectFilter,"Pick Master")
rig = master.Model
grp = GroupSetup(rig,None,"Buffers")
for slave in slaves:
	buffer = AddNull(master,0,0,slave.Name+"_Buf")
	MatchTransform(buffer,slave)
	slave.Kinematics.AddConstraint("Pose",buffer)