# Anim Tools
# -----------------------------------
from win32com.client import constants as siConstants
from Utils import *


# Reset All
# -----------------------------------
def ResetAll():
	controls = GetControllers()
	if not controls:
		XSI.LogMessage("Reset All Aborted...")
		return False
	else:
		XSI.ResetTransform(controls, "siObj", "siSRT", "siXYZ")


# Key All
# -----------------------------------
def KeyAll():
	controls = GetControllers()
	if not controls:
		XSI.LogMessage("Key All Aborted...")
		return False
	else:
		XSI.SelectObj(controls)
		XSI.SaveKeyOnKeyable()
	
	
# Get Controllers
# -----------------------------------
def GetControllers(model=None):
	if not model:
		sel = XSI.Selection(0)

		if sel.Type == "#model":
			model = sel
		else:
			model = sel.Model
		
	ctrl = model.Groups("Anim_Ctrl")
	if not ctrl:
		XSI.LogMessage("Can't find Anim_Ctrl group under " + model.Name, siConstants.siError)
		return None
	else:
		return ctrl.Members

# Clone Symmetrize

# KeyAll()
