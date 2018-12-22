# -------------------------------------------------------------------
# Cleaner
# -------------------------------------------------------------------
from Globals import *
import Utils as uti
import ICETree as ice


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
		target = ice.GetObjectFromReferenceValue(get)
		if target:
			model = target.Model
			grp = model.Groups("PointCache")
			if grp and not uti.ObjectInGroup(target, grp):
				xsi.DeleteObj(target)
			
		get = dynamictree.DataProviderNodes(1)
		collider = ice.GetObjectFromReferenceValue(get)
		if collider:
			model = collider.Model
			grp = model.Groups("PointCache")
			if grp and not uti.ObjectInGroup(collider, grp):
				xsi.DeleteObj(collider)


# ---------------------------------------------------
# Clean Mesh
# ---------------------------------------------------
def CleanMesh(obj):
	delete = []
	xsi.FreezeModeling(obj)
	mode = 0
	xsi.LogMessage(obj.FullName)
	for c in obj.ActivePrimitive.ConstructionHistory:
		if c.Name == "simulationmarker":
			xsi.DeleteObj(c)
		elif c.Name =="GetMeshData" or c.Name == "SetMeshData":
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
		
	xsi.DeleteObj(delete)
	
	return mode


# ---------------------------------------------------
# Clean Transform
# ---------------------------------------------------
def CleanTransform(obj):
	tra = XSIMath.CreateTransform()
	constraints = obj.Kinematics.Constraints
	if constraints.Count > 0:
		xsi.DeleteObj(constraints)
	local = obj.Kinematics.Local
	for p in local.Parameters:
		p.Disconnect()
		
	glob = obj.Kinematics.Global
	for p in glob.Parameters:
		p.Disconnect()
	glob.PutTransform2(0, tra)