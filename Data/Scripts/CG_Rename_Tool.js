
/*===========================================================
 * CG_RenameTool
 *
 * Cyril GIBAUD 2006
 * Email cyril_gib@hotmail.com
 * URL www.studionham.com
 * Version 2.0 : Added the renumbering feature
  * Version 1.6 : Added ignored prefix/suffix feature
 * Version 1.4 : Added duplicate/rename feature
 * Version 1.0 
 
Concept :
Every CG artist will need a rename tool sooner or later.Previously, some good tools were accessible in the NetView to do
that. Since XSI 7, I can't find these tools anymore. So I took some time to enhance one of my first PPG plugin in XSI,
a simple renamer.
I added a real-time preview on the selected objects.

Usage :
From the Animate -> Create => Skeleton toolbar, select "CG_RenameTool".
From the UI you will be able to search&replace strings, remove any number of first and last characters and add a prefix
and\or a suffix. While your setting the values, you can see the preview showing what will happen. You can click "Next"
to see the result on the other objects.

V 1.4 : You can now duplicate a selection or a branch and perform a rename at the same time, so the duplicated objects don't get invalid names. "L_Arm" for instance will become "L_Arm1" if you use XSI's duplicate, but if you perform a search & replace "L" by "R_" at the same time you will get a nice "R_Arm".

V 2.0 : You can now renumber your objects with the padding you want. Objects are numbered in the order of selection. In case you choose "Keep", it will keep the numbering as it is but just change the padding. Ignoring suffixes (Advanced panel) can be usefull if the number is not in the very end of your objet name ("Hair01_geo").

At last remember you can use the commands used by the property to rename your objects in your own scripts. They have a lot of arguments, but you don't have to provide them all.
 
CG_Rename(inColl[Collection handler], inSearch, inReplace, inPrefix, inSuffix, inRemoveFirst, inRemoveLast, inNumbering, inPadding, IgnoredPrefix, IgnoredSuffix)
CG_DuplicateRename(inColl[Collection handler], inSearch, inReplace, inPrefix, inSuffix, inRemoveFirst, inRemoveLast, inMode, inReassignConstraints,  inReassignExpressions)

Known Bugs and limitations :
I hope I'm capable of avoiding bugs on a so simple plugin...but pobody's nerfect.

Possible Updates :
I'll maybe add the ability to compute several "Search&Replace" at a time.
A multiline TextBox to show all the previews could be a good idea as well.

==============================================================*/

//[of]:Plugin Declaration
function XSILoadPlugin( in_reg )
{
	in_reg.Author = "Cyril GIBAUD";
	in_reg.Name = "CG_RenameTool";
	in_reg.Email = "";
	in_reg.URL = "";
	in_reg.Major = 1;
	in_reg.Minor = 0;

	in_reg.RegisterCommand("CG_RenameTool","CG_RenameTool");
	in_reg.RegisterProperty("CG_RenameProp");
	in_reg.RegisterCommand("CG_Rename","CG_Rename");
	in_reg.RegisterCommand("CG_DuplicateRename","CG_DuplicateRename");
	
	in_reg.RegisterMenu( siMenuTbCreateSkeletonID, "CG_RenameTool_Menu", false, false);

	return true;
}

function XSIUnloadPlugin( in_reg )
{
	strPluginName = in_reg.Name;
	Application.LogMessage(strPluginName + " has been unloaded.");
	return true;
}
//[cf]

//[of]:Menu
function CG_RenameTool_Menu_Init( in_ctxt )
{
	var oMenu = in_ctxt.source;
  oMenu.AddCommandItem("&CG_RenameTool", "CG_RenameTool");
}
//[cf]

//[of]:CG_RenameTool (Command)
function CG_RenameTool_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "Need details ?";
	oCmd.ReturnValue = true;


	return true;
}

function CG_RenameTool_Execute()
{
	var oPSet = ActiveSceneRoot.AddProperty( "CG_RenameProp", false, "RenameTool" );
	InspectObj( oPSet, "", "RenameTool", 3  );
	
	if(selection.Count > 0)
	{
		oPSet.PreviewO.Value = selection(0).Name;
		oPSet.Preview.Value = GetPreview(oPSet);
		oPSet.SelectionO.Value = selection.GetAsText();
	}
	
	return true;
}
//[cf]

//[of]:CG_Rename (Command)
function CG_Rename_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "Need details ?";
	oCmd.ReturnValue = true;

	var oArgs;
	oArgs = oCmd.Arguments;

	oArgs.AddWithHandler("inColl", "Collection");

	oArgs.Add("inSearch", siArgumentInput, "" );
	oArgs.Add("inReplace", siArgumentInput, "" );
	oArgs.Add("inPrefix", siArgumentInput, "" );
	oArgs.Add("inSuffix", siArgumentInput, "" );
	oArgs.Add("inRemoveFirst", siArgumentInput, 0 );	
	oArgs.Add("inRemoveLast", siArgumentInput, 0 );
	oArgs.Add("inNumbering", siArgumentInput, "Ignore" );
	oArgs.Add("inPadding", siArgumentInput, 2 );
	oArgs.Add("IgnoredPrefix", siArgumentInput, "" );
	oArgs.Add("IgnoredSuffix", siArgumentInput, "" );

	oArgs.Add("inConds", siArgumentInput, "" );
	
	return true;
}

function CG_Rename_Execute(inColl, inSearch, inReplace, inPrefix, inSuffix, inRemoveFirst, inRemoveLast,inNumbering,inPadding,IgnoredPrefix, IgnoredSuffix, inConds)
{
	if(inColl.Count > 0)
	{
		for(var i = 0; i < inColl.Count; i++)
		{
			inColl(i).Name = ApplyChanges(inColl(i).Name, inSearch, inReplace, inPrefix, inSuffix, inRemoveFirst, inRemoveLast,inNumbering, inPadding,IgnoredPrefix,IgnoredSuffix,i + 1, inConds);
		}

		return true;
	}
}
//[cf]

//[of]:CG_DuplicateRename (Command)
function CG_DuplicateRename_Init( ctxt )
{
	var oCmd;
	oCmd = ctxt.Source;
	oCmd.Description = "Need details ?";
	oCmd.ReturnValue = true;

	var oArgs;
	oArgs = oCmd.Arguments;

	oArgs.AddWithHandler("inColl", "Collection");

	oArgs.Add("inSearch", siArgumentInput, "" );
	oArgs.Add("inReplace", siArgumentInput, "" );
	oArgs.Add("inPrefix", siArgumentInput, "" );
	oArgs.Add("inSuffix", siArgumentInput, "" );
	oArgs.Add("inRemoveFirst", siArgumentInput, 0 );	
	oArgs.Add("inRemoveLast", siArgumentInput, 0 );
	oArgs.Add("inMode", siArgumentInput, "" );
	oArgs.Add("inReassignConstraints", siArgumentInput, true );
	oArgs.Add("inReassignExpressions", siArgumentInput, true );
	return true;
}

function CG_DuplicateRename_Execute(inColl, inSearch, inReplace, inPrefix, inSuffix, inRemoveFirst, inRemoveLast, inMode, inReassignConstraints, inReassignExpressions)
{
	var oOrigObjects = SelectChildNodes(inColl, false, false);
	var oNewObjects;
	
	CG_Rename_Execute(oOrigObjects, "", "", "", "_");
	
	switch(inMode)
	{
		case "XY" :
			DuplicateSymmetry(inColl, null, null, 0, 0, 1, 0, null);
		break;
		
		case "XZ" :
			DuplicateSymmetry(inColl, null, null, 0, 1, 0, 0, null);
		break;
		
		case "YZ" :
			DuplicateSymmetry(inColl, null, null, 1, 0, 0, 0, null);
		break;
		
		default :
			Duplicate(inColl, null, 2, 1, 1, 0, 0, 1, 0, 0, null, null, null, null, null, null, null, null, null, null, 0);
		break;
	}
	
	oNewObjects = SelectChildNodes(null, false, false);
	var oModel = oNewObjects(0).Model;
	var sName = "";	
	
	//Fix in case we have models in the selection
	for(var i = 0; i < oNewObjects.Count; i++)
	{
		sName = oNewObjects(i).Name;
		if(sName.length < 3 || sName.substring(sName.length - 2, sName.length) != "_1")
		{
			oNewObjects(i).Name = sName + "1";
		}
	}
	
	if(inReassignConstraints == true)
	{
		for(var i = 0; i < oNewObjects.Count; i++)
		{
			var Cns = oNewObjects(i).Kinematics.Constraints;
			
			if(Cns.Count > 0)
			{
				for(var j = 0; j < Cns.Count; j++)
				{
					var NewCnsing = new ActiveXObject( "XSI.Collection" );
					var Cnsing = Cns(j).Constraining;
					var UpV = Cns(j).UpVectorReference;
					var CnsingObj;
					var bRetarget = false;
					
					if(UpV != null)
					{
						var RetargetName = ApplyChanges(UpV.Name, inSearch, inReplace, inPrefix, inSuffix,inRemoveFirst, inRemoveLast);
						var RetargetObj = oModel.FindChild(RetargetName);
													
						if(RetargetObj != null)
						{
							bRetarget = true;
							UpV = RetargetObj;
						}
					}
					
					for(var k = 0; k < Cnsing.Count; k++)
					{
						CnsingObj = Cnsing(k);
						
						var RetargetName = ApplyChanges(CnsingObj.Name, inSearch, inReplace, inPrefix, inSuffix,inRemoveFirst, inRemoveLast);
						var RetargetObj = oModel.FindChild(RetargetName);
													
						if(RetargetObj != null)
						{
							bRetarget = true;
							NewCnsing.Add(RetargetObj);
						}
						else
						{
							NewCnsing.Add(CnsingObj);
						}
					}
					
					if(bRetarget == true)
					{
						RetargetConstraint(Cns(j), NewCnsing, UpV);
					}
				}
			}
		}
	}
	
	if(inReassignExpressions == true)
	{
		for(var i = 0; i < oNewObjects.Count; i++)
		{
			var Animated = oNewObjects(i).NodeAnimatedParameters(siExpressionSource);
			
			for(var j = 0; j < Animated.Count; j++)
			{
				var Expr = Animated(j).Source;
				var ExprInputs = Expr.InputPorts;
				
				for(var k = 0; k < ExprInputs.Count; k++)
				{
					var Input = ExprInputs(k).Target2.Parent3DObject;
					
					if(bBelongsToColl(oNewObjects, Input) == false)
					{
						var RetargetName = ApplyChanges(Input.Name, inSearch, inReplace, inPrefix, inSuffix,inRemoveFirst, inRemoveLast);
						var RetargetObj = oModel.FindChild(RetargetName);
													
						if(RetargetObj != null)
						{
							var reg=new RegExp(Input.FullName, "g");
							Expr.Parameters("Definition").Value = Expr.Parameters("Definition").Value.replace(reg, RetargetObj.FullName);
						}
					}
				}
			}
		}
	}

	CG_Rename_Execute(oNewObjects, "", "", "", "", 0, 2);
	CG_Rename_Execute(oNewObjects, inSearch, inReplace, inPrefix, inSuffix,inRemoveFirst, inRemoveLast);
	CG_Rename_Execute(oOrigObjects, "", "", "", "", 0, 1);
}
//[cf]

//[of]:CG_RenameProp (Property)
function CG_RenameProp_OnInit( )
{
	Application.LogMessage("CG_RenameProp_OnInit called",siVerbose);
}

function CG_RenameProp_OnClosed( )
{
	DeleteObj(PPG.Inspected.Item(0));
}

//[of]:Define
function CG_RenameProp_Define( in_ctxt )
{
	var oCustomProperty;
	oCustomProperty = in_ctxt.Source;
	oCustomProperty.AddParameter2("Search",siString,"",null,null,null,null,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("Hierar",siBool,false,null,null,null,null,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("Replace",siString,"",null,null,null,null,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("Prefix",siString,"",null,null,null,null,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("RemNFirst",siInt4,0,0,10000,0,20,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("Suffix",siString,"",null,null,null,null,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("RemoveNLast",siInt4,0,0,10000,0,20,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("Preview",siString,"",null,null,null,null,siClassifUnknown,siPersistable | siReadOnly);
	oCustomProperty.AddParameter2("PreviewO",siString,"",null,null,null,null,siClassifUnknown,siPersistable | siReadOnly);
	oCustomProperty.AddParameter2("ObjIdx",siInt4,0,0,10000,0,20,siClassifUnknown,siPersistable);

	oCustomProperty.AddParameter2("ReassignConstraints", siBool,1,0,1,0,1,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("ReassignExpressions", siBool,1,0,1,0,1,siClassifUnknown,siPersistable);

	oCustomProperty.AddParameter2("Conditions",siString,"",null,null,null,null,siClassifUnknown,siPersistable | siReadOnly);

	oCustomProperty.AddParameter2("SelectionO",siString,"",null,null,null,null,siClassifUnknown,siPersistable | siReadOnly);

	oCustomProperty.AddParameter2("DuplicateMode",siString,"No symmetry",null,null,null,null,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("NumberingMode",siString,"Ignore",null,null,null,null,siClassifUnknown,siPersistable);
	
	oCustomProperty.AddParameter2("NumberingPadding",siInt4,2,0,10000,0,5,siClassifUnknown,siPersistable);
	
	oCustomProperty.AddParameter2("IgnoredPrefix",siString,"",null,null,null,null,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("IgnoredSuffix",siString,"",null,null,null,null,siClassifUnknown,siPersistable);

	oCustomProperty.AddParameter2("RenumberMode",siInt4,0,0,10000,0,20,siClassifUnknown,siPersistable);
	oCustomProperty.AddParameter2("RenumberOrder",siInt4,0,0,10000,0,20,siClassifUnknown,siPersistable);
	
	return true;
}

function CG_RenameProp_DefineLayout( in_ctxt )
{
	var oLayout,oItem;
	oLayout = in_ctxt.Source;
	oLayout.Clear();
	
	oLayout.AddTab("Rename");
	
	// !! DUPLICATE !!
	
	oLayout.AddRow();
		oLayout.AddItem("Preview");
		oLayout.AddButton("Next", "Next Object -->");
	oLayout.EndRow();
	
	oLayout.AddStaticText("");
	
	oLayout.AddGroup("Search&Replace");
		oLayout.AddRow();
			oLayout.AddItem("Search", "Search:");
			oLayout.AddItem("Replace", "Replace:");
		oLayout.EndRow();
	oLayout.EndGroup();
	
	oLayout.AddGroup("Prefix&Suffix");
		oLayout.AddRow();
			oLayout.AddItem("Prefix", "Prefix:" );
			oLayout.AddItem("Suffix", "Suffix:");
		oLayout.EndRow();
		oLayout.AddRow();
			oLayout.AddItem("RemNFirst", "Remove first chars :");
			oLayout.AddItem("RemoveNLast", "Remove last chars :");
		oLayout.EndRow();
	oLayout.EndGroup();
		
	// !! END OF DUPLICATE !!
	
	oLayout.AddGroup("Numbering");
	oLayout.AddRow();
		oItem = oLayout.AddItem("NumberingMode", "", siControlCombo);
		oItem.SetAttribute(siUINoLabel, true)
		oItem.UIItems = ["Ignore","Ignore","Remove","Remove","Renumber","Renumber","Keep","Keep"];
		oItem = oLayout.AddItem("NumberingPadding", "Padding");
	oLayout.EndRow();
	oLayout.EndGroup();
	
	oLayout.AddStaticText("");
	
	oLayout.AddItem("Hierar", "Consider hierarchy:");
	//oLayout.AddItem("Conditions", "Conditions:");
	
	oLayout.AddButton("Rename");
	
	oLayout.AddTab("Duplicate");
	
	// !! DUPLICATE !!
	
		oLayout.AddRow();
		oLayout.AddItem("Preview");
		oLayout.AddButton("Next", "Next Object -->");
	oLayout.EndRow();
	
	oLayout.AddStaticText("");
	
	oLayout.AddGroup("Search&Replace");
		oLayout.AddRow();
			oLayout.AddItem("Search", "Search:");
			oLayout.AddItem("Replace", "Replace:");
		oLayout.EndRow();
	oLayout.EndGroup();
	
	oLayout.AddGroup("Prefix&Suffix");
		oLayout.AddRow();
			oLayout.AddItem("Prefix", "Prefix:" );
			oLayout.AddItem("Suffix", "Suffix:");
		oLayout.EndRow();
		oLayout.AddRow();
			oLayout.AddItem("RemNFirst", "Remove first chars :");
			oLayout.AddItem("RemoveNLast", "Remove last chars :");
		oLayout.EndRow();
	oLayout.EndGroup();
	
	// !! END OF DUPLICATE !!
	
	oLayout.AddStaticText("");
	
	oLayout.AddRow();
		oLayout.AddItem("ReassignConstraints", "Reassign constraints");
		oLayout.AddItem("ReassignExpressions", "Reassign expressions");
	oLayout.EndRow();
	
	oItem = oLayout.AddItem("DuplicateMode", "Symmetry", siControlCombo);
	oItem.UIItems = ["No symmetry","No symmetry","XY","XY","XZ","XZ","YZ","YZ"];

	oLayout.AddButton("Duplicate");

	oLayout.AddTab("Advanced Settings");
	
	oLayout.AddGroup("Ignore Prefix&Suffix");
		oLayout.AddRow();
		oLayout.AddItem("IgnoredPrefix", "Prefix:" );
		oLayout.AddItem("IgnoredSuffix", "Suffix:");
		oLayout.EndRow();
	oLayout.EndGroup();
	
	return true;
}
//[cf]

//[of]:Buttons
function CG_RenameProp_Rename_OnClicked()
{
	if(selection.Count > 0)
	{
		var inProp = PPG.Inspected.Item(0);
		CG_Rename(GetValue("SelectionList"), inProp.Parameters("Search").Value, inProp.Parameters("Replace").Value, inProp.Parameters("Prefix").Value, inProp.Parameters("Suffix").Value,0 + inProp.Parameters("RemNFirst").Value, 0 + inProp.Parameters("RemoveNLast").Value, inProp.Parameters("NumberingMode").Value, inProp.Parameters("NumberingPadding").Value, inProp.Parameters("Conditions").Value );
	}
	else
	{
		logmessage("Please select at least one object to rename !!", siError);
	}
}

function CG_RenameProp_Duplicate_OnClicked()
{
	if(selection.Count > 0)
	{
		var inProp = PPG.Inspected.Item(0);
		CG_DuplicateRename(GetValue("SelectionList"), inProp.Parameters("Search").Value, inProp.Parameters("Replace").Value, inProp.Parameters("Prefix").Value, inProp.Parameters("Suffix").Value,0 + inProp.Parameters("RemNFirst").Value, 0 + inProp.Parameters("RemoveNLast").Value, inProp.Parameters("DuplicateMode").Value, inProp.Parameters("ReassignConstraints").Value == true, inProp.Parameters("ReassignExpressions").Value);
	}
	else
	{
		logmessage("Please select at least one object to duplicate !!", siError);
	}
}

function CG_RenameProp_Next_OnClicked( )
{
	var oPSet = PPG.Inspected.Item(0);
	var CurIndex = oPSet.ObjIdx.Value + 1;
	
	if(CurIndex >= selection.Count)
	{
		CurIndex = 0;
	}
	
	oPSet.ObjIdx.Value = CurIndex;
	
	oPSet.PreviewO.Value = selection(CurIndex).Name;
	PPG.Preview.Value = GetPreview(oPSet);
}
//[cf]

//[of]:OnChanged
function CG_RenameProp_Search_OnChanged( )
{
	PPG.Preview.Value = GetPreview(PPG.Inspected.Item(0));
}

function CG_RenameProp_Replace_OnChanged( )
{
	PPG.Preview.Value = GetPreview(PPG.Inspected.Item(0));
}

function CG_RenameProp_Prefix_OnChanged( )
{
	PPG.Preview.Value = GetPreview(PPG.Inspected.Item(0));
}

function CG_RenameProp_RemNFirst_OnChanged( )
{
	PPG.Preview.Value = GetPreview(PPG.Inspected.Item(0));
}

function CG_RenameProp_Suffix_OnChanged( )
{
	PPG.Preview.Value = GetPreview(PPG.Inspected.Item(0));
}

function CG_RenameProp_RemoveNLast_OnChanged( )
{
	PPG.Preview.Value = GetPreview(PPG.Inspected.Item(0));
}

function CG_RenameProp_NumberingMode_OnChanged( )
{
	PPG.Preview.Value = GetPreview(PPG.Inspected.Item(0));
}

function CG_RenameProp_NumberingPadding_OnChanged( )
{
	PPG.Preview.Value = GetPreview(PPG.Inspected.Item(0));
}

function CG_RenameProp_Hierar_OnChanged( )
{
	oPSet = PPG.Inspected.Item(0);
	
	if(PPG.Hierar.Value == true)
	{
		PPG.PreviewO.Value = selection(0).Name;
		PPG.Preview.Value = GetPreview(oPSet);
		PPG.SelectionO.Value = selection.GetAsText();
	
		SelectBranch(null);
		SelectChildNodes(null, null, null);
	}
	else
	{
		try
		{
			DeselectAll();
			SelectObj(PPG.SelectionO.Value);
		}
		catch(e){};
	}
	
	PPG.ObjIdx.Value = 0;
	PPG.Preview.Value = GetPreview(PPG.Inspected.Item(0));
}
//[cf]

//[cf]

//[of]:Helpers
function GetPreview(oPSet)
{
	var CurIdx = oPSet.ObjIdx.Value;
	var CurObj = selection(CurIdx);
	
	if(CurObj != null)
	{
		var CurModel = CurObj.Model;
	
		return ApplyPropChanges(oPSet.PreviewO.Value, oPSet) + " (" + (CurIdx + 1) + " / " + selection.Count + ")";
	}
	
	return "";
}

function ApplyPropChanges(inString, inProp)
{
	return ApplyChanges(inString, inProp.Search.Value, inProp.Replace.Value, inProp.Prefix.Value, inProp.Suffix.Value, 0 + inProp.RemNFirst.Value, 0 + inProp.RemoveNLast.Value, inProp.NumberingMode.Value, inProp.NumberingPadding.Value, inProp.IgnoredPrefix.Value, inProp.IgnoredSuffix.Value, 1  );
}

function ApplyChanges(inString, inSearch, inReplace, inPrefix, inSuffix, inRemoveFirst, inRemoveLast,inNumbering,inPadding,IgnoredPrefix,IgnoredSuffix, Index, inConds )
{
	var modifiedString = inString;
	var prefix = "";
	var suffix = "";
	
	if(IgnoredPrefix != "" && IgnoredPrefix != undefined)
	{
		var PrefixLength = IgnoredPrefix.length;
		if(IgnoredPrefix == modifiedString.substring(0, PrefixLength))
		{
			prefix = IgnoredPrefix;
			modifiedString = modifiedString.substring(PrefixLength, modifiedString.length - PrefixLength);
		}
	}
	
	if(IgnoredSuffix != "" && IgnoredSuffix != undefined)
	{
		var SuffixLength = IgnoredSuffix.length;
		if(IgnoredSuffix == modifiedString.substring(modifiedString.length - SuffixLength, modifiedString.length))
		{
			suffix = IgnoredSuffix;
			modifiedString = modifiedString.substring(0, modifiedString.length - SuffixLength);
		}
	}
	
	//Search & Replace
	if (inSearch != "" && inSearch != undefined)
	{
		modifiedString = modifiedString.replace(inSearch, inReplace);
	}
	
	var Number = "";
	
	//Numbering data
	if (inNumbering != undefined && inNumbering != "Ignore")
	{
		var Number = GetNumbers(modifiedString);
		if(Number.length > 0)
		{
			Number = Number[0];
			modifiedString = modifiedString.substr(0, modifiedString.length - Number.length);
		}
		else
		{
			Number = "";
		}
	}

	//Remove First
	if (inRemoveFirst != 0  && inRemoveFirst != undefined)
	{
		if(inRemoveFirst >= modifiedString.length)
		{
			modifiedString = "";
		}
		else
		{
			modifiedString = modifiedString.substr(inRemoveFirst, modifiedString.length - inRemoveFirst);
		}
	}
	
	//Remove Last
	if (inRemoveLast != 0  && inRemoveLast != undefined)
	{
		if(inRemoveLast >= modifiedString.length)
		{
			modifiedString = "";
		}
		else
		{
			modifiedString = modifiedString.substr(0, modifiedString.length - inRemoveLast);
		}
	}
	
	//Prefix
	if (inPrefix != "" && inPrefix != undefined)
	{
		modifiedString = inPrefix + modifiedString;
	}
	
	 //Suffix
	if (inSuffix != "" && inSuffix != undefined)
	{
		modifiedString = modifiedString + inSuffix;
	}
	
	//Numbering
	if(inNumbering == "Renumber")
	{
		modifiedString = Pad(modifiedString, Index, inPadding);
	}
	else
	{
		if (inNumbering == "Keep" && Number != "")
		{
			modifiedString = Pad(modifiedString, Number, inPadding);
		}
	}
	
	
	
	return prefix + modifiedString + suffix;
}

function GetNumbers(inString)
{
	var reg = new RegExp("[0-9]*$","g");
	return reg.exec(inString);
}	

function Pad(Prefix, Postfix, inPad)
{
	var reg = new RegExp("[1-9]*$","g");
	var Num = reg.exec(Postfix)[0];
	
	while(Num.length < inPad)
	{
		Num = "0" + Num;
	}
	
	return Prefix + Num;
}

function bBelongsToColl(oColl, oObj)
{
	for(var i = 0; i < oColl.Count; i++)
	{
		if(oColl(i).UniqueName == oObj.UniqueName)
		{
			return true;
		}
	}
	
	return false;
}

function RetargetConstraint(inCns, inCnsing, inUpVector)
{
	var UserPath = XSIUtils.BuildPath( Application.InstallationPath( siUserPath ), "Data", "DSPresets", "Properties") + "\\temp.Preset";
	SavePreset(inCns,UserPath);

	var Constrained = inCns.Constrained;
	var CnSPresetName = "";
	
	switch(inCns.Type)
    {
        case "2ptscns" : //• TwoPoints - 
            CnSPresetName = "TwoPoints";
        break;
        case "3ptscns" : //• ThreePoints - 
            CnSPresetName = "ThreePoints";            
        break;
        case "surfcns" : //• Surface - 
            CnSPresetName = "Surface";
        break;
        case "distcns" : //• Distance - 
            CnSPresetName = "Distance";
        break;
        case "poscns" : //• Position - 
            CnSPresetName = "Position";
        break;
        case "oricns" : //• Orientation - 
            CnSPresetName = "Orientation";
        break;
        case "posecns" : //• Pose - 
            CnSPresetName = "Pose";
        break;
        case "sclcns" : //• Scaling - 
            CnSPresetName = "Scaling";
        break;
        case "bplanecns" : //• BoundingPlane - 
            CnSPresetName = "BoundingPlane";
        break;
        case "dircns" : //• Direction - 
            CnSPresetName = "Direction";
        break;
        case "symmetrycns" : //• Symmetry - 
            CnSPresetName = "Symmetry";
        break;
        case "objclscns" : //• ObjectToCluster - 
            CnSPresetName = "ObjectToCluster";
        break;
        case "Nptscns" : //• NPoints - 
            CnSPresetName = "NPoints";
        break;
        case "pathcns" : //• Path - 
            CnSPresetName = "Path";
        break;
        case "crvcns" : //• Trajectory - 
            CnSPresetName = "Trajectory";
        break;
        case "bvolcns" : //• BoundingVolume - 
            CnSPresetName = "BoundingVolume";
        break;
    }
	
	Deleteobj(inCns);
	var NewCns = Constrained.Kinematics.AddConstraint(CnSPresetName, inCnsing, false);
	
    if (inUpVector != null)
    {
        ApplyOp("UpVectorDefiner", NewCns.FullName + ";" + inUpVector.FullName);
    }
	
	LoadPreset(UserPath,NewCns);
}
//[cf]


