from Utils2 import *

source = xsi.ActiveSceneRoot.FindChild("Perso_Low")
target = xsi.ActiveSceneRoot.FindChild("Perso_Rig")
sg = source.Groups("Controls")
tg = target.Groups("Controls")

def FindInGroup(grp,name):
	for m in grp.Members:
		if m.Name == name:
			return m
	return None
	
for sm in sg.Members:
	#xsi.LogMessage("Source : "+sm.FullName)
	tm = FindInGroup(tg,sm.Name)
	if tm:
		#xsi.LogMessage("Target : "+tm.FullName)
		tm.Kinematics.AddConstraint("Pose",sm)