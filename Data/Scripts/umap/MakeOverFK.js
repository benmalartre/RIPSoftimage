var oSel = Selection;
var oParent = ActiveSceneRoot;
for(a=0;a<oSel.Count;a++)
{
	var oFK = oParent.AddNull(oSel(a).Name+"_FK");
	oFK.Kinematics.Global.Transform = oSel(a).Kinematics.Global.Transform;
	oSel(a).Kinematics.AddConstraint("Pose",oFK,false);
	oParent = oFK;
}