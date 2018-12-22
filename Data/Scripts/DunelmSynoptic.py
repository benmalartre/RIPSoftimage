from Globals import *

def XSILoadPlugin( in_reg ):
	in_reg.Author = "benmalartre"
	in_reg.Name = "Dunelm Synoptic Plugin"
	in_reg.Major = 1
	in_reg.Minor = 0

	in_reg.RegisterProperty("DunelmSynoptic")	
	in_reg.RegisterCommand("ApplyDunelmSynoptic")
	in_reg.RegisterCommand("DemoDunelmSynoptic")


	XSILoadPlugin = True


def DunelmSynoptic_Define( ctxt ):
	oCustomProperty = ctxt.Source

	oCustomProperty.AddParameter2("path",constants.siString,"",None,None,None,None,0,constants.siPersistable)
	
	#The "path" parameter associated with the synoptic control
	#specifies the full path of the synoptic html file.
	#To avoid hard-coding an absolute path, we determine the path 
	#relative to this script file.
	oThisPlugin = Application.Plugins( "Dunelm Synoptic Plugin" )
	oCustomProperty.path.Value = "D:\Projects\Dunelm\Synoptic\Rory.htm"
	#SynopticoThisPlugin.OriginPath + ".." + XSIUtils.Slash + "synoptic" + XSIUtils.Slash + "embeddedsynoptic.htm"

def DunelmSynoptic_DefineLayout( ctxt ):
	oLayout = ctxt.Source
	oLayout.Clear()

	oLayout.AddRow()
	oLayout.AddGroup("Controls")
	oLayout.AddRow()
	oBtn = oLayout.AddButton("KeyAll","KeyAll")
	oLayout.AddButton("KeySelected","KeySelected")
	#oBtn.SetAttribute(constants.siUI
	oLayout.EndRow()
	oLayout.AddRow()
	oLayout.AddButton("RemoveAll","RemoveAll")
	oLayout.AddButton("RemoveSelected","RemoveSelected")
	oLayout.EndRow()
	oLayout.AddRow()
	oLayout.AddButton("ResetAll","ResetAll")
	oLayout.AddButton("ResetSelected","ResetSelected")
	oLayout.EndRow()
	oLayout.EndGroup()
	oLayout.AddGroup("Help")
	oLayout.AddStaticText("Click : Select Object\nCtrl+Click : Toggle Select Object\nShift+Click : Add Select Object\nCtrl+Shift+Click : Remove Select Object",400)

	oLayout.EndGroup()
	oLayout.EndRow()
	#UI text that explains how to use the synoptic view
		
	#Create the synoptic widget, which loads the synoptic view specified by the path parameter
	oItem = oLayout.AddItem("path", "", constants.siControlSynoptic)
	oItem.SetAttribute(constants.siUINoLabel,True)
	

#Set the UI text using the name of the parent object
def DunelmSynoptic_OnInit():
	oThisPlugin = Application.Plugins( "Dunelm Synoptic Plugin")
	path = oThisPlugin.OriginPath + ".." + XSIUtils.Slash + "Synoptic" + XSIUtils.Slash + "Rory.htm"
	PPG.Path.Value = path
	PPG.Refresh()
	
# Key All
#-----------------------------------
def DunelmSynoptic_KeyAll_OnClicked():
	model = PPG.Inspected(0)
	ctrls = GetControllers(model)
	if not ctrls:
		xsi.LogMessage("Key All Aborted...")
		return False
	else:
		xsi.SelectObj(ctrls);	
		xsi.SaveKeyOnKeyable();

# Key Selected
#-----------------------------------	
def DunelmSynoptic_KeySelected_OnClicked():
	sel = xsi.Selection
	if sel.Count == 0:
		xsi.LogMessage("Nothing Selected ---> Key Selected Aborted...",constants.siWarning)
	else:
		xsi.SaveKeyOnKeyable()
		


# Reset All
#-----------------------------------	
def DunelmSynoptic_ResetAll_OnClicked():	
	model = PPG.Inspected(0).Parent3DObject
	ctrls = GetControllers(model)
	if not ctrls:
		xsi.LogMessage("Reset All Aborted...")
		return False
	else:
		xsi.ResetTransform(ctrls , "siObj", "siSRT", "siXYZ")
		
def DunelmSynoptic_ResetSelected_OnClicked():
	ctrls = xsi.Selection
	if not ctrls:
		xsi.LogMessage("Reset Selected Aborted...")
		return False
	else:
		xsi.ResetTransform(ctrls , "siObj", "siSRT", "siXYZ")
		
# Remove All
#-----------------------------------
def DunelmSynoptic_RemoveAll_OnClicked():
	model = PPG.Inspected(0).Parent3DObject
	ctrls = GetControllers(model)
	if not ctrls:
		xsi.LogMessage("Remove All Aborted...")
		return False
	else:
		xsi.SelectObj(ctrls);	
		request = XSIUIToolkit.MsgBox("Do you really want to remove ALL animation on "+model.Name,constants.siMsgYesNo|constants.siMsgQuestion,"Dunelm")
		if request == constants.siMsgYes:
			xsi.RemoveAllAnimation(ctrls)
			
# Remove Selected
#-----------------------------------
def DunelmSynoptic_RemoveSelected_OnClicked():
	ctrls = xsi.Selection
	if not ctrls:
		xsi.LogMessage("Remove Selected Aborted...")
		return False
	else:	
		request = XSIUIToolkit.MsgBox("Do you really want to remove animation on selected controllers",constants.siMsgYesNo|constants.siMsgQuestion,"Dunelm")
		if request == constants.siMsgYes:
			xsi.RemoveAllAnimation(ctrls)

# Key Selected
#-----------------------------------	
def DunelmSynoptic_KeySelected_OnClicked():
	sel = xsi.Selection
	if sel.Count == 0:
		xsi.LogMessage("Nothing Selected ---> Key Selected Aborted...",constants.siWarning)
	else:
		xsi.SaveKeyOnKeyable()
		

def ApplyDunelmSynoptic_Init( ctxt ):

	oCmd = ctxt.Source
	oCmd.Description = ""
	oCmd.ReturnValue = True

	oArgs = oCmd.Arguments
	
	#The Collection arg handler gives us the current selection 
	#if no argument is explicitly specified
	oArgs.AddWithHandler("target","Collection")
	

def ApplyDunelmSynoptic_Execute( target ):
	
	#Error handling of the input argument	
	if target == "":
		xsi.Logmessage("Invalid argument to ApplyEmbeddedSynoptic", constants.siError)
		return
		
	elif target.Type == "XSICollection":
		#target may be a collection of items, such as the current selection	
		
		for oItem in target:
			if not oItem.IsClassOf( constants.siX3DObjectID ):			
				xsi.LogMessage("Invalid argument " + oItem.FullName + " to ApplyDunelmSynoptic", constants.siError	)		
				return
	
	elif not target.IsClassOf( constants.siX3DObjectID ):
		xsi.Logmessage("Invalid argument " + oItem.FullName + " to ApplyDunelmSynoptic", constants.siError	)			
	
	#Create the custom property	
	prop = target(0).AddProperty("DunelmSynoptic")

	#Show the custom property in the property panel	
	if  Application.Interactive:
		view = xsi.Desktop.Activelayout.Createview("property panel","Synoptic")	
		view.SetAttributeValue("targetcontent",prop.FullName)
		
		#We use a Property Panel view instead of InspectObj because
		#we can resize the view to better fit the synoptic view
		view.Resize(512,600)


#For demonstration purposes,  create an object and add the custom property
def DemoDunelmSynoptic_Execute():
	target = xsi.Selection(0)
	xsi.ApplyDunelmSynoptic( target )

	
# Get Controllers
#-----------------------------------
def GetControllers(model=None):
	if not model:
		sel = xsi.Selection(0)

		if sel.Type == "#model":
			model = sel
		else:
			model = sel.Model
		
	ctrl = model.Groups("Anim_Ctrl")
	if not ctrl:
		xsi.LogMessage("Can't find Anim_Ctrl group under "+model.Name,constants.siError)
		return None
	else:
		return ctrl.Members




