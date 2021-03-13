# -------------------------------------------------------------------
# Cleaner
# -------------------------------------------------------------------
from Globals import *
import Utils
import ICETree


# ---------------------------------------------------
# Clean Hair
# ---------------------------------------------------
def CleanHair(cage):
	prim = cage.ActivePrimitive
	history = prim.ConstructionHistory
		
	hasdynamic = history.Find("simulationmarker")
	if hasdynamic:
		dynamictree = prim.ICETrees.Find("SyflexDynamics")
		if not dynamictree:
			dynamictree = prim.ICETrees.Find("SoftBodyDynamics")
		if not dynamictree:
			return
			
		get = dynamictree.DataProviderNodes(0)
		target = ICETree.GetObjectFromReferenceValue(get)
		if target:
			model = target.Model
			grp = model.Groups("PointCache")
			if grp and not Utils.ObjectInGroup(target, grp):
				XSI.DeleteObj(target)
			
		get = dynamictree.DataProviderNodes(1)
		collider = ICETree.GetObjectFromReferenceValue(get)
		if collider:
			model = collider.Model
			grp = model.Groups("PointCache")
			if grp and not Utils.ObjectInGroup(collider, grp):
				XSI.DeleteObj(collider)


# ---------------------------------------------------
# Clean Mesh
# ---------------------------------------------------
def CleanMesh(obj):
	delete = []
	XSI.FreezeModeling(obj)
	mode = 0
	XSI.LogMessage(obj.FullName)
	for c in obj.ActivePrimitive.ConstructionHistory:
		if c.Name == "simulationmarker":
			XSI.DeleteObj(c)
		elif c.Name == "GetMeshData" or c.Name == "SetMeshData":
			CleanHair(obj)
			mode = 1
			continue
		delete.append(c)
	
	for c in obj.ActivePrimitive.Geometry.Clusters:
		# delete pnt clusters except weight map
		if c.Type == "pnt":
			if c.Name.find("WeightMap") == -1:
				delete.append(c)
		# delete all edge cluster
		if c.Type == "edge":
			delete.append(c)
		
	XSI.DeleteObj(delete)
	
	return mode


# ---------------------------------------------------
# Clean Transform
# ---------------------------------------------------
def CleanTransform(obj):
	transform = XSIMath.CreateTransform()
	constraints = obj.Kinematics.Constraints
	if constraints.Count > 0:
		XSI.DeleteObj(constraints)
	local = obj.Kinematics.Local
	for parameter in local.Parameters:
		parameter.Disconnect()
		
	glob = obj.Kinematics.Global
	for parameter in glob.Parameters:
		parameter.Disconnect()
	glob.PutTransform2(0, transform)
