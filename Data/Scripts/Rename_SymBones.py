from win32com.client import constants
from win32com.client.dynamic import Dispatch

app = Dispatch('XSI.Application').Application
XSIFactory = Dispatch('XSI.Factory')
XSIMath = Dispatch('XSI.Math')
XSIUIToolKit = Dispatch('XSI.UIToolKit')
XSIUtils = Dispatch('XSI.Utils')

for selected in app.Selection:
	selected.Name = selected.Name.replace("L_", "R_")[:-1]

children = app.ActiveSceneRoot.FindChildren()
roots = [child for child in children if child.Type == "root"]
effectors = [child for child in children if child.Type == "eff"]
for child in children:
	print(child.Name, child.Type)


for root in roots:
	app.SetValue("{}.root.size".format(root.FullName), 0.1, "")

for effector in effectors:
	app.SetValue("{}.eff.size".format(effector.FullName), 0.1, "")

	
