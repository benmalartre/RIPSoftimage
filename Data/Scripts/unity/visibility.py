from Utils2 import *

model = xsi.Selection(0).Model
prop = model.Properties("CharacterControl")
master = prop.Parameters("L_Leg_IKFK")
for selected in xsi.Selection:
	selected.Properties("Visibility").Parameters("ViewVis").AddExpression("1 - "+str(master))
