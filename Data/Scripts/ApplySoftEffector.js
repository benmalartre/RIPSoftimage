
// XSI Soft Effector Operator (c) Andrew Nicholas 2006
// www.andynicholas.com


function XSILoadPlugin( in_reg )
{
	in_reg.Author = "Andy Nicholas";
	in_reg.Name = "FXNut_SoftEffector";
	in_reg.Email = "andy@andynicholas.com";
	in_reg.URL = "www.andynicholas.com";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterCommand("ApplySoftEffector","ApplySoftEffector");
	in_reg.RegisterCommand("ApplyStretchySoftEffector","ApplyStretchySoftEffector");
	in_reg.RegisterMenu(siMenuTbCreateSkeletonID,"ApplySoftEffector_Menu",false,false);

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.");
	return true;
}

function ApplySoftEffector_Menu_Init( ctxt )
{
	var oMenu;
	oMenu = ctxt.Source;
	oMenu.AddCommandItem("Apply Soft Effector","ApplySoftEffector");
	oMenu.AddCommandItem("Apply Stretchy Soft Effector","ApplyStretchySoftEffector");
	return true;
}

function ApplySoftEffector_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "";
	oCmd.ReturnValue = true;

	return true;
}

function ApplySoftEffector_Execute()
{
	if(Selection.count==0)
	{
		Logmessage("You must select part of a bone chain",siError);
		return null;
	}
	
	var obj = Selection(0);
	if(!obj.IsClassOf(siChainElementID))
	{
		Logmessage("You must select part of a bone chain",siError);
		return null;
	}
	
	var effector = obj.Effector;
	var root = obj.Root;
	
	var effector_pos = XSIMath.CreateVector3();
	effector.kinematics.global.transform.GetTranslation(effector_pos);
	
	//Calculate total chain length and construct expression
	var expr_str=""
	
	var bones = root.Bones;
	var numbones = bones.count;
	var total_chain_len = 0;
	for(var i=0;i<numbones;++i)
	{
		total_chain_len+=bones(i).ActivePrimitive.Parameters("length").value;
		expr_str+=bones(i).fullname+".bone.length+";
	}
	expr_str = expr_str.substr(0,expr_str.length-1);
	
	//Add soft effector null
	var softeff = GetPrim("Null", "SoftEffector", root.fullname, null);
	SetValue(softeff+".null.primary_icon", 4, null);

	//Set it to the right position
	var xform = XSIMath.CreateTransform();
	xform.SetTranslation(effector_pos);
	softeff.kinematics.global.transform = xform;
	
	//Create scripted operator
	var scop = XSIFactory.CreateScriptedOp( "SoftEffector", SoftEffector_Update.toString(), "JScript" );

	//Add connections
	scop.AddOutputPort(effector.fullname+".kine.global");
	scop.AddInputPort(effector.fullname+".kine.global");
	scop.AddInputPort(root.fullname+".kine.global");
	scop.AddInputPort(softeff.fullname+".kine.global");
	
	//Add operator parameters	
	var chainlength_paramdef = XSIFactory.CreateParamDef("ChainLength", siFloat, siClassifPositioning, siAnimatable|siPersistable, "ChainLength", "Chain Length", 1, 0.0001, null, 0, 50);
	var softdistance_paramdef = XSIFactory.CreateParamDef("SoftDistance", siFloat, siClassifPositioning, siAnimatable|siPersistable, "SoftDistance", "Soft Distance", 0.4, 0.0, null, 0, 5);
	var chainlength_param = scop.AddParameter(chainlength_paramdef);
	var softdistance_param = scop.AddParameter(softdistance_paramdef);

	//Connect the operator
	scop.Connect();
	
	//Set chain length
	chainlength_param.value	= total_chain_len;

	//Also apply expression to automatically set chain length
	SetExpr(chainlength_param.fullname, expr_str, null);
	
	
	var prop = softeff.AddCustomProperty("SoftEffector");
	prop.parameters("Name").SetCapabilityFlag(siNotInspectable,true);
	var paramA = prop.AddProxyParameter(chainlength_param, "ChainLength", "Chain Length");
	var paramB = prop.AddParameter2("SoftDistance", siFloat, 0.4, 0.0, null, 0.0, 5.0, siClassifPositioning, siAnimatable|siPersistable|siKeyable);
	SetExpr(softdistance_param.fullname, paramB.fullname, null);

	InspectObj(prop, "", "Soft Effector");
	
	return true;
}


function SoftEffector_Update(In_UpdateContext, Out, Inglobal, rootGlobal, targetGlobal)
{
	var obj = In_UpdateContext.UserData;
	if(obj==null)
	{
		obj = new Object();
		obj.rootpos = XSIMath.CreateVector3();
		obj.effpos = XSIMath.CreateVector3();
		obj.targetpos = XSIMath.CreateVector3();
		obj.diffpos = XSIMath.CreateVector3();
		obj.xform = XSIMath.CreateTransform();
		In_UpdateContext.UserData=obj;
	}
	
	var rootpos = obj.rootpos;
	var effpos = obj.effpos;
	var targetpos = obj.targetpos;
	var diffpos = obj.diffpos;
	var xform = obj.xform;
	
	var chainlen = In_UpdateContext.operator.ChainLength.value;
	var softdist = In_UpdateContext.operator.SoftDistance.value;
	
	rootGlobal.value.transform.GetTranslation(rootpos);
	Inglobal.value.transform.GetTranslation(effpos);
	targetGlobal.value.transform.GetTranslation(targetpos);
	
	
	diffpos.Sub(targetpos, rootpos);
	var curlen = diffpos.Length();
	
	xform.Copy(Inglobal.value.transform);
	
	var diff = curlen - (chainlen-softdist);
	if(diff>0 && softdist>=0.00001 )
	{
		diffpos.ScaleInPlace(1.0/curlen);
	
		var da = chainlen - softdist;
		var db = chainlen;
	
		var newlen = softdist*(1.0 - Math.exp(-(curlen-da)/softdist)) + da;
	
		diffpos.ScaleInPlace(newlen);
	
		targetpos.Add(diffpos,rootpos);
	}
	xform.SetTranslation(targetpos);
	Out.value.transform = xform;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

function ApplyStretchySoftEffector_Execute()
{
	if(Selection.count==0)
	{
		Logmessage("You must select part of a bone chain",siError);
		return null;
	}
	
	var obj = Selection(0);
	if(!obj.IsClassOf(siChainElementID))
	{
		Logmessage("You must select part of a bone chain",siError);
		return null;
	}
	
	var autoinspect = GetValue("preferences.Interaction.autoinspect");
	SetValue("preferences.Interaction.autoinspect", false, null);

	var effector = obj.Effector;
	var root = obj.Root;
	
	var effector_pos = XSIMath.CreateVector3();
	effector.kinematics.global.transform.GetTranslation(effector_pos);
	
	//Add soft effector null
	var softeff = GetPrim("Null", "StretchySoftEffector", root.fullname, null);
	SetValue(softeff+".null.primary_icon", 4, null);

	//Set it to the right position
	var xform = XSIMath.CreateTransform();
	xform.SetTranslation(effector_pos);
	softeff.kinematics.global.transform = xform;
	
	
	//Calculate total chain length
	var bones = root.Bones;
	var numbones = bones.count;
	var total_chain_len = 0;
	for(var i=0;i<numbones;++i)
	{
		total_chain_len+=bones(i).ActivePrimitive.Parameters("length").value;		
	}
	
	var prop = softeff.AddCustomProperty("StretchySoftEffector");
	prop.parameters("Name").SetCapabilityFlag(siNotInspectable,true);
	var prop_BoneScale_param = prop.AddParameter2("BoneScale", siFloat, 1.0, 0.0, null, 0.0, 5.0, siClassifPositioning, siAnimatable|siPersistable);
	var prop_RestChainLength_param = prop.AddParameter2("RestChainLength", siFloat, 4, 0.0, null, 0.0, 50.0, siClassifPositioning, siAnimatable|siPersistable);
	var prop_SoftDistance_param = prop.AddParameter2("SoftDistance", siFloat, 0.4, 0.0, null, 0.0, 5.0, siClassifPositioning, siAnimatable|siPersistable|siKeyable);

	
	//Create scripted operator for the bone scale
	var scop = XSIFactory.CreateScriptedOp( "StretchySoftEffector", StretchySoftEffector_Update.toString(), "JScript" );

	//Add connections
	scop.AddOutputPort(prop_BoneScale_param.fullname);
	scop.AddInputPort(softeff.fullname+".kine.global");
	scop.AddInputPort(root.fullname+".kine.global");
	
	//Add operator parameters	
	var chainlength_paramdef = XSIFactory.CreateParamDef("RestChainLength", siFloat, siClassifPositioning, siAnimatable|siPersistable, "RestChainLength", "Rest Chain Length", 1, 0.0001, null, 0, 50);
	var softdistance_paramdef = XSIFactory.CreateParamDef("SoftDistance", siFloat, siClassifPositioning, siAnimatable|siPersistable, "SoftDistance", "Soft Distance", 0.4, 0.0, null, 0, 5);
	var chainlength_param = scop.AddParameter(chainlength_paramdef);
	var softdistance_param = scop.AddParameter(softdistance_paramdef);

	//Connect the operator
	scop.Connect();
	scop.SetCapabilityFlag(siNotInspectable,true);
	
	var expr_col = SetExpr(chainlength_param.fullname, prop_RestChainLength_param.fullname, null);
	expr_col = expr_col.Item(0);
	expr_col(0).SetCapabilityFlag(siNotInspectable,true);
	
	expr_col = SetExpr(softdistance_param.fullname, prop_SoftDistance_param.fullname, null);
	expr_col = expr_col.Item(0);
	expr_col(0).SetCapabilityFlag(siNotInspectable,true);
		
	//Set chain length
	chainlength_param.value	= total_chain_len;	
	
	var expr_str=""
	for(var i=0;i<numbones;++i)
	{
		var cur_bone = bones(i);
		var cur_bonelen = cur_bone.ActivePrimitive.Parameters("length").value;

		var param_bone = prop.AddParameter2(cur_bone.name+"_RestLength", siFloat, 0.4, 0.0, null, 0.0, 5.0, siClassifPositioning, siAnimatable|siPersistable|siKeyable);
		param_bone.value = cur_bonelen;
		
		expr_str+=param_bone.fullname+"+";		
		
		expr_col = SetExpr(cur_bone.fullname+".bone.length", param_bone.fullname+" * "+prop_BoneScale_param.fullname, null);
		expr_col = expr_col.Item(0);
		expr_col(0).SetCapabilityFlag(siNotInspectable,true);
	}
	
	//Set the expression for the total bone length
	expr_str = expr_str.substr(0,expr_str.length-1);
	var expr_col = SetExpr(prop_RestChainLength_param.fullname, expr_str, null);
	var expr_col = expr_col.Item(0);
	expr_col(0).SetCapabilityFlag(siNotInspectable,true);

	
	ApplyCns("Position", effector.fullname, softeff.fullname, null);
	
	SetValue("preferences.Interaction.autoinspect", autoinspect, null);
	
	InspectObj(prop, "", "Stretchy Soft Effector");
	return true;
}


function StretchySoftEffector_Update(In_UpdateContext, Out, rootGlobal, effGlobal)
{
	var obj = In_UpdateContext.UserData;
	if(obj==null)
	{
		obj = new Object();
		obj.rootpos = XSIMath.CreateVector3();
		obj.effpos = XSIMath.CreateVector3();
		In_UpdateContext.UserData=obj;
	}
	var rootpos = obj.rootpos;
	var effpos = obj.effpos;
	
	var softdist = In_UpdateContext.operator.SoftDistance.value;
	var chainlen = In_UpdateContext.operator.RestChainLength.value;
	rootGlobal.value.transform.GetTranslation(rootpos);
	effGlobal.value.transform.GetTranslation(effpos);
	
	rootpos.SubInPlace(effpos);
	
	var dist = rootpos.length();
	var da = chainlen-softdist;
	var scale = 1.0;
	if(dist>da)
	{
		var shortd = softdist *(1.0 - Math.exp(-(dist - da)/softdist))+da;
		scale = dist/shortd;
	}
	
	Out.Value = scale;

}