# Anim Tools
#-----------------------------------
from win32com.client import constants as siConstants
from Utils import *

# Reset All
#-----------------------------------
def ResetAll():
	ctrls = GetControllers()
	if not ctrls:
		xsi.LogMessage("Reset All Aborted...")
		return False
	else:
		xsi.ResetTransform(ctrls, "siObj", "siSRT", "siXYZ")

# Key All
#-----------------------------------
def KeyAll():
	ctrls = GetControllers()
	if not ctrls:
		xsi.LogMessage("Key All Aborted...")
		return False
	else:
		xsi.SelectObj(ctrls)
		xsi.SaveKeyOnKeyable()
	
	
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
		xsi.LogMessage("Can't find Anim_Ctrl group under "+model.Name, siConstants.siError)
		return None
	else:
		return ctrl.Members

# Clone Symmetrize

#KeyAll()
