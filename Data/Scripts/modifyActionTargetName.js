//modify action sources name
//-----------------------------------

var oSource = selection(0);
var oSourceItems = oSource.SourceItems;
var toReplace = ["R_Lip_Ctrl","R_Lip_Wave_Ctrl","R_Middle_Lip_Ctrl","R_Low_Middle_Ctrl","M_Up_Lip_Ctrl",
					"M_Low_Lip_Ctrl","L_Middle_Lip_Ctrl","L_Lip_Ctrl","L_Low_Middle_Ctrl"];
var replaceBy = ["R_Lip_Corner","R_Lip_Wave","R_Lip_Up","R_Lip_Low","M_Lip_Up",
					"M_Lip_Low","L_Lip_Up","L_Lip_Corner","L_Lip_Low"];

for(p=0;p<oSourceItems.count;p++)
{
	for(x=0;x<toReplace.length;x++)
	{
		var newTargetName = oSourceItems(p).Target.replace(toReplace[x],replaceBy[x]);
		oSourceItems(p).Target = newTargetName;
	}
} 
 
