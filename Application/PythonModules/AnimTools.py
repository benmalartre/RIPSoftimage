from win32com.client import constants
from Utils import *


def ResetAll():
	""" Reset all controls
	"""
	controls = GetControllers()
	if not controls:
		XSI.LogMessage('Reset All Aborted...')
		return False
	else:
		XSI.ResetTransform(controls, 'siObj', 'siSRT', 'siXYZ')


def KeyAll():
	""" Key all controls
	"""
	controls = GetControllers()
	if not controls:
		XSI.LogMessage('Key All Aborted...')
		return False
	else:
		XSI.SelectObj(controls)
		XSI.SaveKeyOnKeyable()
	

def GetControllers(model=None):
	""" Get controls from model or select
	:param Model model: SDK Model object (if none will use selection to find model)
	:return XSI.Collection: SDK X3DObject collection of controls
	"""
	if not model:
		sel = XSI.Selection(0)

		if sel.Type == '#model':
			model = sel
		else:
			model = sel.Model
		
	ctrl = model.Groups('Anim_Ctrl')
	if not ctrl:
		XSI.LogMessage('Can\'t find Anim_Ctrl group under "{}"'.format(model.Name), constants.siError)
		return None
	else:
		return ctrl.Members
