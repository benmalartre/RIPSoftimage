from Utils2 import *


def ChangeExpression(parameter):
	if parameter and parameter.Source and parameter.Source.Type == "Expression":
		expr = parameter.Source.Parameters("Definition").Value
		parameter.Source.Parameters("Definition").Value = expr.replace("L_","R_")


def SetShadowIcon(d, s):
	dp = d.ActivePrimitive
	sp = s.ActivePrimitive
	
	dp.Parameters("shadow_icon").Value = sp.Parameters("shadow_icon").Value
	
	dp.Parameters("shadow_offsetX").Value = sp.Parameters("shadow_offsetX").Value
	dp.Parameters("shadow_offsetY").Value = sp.Parameters("shadow_offsetY").Value
	dp.Parameters("shadow_offsetZ").Value = sp.Parameters("shadow_offsetZ").Value
	
	dp.Parameters("shadow_scaleX").Value = sp.Parameters("shadow_scaleX").Value
	dp.Parameters("shadow_scaleY").Value = sp.Parameters("shadow_scaleY").Value
	dp.Parameters("shadow_scaleZ").Value = sp.Parameters("shadow_scaleZ").Value


def DuplicateNullSymmetry(null, parent):
	if not parent:
		parent = null.Parent3DObject

	icon = null.ActivePrimitive.Parameters("primary_icon").Value
	size = null.ActivePrimitive.Parameters("size").Value
	name = null.Name.replace("L_", "R_")
	r = null.ActivePrimitive.Parameters("R").Value
	g = null.ActivePrimitive.Parameters("G").Value
	b = null.ActivePrimitive.Parameters("B").Value
	d = AddNull(parent, icon, size, name, r, g, b)
	
	SetShadowIcon(d, null)
	SetWireColor(d, 0.122, 0.875, 0.753)
	
	SimpleSymmetry(d, null, 0, 0)
	ChangeExpression(Dispatch(d.Properties("Visibility").Parameters("ViewVis")))
	ChangeExpression(Dispatch(d.Properties("Visibility").Parameters("Selectability")))

	for c in null.Children:		
		DuplicateNullSymmetry(c, d)


def DuplicateSkeletonSymmetry(root):
	xsi.DuplicateSymmetry(root, 0, "", 1, 0, 0)(0)
	xsi.SelectChildNodes()
	for s in xsi.Selection:
		s.Name = s.Name.replace("L_", "R_")[:-1]
		ChangeExpression(Dispatch(s.Properties("Visibility").Parameters("ViewVis")))
		ChangeExpression(Dispatch(s.Properties("Visibility").Parameters("Selectability")))


selected = xsi.Selection(0)

if selected.Type == "null":
	DuplicateNullSymmetry(selected, None)

elif selected.Type == "root":
	DuplicateSkeletonSymmetry(selected)
