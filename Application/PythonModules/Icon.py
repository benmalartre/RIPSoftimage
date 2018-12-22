# --------------------------------------------
# Icon module
# --------------------------------------------
import math

from Globals import xsi
from Globals import XSIMath
from win32com.client import constants as siConstants
import Utils as uti


# Create Rig Control Curve
def IRIcon( parent, name, t, color, icon, width=1, height=1, depth=1, po=None, ro=None, ap=None):
	if not parent:
		parent = xsi.ActiveSceneRoot
		
	if not po:
		po = XSIMath.CreateVector3()
	if not ro:
		ro = XSIMath.CreateRotation()
	if not ap:
		ap = XSIMath.CreateVector3()

	if icon == "cube":
		ctl = cube(parent, name, width, height, depth, color, t, po, ro)
	elif icon == "pyramid":
		ctl = pyramid(parent, name, width, height, depth, color, t, po, ro)
	elif icon == "square":
		ctl = square(parent, name, width, depth, color, t, po, ro)
	elif icon == "flower":
		ctl = flower(parent, name, width, color, t, po, ro)
	elif icon == "circle":
		ctl = circle(parent, name, width, color, t, po, ro)
	elif icon == "cylinder":
		ctl = cylinder(parent, name, width, height, color, t, po, ro)
	elif icon == "compas":
		ctl = compas(parent, name, width, color, t, po, ro)
	elif icon == "foil":
		ctl = foil(parent, name, width, height, color, t, po, ro)
	elif icon == "diamond":
		ctl = diamond(parent, name, width, color, t, po, ro)
	elif icon == "leash":
		ctl = leash(parent, name, width, ap, color, t, po, ro)
	elif icon == "cubewithpeak":
		ctl = cubewithpeak(parent, name, width, color, t, po, ro)
	elif icon == "sphere":
		ctl = sphere(parent, name, width, color, t, po, ro)
	elif icon == "arrow":
		ctl = arrow(parent, name, width, color, t, po, ro)
	elif icon == "crossarrow":
		ctl = crossarrow(parent, name, width, color, t, po, ro)
	elif icon == "bendedarrow":
		ctl = bendedarrow(parent, name, width, color, t, po, ro)
	elif icon == "bendedarrow2":
		ctl = bendedarrow2(parent, name, width, color, t, po, ro)
	elif icon == "cross":
		ctl = cross(parent, name, width, color, t, po, ro)
	elif icon == "glasses":
		ctl = glasses(parent, name, width, color, t, po, ro)
	elif icon == "lookat":
		ctl = lookat(parent, name, width, color, t, po, ro)
	elif icon == "eyearrow":
		ctl = eyearrow(parent, name, width, color, t, po, ro)
	elif icon == "anglesurvey":
		ctl = anglesurvey(parent, name, width, color, t, po, ro)
	elif icon == "eyeball":
		ctl = eyeball(parent, name, width, color, t, po, ro)
	elif icon == "rectanglecube":
		ctl = rectanglecube(parent, name, width, color, t, po, ro)
	elif icon == "man":
		ctl = man(parent, name, width, color, t, po, ro)
	elif icon == "null":
		ctl = null(parent, name, width, color, t, po, ro)
	else: 
		xsi.LogMessage("[RigIcon] : invalid type of icon", siConstants.siError)
		return
			
	return ctl


# ========================================================
# Create a curve with a SIMPLE shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param endPosition SIVector3 - The end position of the curve
def simple(parent, name="Simple", color=[0, 0, 0], t=XSIMath.CreateTransform(), endPosition=XSIMath.CreateVector3()):
	p = t.Translation
	p1 = XSIMath.MapWorldPositionToObjectSpace(t, p)
	p = endPosition
	p2 = XSIMath.MapWorldPositionToObjectSpace(t, p)
	pos_offset = XSIMath.CreateVector3()
	rot_offset = XSIMath.CreateRotation()

	points = getPointArrayWithOffset([p1, p2], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, False, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t
	return curve


# ========================================================
# Create a curve with a CUBE shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param height Float - Height of the shape.
# @param depth Float - Depth of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def cube(parent, name="cube", width=1, height=1, depth=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):
	lenX = width * 0.5
	lenY = height * 0.5
	lenZ = depth * 0.5

	# p is positive, N is negative
	ppp = XSIMath.CreateVector3(lenX, lenY, lenZ)
	ppN = XSIMath.CreateVector3(lenX, lenY, lenZ*-1)
	pNp = XSIMath.CreateVector3(lenX, lenY*-1, lenZ)
	Npp = XSIMath.CreateVector3(lenX*-1, lenY, lenZ)
	pNN = XSIMath.CreateVector3(lenX, lenY*-1, lenZ*-1)
	NNp = XSIMath.CreateVector3(lenX*-1, lenY*-1, lenZ)
	NpN = XSIMath.CreateVector3(lenX*-1, lenY, lenZ*-1)
	NNN = XSIMath.CreateVector3(lenX*-1, lenY*-1, lenZ*-1)

	points = getPointArrayWithOffset([ppp, ppN, NpN, NNN, NNp, Npp, NpN, Npp, ppp, pNp, NNp, pNp, pNN, ppN, pNN, NNN], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, False, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t
	return curve


# ========================================================
# Create a curve with a PYRAMIDE shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param height Float - Height of the shape.
# @param depth Float - Depth of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def pyramid(parent, name="pyramid", width=1, height=1, depth=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	lenX = width * 0.5
	lenY = height
	lenZ = depth * 0.5

	# p is positive, N is negative
	top = XSIMath.CreateVector3(0, lenY, 0)
	pp = XSIMath.CreateVector3(lenX, 0, lenZ)
	pN = XSIMath.CreateVector3(lenX, 0, lenZ*-1)
	Np = XSIMath.CreateVector3(lenX*-1, 0, lenZ)
	NN = XSIMath.CreateVector3(lenX*-1, 0, lenZ*-1)

	points = getPointArrayWithOffset([pp, top, pN, pp, Np, top, NN, Np, NN, pN], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, False, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a SQUARE shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param depth Float - Depth of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def square(parent, name="square", width=1, depth=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	lenX = width * 0.5
	lenZ = depth * 0.5

	v0 = XSIMath.CreateVector3(lenX, 0, lenZ)
	v1 = XSIMath.CreateVector3(lenX, 0, lenZ*-1)
	v2 = XSIMath.CreateVector3(lenX*-1, 0, lenZ*-1)
	v3 = XSIMath.CreateVector3(lenX*-1, 0, lenZ)

	points = getPointArrayWithOffset([v0, v1, v2, v3], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, True, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a FLOWER shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def flower(parent, name="flower", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width

	v0 = XSIMath.CreateVector3(0, -dlen, 0)
	v1 = XSIMath.CreateVector3(-dlen * .4, dlen * .4, 0)
	v2 = XSIMath.CreateVector3(dlen, 0, 0)
	v3 = XSIMath.CreateVector3(-dlen * .4, -dlen * .4, 0)
	v4 = XSIMath.CreateVector3(0, dlen, 0)
	v5 = XSIMath.CreateVector3(dlen * .4, -dlen * .4, 0)
	v6 = XSIMath.CreateVector3(-dlen, 0, 0)
	v7 = XSIMath.CreateVector3(dlen * .4, dlen * .4, 0)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, True, 3, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0],color[1],color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a CIRCLE shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def circle(parent, name="circle", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width * 0.5

	v0 = XSIMath.CreateVector3(0, 0, -dlen * 1.108)
	v1 = XSIMath.CreateVector3(dlen * .78, 0, -dlen * .78)
	v2 = XSIMath.CreateVector3(dlen * 1.108, 0, 0)
	v3 = XSIMath.CreateVector3(dlen * .78, 0, dlen * .78)
	v4 = XSIMath.CreateVector3(0, 0, dlen * 1.108)
	v5 = XSIMath.CreateVector3(-dlen * .78, 0, dlen * .78)
	v6 = XSIMath.CreateVector3(-dlen * 1.108, 0, 0)
	v7 = XSIMath.CreateVector3(-dlen * .78, 0, -dlen * .78)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, True, 3, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a CYLINDER shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def cylinder(parent, name="cylinder", width=1, heigth=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width * .5
	dhei = heigth * .5

	# upper circle
	v0 = XSIMath.CreateVector3(0, dhei, -dlen * 1.108)
	v1 = XSIMath.CreateVector3(dlen * .78, dhei, -dlen * .78)
	v2 = XSIMath.CreateVector3(dlen * 1.108, dhei, 0)
	v3 = XSIMath.CreateVector3(dlen * .78, dhei, dlen * .78)
	v4 = XSIMath.CreateVector3(0, dhei, dlen * 1.108)
	v5 = XSIMath.CreateVector3(-dlen * .78, dhei, dlen * .78)
	v6 = XSIMath.CreateVector3(-dlen * 1.108, dhei, 0)
	v7 = XSIMath.CreateVector3(-dlen * .78, dhei, -dlen * .78)

	# lower circle
	v8 = XSIMath.CreateVector3(0, -dhei, -dlen * 1.108)
	v9 = XSIMath.CreateVector3(dlen * .78, -dhei, -dlen * .78)
	v10 = XSIMath.CreateVector3(dlen * 1.108, -dhei, 0)
	v11 = XSIMath.CreateVector3(dlen * .78, -dhei, dlen * .78)
	v12 = XSIMath.CreateVector3(0, -dhei, dlen * 1.108)
	v13 = XSIMath.CreateVector3(-dlen * .78, -dhei, dlen * .78)
	v14 = XSIMath.CreateVector3(-dlen * 1.108, -dhei, 0)
	v15 = XSIMath.CreateVector3(-dlen * .78, -dhei, -dlen * .78)

	# curves
	v16 = XSIMath.CreateVector3(0, dhei, -dlen)
	v17 = XSIMath.CreateVector3(0, -dhei, -dlen)
	v18 = XSIMath.CreateVector3(0, -dhei, dlen)
	v19 = XSIMath.CreateVector3(0, dhei, dlen)

	v20 = XSIMath.CreateVector3(dlen, dhei, 0)
	v21 = XSIMath.CreateVector3(dlen, -dhei, 0)
	v22 = XSIMath.CreateVector3(-dlen, -dhei, 0)
	v23 = XSIMath.CreateVector3(-dlen, dhei, 0)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23], pos_offset, rot_offset)

	kn = [0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4]
	ncp = [8, 8, 4, 4]
	nkn = [9, 9, 5, 5]
	closed = [True, True, True, True]
	degree = [3, 3, 1, 1]
	param = [siConstants.siNonUniformParameterization] * 4

	curve = parent.AddNurbsCurveList2(4, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a COMPAS shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def compas(parent, name="compas", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width * 0.5

	division = 24
	r = XSIMath.CreateRotation(0, (2*math.pi)/(division+0.0), 0)

	point_pos = []
	v = XSIMath.CreateVector3(0, 0, dlen)

	for i in range(division):
		if i == division/2:
			w = XSIMath.CreateVector3(v.X, v.Y, v.Z-dlen*.4)
		else:
			w = XSIMath.CreateVector3(v.X, v.Y, v.Z)
		point_pos.append(w)
		v.MulByRotationInPlace(r)

	v = XSIMath.CreateVector3(0, 0, dlen*.95)
	for i in range(division):
		point_pos.append(XSIMath.CreateVector3(v.X, v.Y, v.Z))
		v.MulByRotationInPlace(r)

	points = getPointArrayWithOffset(point_pos, pos_offset, rot_offset)

	kn = None
	ncp = [division, division]
	nkn = [None, None]
	closed = [True, True]
	degree = [1, 1]
	param = [siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(2, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a FOIL shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param length Float - Length of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def foil(parent, name="foil", width=1, length=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width * 0.5

	# line
	v0 = XSIMath.CreateVector3(0, 0, 0)
	v1 = XSIMath.CreateVector3(length, 0, 0)

	# circle
	v2 = XSIMath.CreateVector3(0, 0, -dlen * 1.108)
	v3 = XSIMath.CreateVector3(0, dlen * .78, -dlen * .78)
	v4 = XSIMath.CreateVector3(0, dlen * 1.108, 0)
	v5 = XSIMath.CreateVector3(0, dlen * .78, dlen * .78)
	v6 = XSIMath.CreateVector3(0, 0, dlen * 1.108)
	v7 = XSIMath.CreateVector3(0, -dlen * .78, dlen * .78)
	v8 = XSIMath.CreateVector3(0, -dlen * 1.108, 0)
	v9 = XSIMath.CreateVector3(0, -dlen * .78, -dlen * .78)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9], pos_offset, rot_offset)

	kn = None
	ncp = [2, 8]
	nkn = [2, 5]
	closed = [False, True]
	degree = [1, 3]
	param = [siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(2, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a DIAMOND shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def diamond(parent, name="diamond", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width * 0.5

	top = XSIMath.CreateVector3(0, dlen, 0)
	pp = XSIMath.CreateVector3(dlen, 0, dlen)
	pN = XSIMath.CreateVector3(dlen, 0, dlen*-1)
	Np = XSIMath.CreateVector3(dlen*-1, 0, dlen)
	NN = XSIMath.CreateVector3(dlen*-1, 0, dlen*-1)
	bottom = XSIMath.CreateVector3(0, -dlen, 0)

	points = getPointArrayWithOffset([pp, top, pN, pp, Np, top, NN, Np, NN, pN, bottom, NN, bottom, Np, bottom, pp], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, False, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a LEASH shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param attachedPos SIVector3 - Position of the end of the leash.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def leash(parent, name="leash", width=1, attachedPos=XSIMath.CreateVector3(), color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width * 0.5

	# leash
	mInv = t.Matrix4
	mInv.InvertInPlace()

	v = XSIMath.CreateVector3()
	v.MulByMatrix4(attachedPos, mInv)
	vNull = XSIMath.CreateVector3()

	# diamond
	top = XSIMath.CreateVector3(0, dlen, 0)
	pp = XSIMath.CreateVector3(dlen, 0, dlen)
	pN = XSIMath.CreateVector3(dlen, 0, dlen*-1)
	Np = XSIMath.CreateVector3(dlen*-1, 0, dlen)
	NN = XSIMath.CreateVector3(dlen*-1, 0, dlen*-1)
	bottom = XSIMath.CreateVector3(0, -dlen, 0)

	points = getPointArrayWithOffset([v, vNull, pp, top, pN, pp, Np, top, NN, Np, NN, pN, bottom, NN, bottom, Np, bottom, pp], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, False, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a CUBE WITH PEAK shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def cubewithpeak(parent, name="cubewithpeak", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width * 0.5

	peak = XSIMath.CreateVector3(0, width, 0)
	ppp = XSIMath.CreateVector3(dlen, dlen, dlen)
	ppN = XSIMath.CreateVector3(dlen, dlen, dlen*-1)
	pNp = XSIMath.CreateVector3(dlen, 0, dlen)
	Npp = XSIMath.CreateVector3(dlen*-1, dlen, dlen)
	pNN = XSIMath.CreateVector3(dlen, 0, dlen*-1)
	NNp = XSIMath.CreateVector3(dlen*-1, 0, dlen)
	NpN = XSIMath.CreateVector3(dlen*-1, dlen, dlen*-1)
	NNN = XSIMath.CreateVector3(dlen*-1, 0, dlen*-1)

	points = getPointArrayWithOffset([peak, ppp, ppN, peak, NpN, ppN, NpN, peak, Npp, NpN, NNN, NNp, Npp, NpN, Npp, ppp, pNp, NNp, pNp, pNN, ppN, pNN, NNN], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, False, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a SPHERE shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def sphere(parent, name="sphere", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width

	v0 = XSIMath.CreateVector3(0.75*dlen, 0, 0)
	v1 = XSIMath.CreateVector3(0, -.75*dlen, 0)
	v2 = XSIMath.CreateVector3(-.75*dlen, 0, 0)
	v3 = XSIMath.CreateVector3(0, 0.75*dlen, 0)
	v4 = XSIMath.CreateVector3(0.75*dlen, 0, 0)
	v5 = XSIMath.CreateVector3(0, 0, -.75*dlen)
	v6 = XSIMath.CreateVector3(-.75*dlen, 0, 0)
	v7 = XSIMath.CreateVector3(0, 0, 0.75*dlen)
	v8 = XSIMath.CreateVector3(0, 0, 0.75*dlen)
	v9 = XSIMath.CreateVector3(0, -.75*dlen, 0)
	v10 = XSIMath.CreateVector3(0, 0, -.75*dlen)
	v11 = XSIMath.CreateVector3(0, 0.75*dlen, 0)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11], pos_offset, rot_offset)

	kn = [0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4]
	ncp = [4, 4, 4]
	nkn = [5, 5, 5]
	closed = [True, True, True]
	degree = [3, 3, 3]
	param = [siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(3, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a ARROW shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def arrow(parent, name="arrow", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):
	dlen = width * 0.5

	v0 = XSIMath.CreateVector3(0, 0.3*dlen, -dlen)
	v1 = XSIMath.CreateVector3(0, 0.3*dlen, 0.3*dlen)
	v2 = XSIMath.CreateVector3(0, 0.6*dlen, 0.3*dlen)
	v3 = XSIMath.CreateVector3(0, 0, dlen)
	v4 = XSIMath.CreateVector3(0, -0.6*dlen, 0.3*dlen)
	v5 = XSIMath.CreateVector3(0, -0.3*dlen, 0.3*dlen)
	v6 = XSIMath.CreateVector3(0, -0.3*dlen, -dlen)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, True, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a CROSS ARROW shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def crossarrow(parent, name="crossArrow", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width * 0.5

	v0 = XSIMath.CreateVector3(0.2*dlen, 0, 0.2*dlen)
	v1 = XSIMath.CreateVector3(0.2*dlen, 0, 0.6*dlen)
	v2 = XSIMath.CreateVector3(0.4*dlen, 0, 0.6*dlen)
	v3 = XSIMath.CreateVector3(0, 0, dlen)
	v4 = XSIMath.CreateVector3(-0.4*dlen, 0, 0.6*dlen)
	v5 = XSIMath.CreateVector3(-0.2*dlen, 0, 0.6*dlen)
	v6 = XSIMath.CreateVector3(-0.2*dlen, 0, 0.2*dlen)
	v7 = XSIMath.CreateVector3(-0.6*dlen, 0, 0.2*dlen)
	v8 = XSIMath.CreateVector3(-0.6*dlen, 0, 0.4*dlen)
	v9 = XSIMath.CreateVector3(-dlen, 0, 0)
	v10 = XSIMath.CreateVector3(-0.6*dlen, 0, -0.4*dlen)
	v11 = XSIMath.CreateVector3(-0.6*dlen, 0, -0.2*dlen)
	v12 = XSIMath.CreateVector3(-0.2*dlen, 0, -0.2*dlen)
	v13 = XSIMath.CreateVector3(-0.2*dlen, 0, -0.6*dlen)
	v14 = XSIMath.CreateVector3(-0.4*dlen, 0, -0.6*dlen)
	v15 = XSIMath.CreateVector3(0, 0, -dlen)
	v16 = XSIMath.CreateVector3(0.4*dlen, 0, -0.6*dlen)
	v17 = XSIMath.CreateVector3(0.2*dlen, 0, -0.6*dlen)
	v18 = XSIMath.CreateVector3(0.2*dlen, 0, -0.2*dlen)
	v19 = XSIMath.CreateVector3(0.6*dlen, 0, -0.2*dlen)
	v20 = XSIMath.CreateVector3(0.6*dlen, 0, -0.4*dlen)
	v21 = XSIMath.CreateVector3(dlen, 0, 0)
	v22 = XSIMath.CreateVector3(0.6*dlen, 0, 0.4*dlen)
	v23 = XSIMath.CreateVector3(0.6*dlen, 0, 0.2*dlen)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, True, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a BENDED ARROW shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def bendedarrow(parent, name="bendedArrow", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	length = width

	v0 = XSIMath.CreateVector3(-.3*length, .90*length, .1*length)
	v1 = XSIMath.CreateVector3(-.2*length, length, .1*length)
	v2 = XSIMath.CreateVector3(0, 1.05*length, .1*length)
	v3 = XSIMath.CreateVector3(.2*length, length, .1*length)
	v4 = XSIMath.CreateVector3(.3*length, .90*length, .1*length)

	v5 = XSIMath.CreateVector3(-.3*length, .90*length, -.1*length)
	v6 = XSIMath.CreateVector3(-.2*length, length, -.1*length)
	v7 = XSIMath.CreateVector3(0, 1.05*length, -.1*length)
	v8 = XSIMath.CreateVector3(.2*length, length, -.1*length)
	v9 = XSIMath.CreateVector3(.3*length, .90*length, -.1*length)

	v10 = XSIMath.CreateVector3(-.3*length, .90*length, .1*length)
	v11 = XSIMath.CreateVector3(-.3*length, .90*length, .2*length)
	v12 = XSIMath.CreateVector3(-.4*length, .70*length, 0)
	v13 = XSIMath.CreateVector3(-.3*length, .90*length, -.2*length)
	v14 = XSIMath.CreateVector3(-.3*length, .90*length, -.1*length)

	v15 = XSIMath.CreateVector3(.3*length, .90*length, -.1*length)
	v16 = XSIMath.CreateVector3(.3*length, .90*length, -.2*length)
	v17 = XSIMath.CreateVector3(.4*length, .70*length, 0)
	v18 = XSIMath.CreateVector3(.3*length, .90*length, .2*length)
	v19 = XSIMath.CreateVector3(.3*length, .90*length, .1*length)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19], pos_offset, rot_offset)

	kn = None
	ncp = [5, 5, 5, 5]
	nkn = [7, 7, 5, 5]
	closed = [False, False, False, False]
	degree = [3, 3, 1, 1]
	param = [siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(4, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a BENDED ARROW shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def bendedarrow2(parent, name="bendedArrow2", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):
	dlen = width * 0.5

	division = 6
	r = XSIMath.CreateRotation((-math.pi)/(division-1.0), 0, 0)

	point_pos = []
	v = XSIMath.CreateVector3(0, 0, dlen)

	for i in range(division):
		w = XSIMath.CreateVector3(v.X, v.Y, v.Z)
		point_pos.append(w)
		v.MulByRotationInPlace(r)

	point_pos.append(XSIMath.CreateVector3(0, 0, dlen))
	point_pos.append(XSIMath.CreateVector3(0, width*.12, width*.35))
	point_pos.append(XSIMath.CreateVector3(0, width*.19, width*.53))

	point_pos.append(XSIMath.CreateVector3(0, 0, -dlen))
	point_pos.append(XSIMath.CreateVector3(0, width*.12, -width*.35))
	point_pos.append(XSIMath.CreateVector3(0, width*.19, -width*.53))

	points = getPointArrayWithOffset(point_pos, pos_offset, rot_offset)

	kn = None
	ncp = [6, 3, 3]
	nkn = [8, 4, 4]
	closed = [False, True, True]
	degree = [3, 1, 1]
	param = [siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(3, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a CROSS shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def cross(parent, name="cross", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	width = width * 0.35
	offset1 = width * .5
	offset2 = width * 1.5

	v0 = XSIMath.CreateVector3(width, offset2, 0)
	v1 = XSIMath.CreateVector3(offset2, width, 0)
	v2 = XSIMath.CreateVector3(offset1, 0, 0)

	v3 = XSIMath.CreateVector3(offset2, -width, 0)
	v4 = XSIMath.CreateVector3(width, -offset2, 0)
	v5 = XSIMath.CreateVector3(0, -offset1, 0)

	v6 = XSIMath.CreateVector3(-width, -offset2, 0)
	v7 = XSIMath.CreateVector3(-offset2, -width, 0)
	v8 = XSIMath.CreateVector3(-offset1, 0, 0)

	v9 = XSIMath.CreateVector3(-offset2, width, 0)
	v10 = XSIMath.CreateVector3(-width, offset2, 0)
	v11 = XSIMath.CreateVector3(0, offset1, 0)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, True, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with GLASSES shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def glasses(parent, name="glasses", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	width = width * 0.5
	height = width * .25
	radius = width * .125

	offsety = XSIMath.CreateVector3(0, -height, 0)

	r = XSIMath.CreateRotation()
	angle = XSIMath.CreateVector3()
	addrot = 180/5.0

	point_pos = []
	for i in range(6):
		p = XSIMath.CreateVector3(radius, 0, 0)
		p.MulByRotationInPlace(r)

		p.AddInPlace(offsety)

		point_pos.append(p)

		angle.Set(angle.X, angle.Y, angle.Z + XSIMath.DegreesToRadians(addrot))
		r.SetFromXYZAngles(angle)

	point_pos.append(XSIMath.CreateVector3(-radius, -height, 0))
	point_pos.append(XSIMath.CreateVector3(-width, -height, 0))
	point_pos.append(XSIMath.CreateVector3(-width, height, 0))
	point_pos.append(XSIMath.CreateVector3(width, height, 0))
	point_pos.append(XSIMath.CreateVector3(width, -height, 0))
	point_pos.append(XSIMath.CreateVector3(radius, -height, 0))

	points = getPointArrayWithOffset(point_pos, pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, False, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a LOOKAT shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def lookat(parent, name="lookat", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	v0 = XSIMath.CreateVector3(0.6*width, 0, 0)
	v1 = XSIMath.CreateVector3(-0.6*width, 0, 0)
	v2 = XSIMath.CreateVector3(0, 0, 0)
	v3 = XSIMath.CreateVector3(0, .6*width, 0)
	v4 = XSIMath.CreateVector3(0, 0, 0)
	v5 = XSIMath.CreateVector3(0, 0, -width)

	v6 = XSIMath.CreateVector3(-.3*width, 0, 0)
	v7 = XSIMath.CreateVector3(-.3*width, 0.1*width, 0)
	v8 = XSIMath.CreateVector3(-0.18*width, 0.31*width, 0)
	v9 = XSIMath.CreateVector3(0.18*width, 0.31*width, 0)
	v10 = XSIMath.CreateVector3(0.3*width, 0.1*width, 0)
	v11 = XSIMath.CreateVector3(0.3*width, 0, 0)

	v12 = XSIMath.CreateVector3(0.3*width, 0, 0)
	v13 = XSIMath.CreateVector3(0, 0, -0.6*width)
	v14 = XSIMath.CreateVector3(-0.3*width, 0, 0)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14], pos_offset, rot_offset)

	kn = [0, 1, 2, 3, 4, 5, 0, 0, 0, 1, 2, 3, 3, 3, 0, 1, 2]
	ncp = [6, 6, 3]
	nkn = [6, 8, 3]
	closed = [False, False, False]
	degree = [1, 3, 1]
	param = [siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(3, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a EYE ARROW shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def eyearrow(parent, name="eyeArrow", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	v0 = XSIMath.CreateVector3(.049*width, 0, 0)
	v1 = XSIMath.CreateVector3(-.049*width, 0, 0)
	v2 = XSIMath.CreateVector3(0, 0, 0)
	v3 = XSIMath.CreateVector3(0, -.049*width, 0)
	v4 = XSIMath.CreateVector3(0, .293*width, 0)
	v5 = XSIMath.CreateVector3(.06*width, .293*width, 0)
	v6 = XSIMath.CreateVector3(-.06*width, .293*width, 0)
	v7 = XSIMath.CreateVector3(.06*width, .293*width, 0)

	v8 = XSIMath.CreateVector3(0, .233*width, 0)
	v9 = XSIMath.CreateVector3(-.06*width, .293*width, 0)
	v10 = XSIMath.CreateVector3(0, .353*width, 0)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10], pos_offset, rot_offset)

	kn = [0, 1, 2, 3, 4, 4.064000040040823, 4.188000040040823, 0, 1, 2, 3, 4]
	ncp = [7, 4]
	nkn = [7, 5]
	closed = [False, True]
	degree = [1, 3]
	param = [siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(2, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a ANGLE SURVEY shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def anglesurvey(parent, name="angleSurvey", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	v0 = XSIMath.CreateVector3(.25*width, 0, width)
	v1 = XSIMath.CreateVector3(0, 0, 0)
	v2 = XSIMath.CreateVector3(-0.25*width, 0, width)

	v3 = XSIMath.CreateVector3(-0.126*width, 0, 0.305*width)
	v4 = XSIMath.CreateVector3(-0.1*width, 0, 0.316*width)
	v5 = XSIMath.CreateVector3(-0.044*width, 0, 0.331*width)
	v6 = XSIMath.CreateVector3(0.044*width, 0, 0.331*width)
	v7 = XSIMath.CreateVector3(0.1*width, 0, 0.316*width)
	v8 = XSIMath.CreateVector3(0.126*width, 0, 0.305*width)

	v9 = XSIMath.CreateVector3(0, 0, 0)
	v10 = XSIMath.CreateVector3(0, 0, 0)
	v11 = XSIMath.CreateVector3(0, 0, 0)
	v12 = XSIMath.CreateVector3(0, -0.15*width, 0)
	v13 = XSIMath.CreateVector3(0, 0.15*width, 0)
	v14 = XSIMath.CreateVector3(0, 0, 0)
	v15 = XSIMath.CreateVector3(0.15*width, 0, 0)
	v16 = XSIMath.CreateVector3(-0.15*width, 0, 0)
	v17 = XSIMath.CreateVector3(0, 0, 0)
	v18 = XSIMath.CreateVector3(0, 0, -0.15*width)

	v19 = XSIMath.CreateVector3(0.15*width, 0, 0)
	v20 = XSIMath.CreateVector3(0, -0.15*width, 0)
	v21 = XSIMath.CreateVector3(-0.15*width, 0, 0)
	v22 = XSIMath.CreateVector3(0, 0.15*width, 0)

	v23 = XSIMath.CreateVector3(0.15*width, 0, 0)
	v24 = XSIMath.CreateVector3(0, 0, -0.15*width)
	v25 = XSIMath.CreateVector3(-0.15*width, 0, 0)
	v26 = XSIMath.CreateVector3(0, 0, 0.15*width)

	v27 = XSIMath.CreateVector3(0, 0, 0.15*width)
	v28 = XSIMath.CreateVector3(0, -0.15*width, 0)
	v29 = XSIMath.CreateVector3(0, 0, -0.15*width)
	v30 = XSIMath.CreateVector3(0, 0.15*width, 0)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30], pos_offset, rot_offset)

	kn = [0, 1, 2, 0, 0, 0, 1, 2, 3, 3, 3, 0, 1, 7, 13, 14, 15, 16, 17, 18, 19, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4]
	ncp = [3, 6, 10, 4, 4, 4]
	nkn = [3, 8, 10, 5, 5, 5]
	closed = [False, False, False, True, True, True]
	degree = [1, 3, 1, 3, 3, 3]
	param = [siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(6, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a EYE BALL shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def eyeball(parent, name="eyeBall", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	v0 = XSIMath.CreateVector3(0, -0.187*width, 0.485*width)
	v1 = XSIMath.CreateVector3(0.187*width, 0, 0.485*width)
	v2 = XSIMath.CreateVector3(0, 0.187*width, 0.485*width)
	v3 = XSIMath.CreateVector3(-0.187*width, 0, 0.485*width)

	v4 = XSIMath.CreateVector3(0, -0.085*width, 0.496*width)
	v5 = XSIMath.CreateVector3(0.085*width, 0, 0.496*width)
	v6 = XSIMath.CreateVector3(0, 0.085*width, 0.496*width)
	v7 = XSIMath.CreateVector3(-0.085*width, 0, 0.496*width)

	v8 = XSIMath.CreateVector3(-0.5*width, 0, 0)
	v9 = XSIMath.CreateVector3(0.5*width, 0, 0)
	v10 = XSIMath.CreateVector3(0, 0, 0)
	v11 = XSIMath.CreateVector3(0, -0.5*width, 0)
	v12 = XSIMath.CreateVector3(0, 0.5*width, 0)
	v13 = XSIMath.CreateVector3(0, 0, 0)
	v14 = XSIMath.CreateVector3(0, 0, -0.5*width)
	v15 = XSIMath.CreateVector3(0, 0, 1.879*width)
	v16 = XSIMath.CreateVector3(-0.05*width, 0, 1.779*width)
	v17 = XSIMath.CreateVector3(0.05*width, 0, 1.779*width)
	v18 = XSIMath.CreateVector3(0, 0, 1.879*width)

	v19 = XSIMath.CreateVector3(0, 0.75*width, 0)
	v20 = XSIMath.CreateVector3(0.75*width, 0, 0)
	v21 = XSIMath.CreateVector3(0, -0.75*width, 0)
	v22 = XSIMath.CreateVector3(-0.75*width, 0, 0)

	v23 = XSIMath.CreateVector3(0, 0.75*width, 0)
	v24 = XSIMath.CreateVector3(0, 0, -0.75*width)
	v25 = XSIMath.CreateVector3(0, -0.75*width, 0)
	v26 = XSIMath.CreateVector3(0, 0, 0.75*width)

	v27 = XSIMath.CreateVector3(0.75*width, 0, 0)
	v28 = XSIMath.CreateVector3(0, 0, -0.75*width)
	v29 = XSIMath.CreateVector3(-0.75*width, 0, 0)
	v30 = XSIMath.CreateVector3(0, 0, 0.75*width)

	v31 = XSIMath.CreateVector3(-0.354*width, 0, 0.335*width)
	v32 = XSIMath.CreateVector3(0, 0.375*width, 0.706*width)
	v33 = XSIMath.CreateVector3(0.354*width, 0, 0.335*width)
	v34 = XSIMath.CreateVector3(0.354*width, 0, 0.335*width)

	v35 = XSIMath.CreateVector3(-0.354*width, 0, 0.335*width)
	v36 = XSIMath.CreateVector3(0, -0.375*width, 0.706*width)
	v37 = XSIMath.CreateVector3(0.354*width, 0, 0.335*width)
	v38 = XSIMath.CreateVector3(0.354*width, 0, 0.335*width)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37, v38], pos_offset, rot_offset)

	kn = [-2, -1, 0, 1, 2, -2, -1, 0, 1, 2, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -2, -1, 0,
		  1, 2, -2, -1, 0, 1, 2, -2, -1, 0, 1, 2, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1]
	ncp = [4, 4, 11, 4, 4, 4, 4, 4]
	nkn = [5, 5, 11, 5, 5, 5, 6, 6]
	closed = [True, True, False, True, True, True, False, False]
	degree = [3, 3, 1, 3, 3, 3, 3, 3]
	param = [siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization,
			 siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(8, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a RECTANGLE CUBE shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def rectanglecube(parent, name="rectangleCube", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	dlen = width * .2

	# p is positive, N is negative
	ppp = XSIMath.CreateVector3(dlen, dlen, width)
	ppN = XSIMath.CreateVector3(dlen, dlen, 0)
	pNp = XSIMath.CreateVector3(dlen, dlen*-1, width)
	Npp = XSIMath.CreateVector3(dlen*-1, dlen, width)
	pNN = XSIMath.CreateVector3(dlen, dlen*-1, 0)
	NNp = XSIMath.CreateVector3(dlen*-1, dlen*-1, width)
	NpN = XSIMath.CreateVector3(dlen*-1, dlen, 0)
	NNN = XSIMath.CreateVector3(dlen*-1, dlen*-1, 0)

	points = getPointArrayWithOffset([ppp, ppN, NpN, NNN, NNp, Npp, NpN, Npp, ppp, pNp, NNp, pNp, pNN, ppN, pNN, NNN], pos_offset, rot_offset)

	curve = parent.AddNurbsCurve(points, None, False, 1, siConstants.siNonUniformParameterization, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a MAN shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def man(parent, name="man", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):

	# pants
	v0 = XSIMath.CreateVector3(.12*width, .40*width, 0)
	v1 = XSIMath.CreateVector3(.15*width, 0, 0)
	v2 = XSIMath.CreateVector3(.075*width, 0, 0)
	v3 = XSIMath.CreateVector3(0, .3*width, 0)
	v4 = XSIMath.CreateVector3(-.075*width, 0, 0)
	v5 = XSIMath.CreateVector3(-.15*width, 0, 0)
	v6 = XSIMath.CreateVector3(-.12*width, .40*width, 0)

	# spine
	v7 = XSIMath.CreateVector3(.15*width, .75*width, 0)
	v8 = XSIMath.CreateVector3(.12*width, .60*width, 0)
	v9 = XSIMath.CreateVector3(.12*width, .42*width, 0)
	v10 = XSIMath.CreateVector3(-.12*width, .42*width, 0)
	v11 = XSIMath.CreateVector3(-.12*width, .60*width, 0)
	v12 = XSIMath.CreateVector3(-.15*width, .75*width, 0)

	# left arm
	v13 = XSIMath.CreateVector3(.17*width, .75*width, 0)
	v14 = XSIMath.CreateVector3(.35*width, .40*width, 0)
	v15 = XSIMath.CreateVector3(.30*width, .35*width, 0)
	v16 = XSIMath.CreateVector3(.14*width, .60*width, 0)

	# right arm
	v17 = XSIMath.CreateVector3(-.17*width, .75*width, 0)
	v18 = XSIMath.CreateVector3(-.35*width, .40*width, 0)
	v19 = XSIMath.CreateVector3(-.30*width, .35*width, 0)
	v20 = XSIMath.CreateVector3(-.14*width, .60*width, 0)

	# head
	v21 = XSIMath.CreateVector3(.06*width, width, 0)
	v22 = XSIMath.CreateVector3(.12*width, width, 0)
	v23 = XSIMath.CreateVector3(.06*width, .78*width, 0)
	v24 = XSIMath.CreateVector3(-.06*width, .78*width, 0)
	v25 = XSIMath.CreateVector3(-.12*width, width, 0)
	v26 = XSIMath.CreateVector3(-.06*width, width, 0)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26], pos_offset, rot_offset)

	kn = [0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6]
	ncp = [7, 6, 4, 4, 6]
	nkn = [8, 7, 5, 5, 7]
	closed = [True, True, True, True, True]
	degree = [1, 1, 1, 1, 3]
	param = [siConstants.siNonUniformParameterization] * 5

	curve = parent.AddNurbsCurveList2(5, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# ========================================================
# Create a curve with a NULL shape.
# @param parent X3DObject - The parent object of the newly created curve.
# @param name String - Name of the curve.
# @param width Float - Width of the shape.
# @param color List of Float - RGB color of the curve.
# @param t SITransform - The global transformation of the curve.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def null(parent, name="null", width=1, color=[0, 0, 0], t=XSIMath.CreateTransform(), pos_offset=XSIMath.CreateVector3(), rot_offset=XSIMath.CreateRotation()):
	dlen = width * 0.5

	v0 = XSIMath.CreateVector3(-dlen, 0, 0)
	v1 = XSIMath.CreateVector3(dlen, 0, 0)

	v2 = XSIMath.CreateVector3(0, -dlen, 0)
	v3 = XSIMath.CreateVector3(0, dlen, 0)

	v4 = XSIMath.CreateVector3(0, 0, -dlen)
	v5 = XSIMath.CreateVector3(0, 0, dlen)

	points = getPointArrayWithOffset([v0, v1, v2, v3, v4, v5], pos_offset, rot_offset)

	kn = None
	ncp = [2, 2, 2]
	nkn = [2, 2, 2]
	closed = [False, False, False]
	degree = [1, 1, 1]
	param = [siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization, siConstants.siNonUniformParameterization]

	curve = parent.AddNurbsCurveList2(3, points, ncp, kn, nkn, closed, degree, param, siConstants.siSINurbs, name)
	uti.SetWireColor(curve, color[0], color[1], color[2])
	curve.Kinematics.Global.Transform = t

	return curve


# --------------------------------------------
# Helpers
# --------------------------------------------
# Convert a list of vector to a List of Float and add the position and rotation offset.
# @param point_pos List of SIVector3 - The point position.
# @param pos_offset SIVector3 - The position offset of the curve from its center.
# @param rot_offset SIRotation - The rotation offset of the curve from its center.
def getPointArrayWithOffset(point_pos, pos_offset, rot_offset):

	points = []
	for v in point_pos:
		v.MulByRotationInPlace(rot_offset)
		points.append(v.X + pos_offset.X)
		points.append(v.Y + pos_offset.Y)
		points.append(v.Z + pos_offset.Z)
		points.append(1)

	return points
