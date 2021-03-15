from Globals import *
import Utils
import ICETree


def CleanHair(cage):
	""" Clean hair cage mesh
	:param X3DObject cage: the hair cage mesh
	"""
	prim = cage.ActivePrimitive
	history = prim.ConstructionHistory
		
	hasdynamic = history.Find('simulationmarker')
	if hasdynamic:
		dynamictree = prim.ICETrees.Find('SyflexDynamics')
		if not dynamictree:
			dynamictree = prim.ICETrees.Find('SoftBodyDynamics')
		if not dynamictree:
			return
			
		get = dynamictree.DataProviderNodes(0)
		target = ICETree.GetObjectFromReferenceValue(get)
		if target:
			model = target.Model
			grp = model.Groups('PointCache')
			if grp and not Utils.ObjectInGroup(target, grp):
				XSI.DeleteObj(target)
			
		get = dynamictree.DataProviderNodes(1)
		collider = ICETree.GetObjectFromReferenceValue(get)
		if collider:
			model = collider.Model
			grp = model.Groups('PointCache')
			if grp and not Utils.ObjectInGroup(collider, grp):
				XSI.DeleteObj(collider)


def CleanMesh(obj):
	""" Clean generic 3D mesh
		- freeze modeling
		- delete construction history
		- delete point clusters excepting those containing weight map
		- delete edge clusters
	:param X3DObject obj: the mesh to clean
	"""
	delete = []
	XSI.FreezeModeling(obj)
	mode = 0
	XSI.LogMessage(obj.FullName)
	for c in obj.ActivePrimitive.ConstructionHistory:
		if c.Name == 'simulationmarker':
			XSI.DeleteObj(c)
		elif c.Name == 'GetMeshData' or c.Name == 'SetMeshData':
			CleanHair(obj)
			mode = 1
			continue
		delete.append(c)
	
	for c in obj.ActivePrimitive.Geometry.Clusters:
		# delete pnt clusters except weight map
		if c.Type == 'pnt':
			if c.Name.find('WeightMap') == -1:
				delete.append(c)
		# delete all edge cluster
		if c.Type == 'edge':
			delete.append(c)
		
	XSI.DeleteObj(delete)
	
	return mode


def CleanTransform(obj):
	""" Clean transform
		- delete constraints
		- delete parameters connexions
	:param X3DObject obj: the objetc to clean transform on
	"""
	transform = XSIMath.CreateTransform()
	constraints = obj.Kinematics.Constraints
	if constraints.Count > 0:
		XSI.DeleteObj(constraints)
	kine_local = obj.Kinematics.Local
	for parameter in kine_local.Parameters:
		parameter.Disconnect()
		
	kine_global = obj.Kinematics.Global
	for parameter in kine_global.Parameters:
		parameter.Disconnect()
	kine_global.PutTransform2(0, transform)
