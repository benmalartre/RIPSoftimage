# -------------------------------------------------------------------
# Property
# -------------------------------------------------------------------
from win32com.client import constants as siConstants
from Globals import XSI
from Globals import XSIFactory
import Utils as uti


# ----------------------------------------------------
# Get Properties by Type
# ----------------------------------------------------
def GetProperties(prop_type, model=None):
	filtered = XSIFactory.CreateActiveXObject("XSI.Collection")
	props = XSI.FindObjects(None, "{76332571-D242-11d0-B69C-00AA003B3EA6}")
	for p in props:
		if p.Type == prop_type:
			if model:
				if p.Parent3DObject.Model.IsEqualTo(model):
					filtered.Add(p)
			else:
				filtered.Add(p)
	return filtered


# ----------------------------------------------------
# Update ListItemsUI
# ----------------------------------------------------
def UpdateListItemsUI(item, param, uiitems):
	# update UI list
	item.UIItems = uiitems

	# look for the previously selected value
	old = param.Value
	i = 0
	for i in range(0, len(uiitems), 2):
		if uiitems[i+1] == old:
			break

	if i >= len(uiitems):
		# not found, select first entry
		newvalue = ""
		if len(uiitems) > 0:
			newvalue = uiitems[1]
		if not param.Value == newvalue:
			param.Value = newvalue
			return True

	return False


# ----------------------------------------------------
# Build List WeightMaps
# ----------------------------------------------------
def BuildListWeightMaps(obj, prop, clsname):
	geom = obj.ActivePrimitive.Geometry
	list = []
	cnt = 0
	
	if clsname:
		cls = geom.Clusters(clsname)
		if cls:
			for w in cls.LocalProperties.Filter("wtmap"):
				list.append(w.Name)
				list.append(cnt)
				cnt += 1
			return list
			
	# check all cluster for weightmap already exists
	for cls in geom.Clusters.Filter("pnt"):
		for w in cls.LocalProperties.Filter("wtmap"):
				list.append(w.Name)
				list.append(cnt)
				cnt += 1
	
	return list	


# ----------------------------------------------------
# Get Logo
# ----------------------------------------------------
def GetLogo(prop):
	itemName = "ClothLogo"
	path = "E:\\Projects\\RnD\\bWorkgroup\\Application\\Plugins\\Cloth\\Icons\\Cloth.bmp"
	layout = prop.PPGLayout
	item = layout.AddItem(itemName, itemName, siConstants.siControlBitmap)
	item.SetAttribute(siConstants.siUIFilePath, path)
	item.SetAttribute(siConstants.siUINoLabel, True)


# ----------------------------------------------------
# Find Model Owner of this PPG
# ----------------------------------------------------
def FindPPGModel(prop):
	obj = prop.Parent3DObject
	if obj.Type == "#model":
		model = obj
	else:
		model = obj.Model
	return model


# ----------------------------------------------------
# Find Child From Parameter Value
# ----------------------------------------------------
def FindChildFromParameterValue(prop, param):
	value = prop.Parameters(param).Value
	if not value:
		XSI.LogMessage("[FindChildFromParameterValue] : Parameter " + param + " is Empty... Sorry!")
		return None
	
	model = FindPPGModel(prop)
	XSI.LogMessage(model)
	out = model.FindChild(value)
	return out


# ----------------------------------------------------
# Find Property From Parameter Value
# ----------------------------------------------------
def FindPropertyFromParameterValue(prop, param, parent):
	value = prop.Parameters(param).Value
	if not value:
		XSI.LogMessage("[FindPropertyFromParameterValue] : Parameter " + param + " is Empty... Skipped!")
		return None
	
	out = parent.Properties(value)
	return out


# ----------------------------------------------------
# Find Cluster From Parameter Value
# ----------------------------------------------------
def FindClusterFromParameterValue(prop, obj, param):
	cls = obj.ActivePrimitive.Geometry.Clusters
	value = prop.Parameters(param).Value
	if not value:
		return None
		
	for c in cls:
		if c.Name == value:
			return c
	return None


# ----------------------------------------------------
# Get Object From Selected UIItem
# ----------------------------------------------------
def GetObjectFromStringParameter(prop, inname):
	clsname = prop.Parameters(inname).Value
	cls = None
	
	if clsname == "":
		return cls
	try:
		cls = XSI.Dictionary.GetObject(clsname)
		return cls
	except:
		return None


# ----------------------------------------------------
# Get Object From Selected UIItem
# ----------------------------------------------------
def GetObjectFromSelectedUIItem(prop, inname):
	obj = prop.Parent3DObject
	if obj.Type == "#model":
		model = obj
	else:
		model = obj.Model
	_list = prop.Parameters(inname+"List").Value
	if _list == "":
		return None
		
	selected = prop.Parameters(inname+"Chooser").Value
	if selected:
		split = _list.split("|")
		split.pop()
		
		out = None
		try:
			out = model.FindChild(selected)
		except:
			XSI.LogMessage("[Property] Can't find Object " + selected + " : Remove it from list...", siConstants.siWarning)
		return out
	return None


# ----------------------------------------------------
# Get ID From Selected UIItem
# ----------------------------------------------------
def GetIDFromSelectedUIItem(prop, inname):
	idx = prop.Parameters(inname+"List").Value + 1
	return idx


# ----------------------------------------------------
# Build List From Tab String
# ----------------------------------------------------
def BuildListFromTabString(names):
	split = names.split("|")
	out = []

	for idx, item in enumerate(split):
		if item:
			out.append(item)
			out.append(idx)
	return out
	

# ----------------------------------------------------
# Add Object List
# ----------------------------------------------------
def AddObjectList(prop, inname):
	layout = prop.PPGLayout
	layout.AddGroup(inname)
	sList = prop.Parameters(inname).Value
	aList = BuildListFromTabString(sList)
	
	item = layout.AddEnumControl(inname+"List", aList, "", siConstants.siControlListBox)
	item.SetAttribute(siConstants.siUINoLabel, True)

	item.UIItems = aList
	layout.AddRow()
	layout.AddButton("Add"+inname, "Add")
	layout.AddButton("Remove"+inname, "Remove")
	layout.EndRow()
	layout.EndGroup()


# ----------------------------------------------------
# Add Object To List
# ----------------------------------------------------
def AddObjectToList(prop, inname, message):
	XSI.LogMessage("Add Object To List Called...")
	
	obj = prop.Parent3DObject
	model = obj.Model
	
	pick = uti.PickElement(siConstants.siPolyMeshFilter, message)
	if pick:
		if not pick.Model.FullName == model.FullName:
			XSI.LogMessage("Picked Object MUST be in same Model as Reference Object", siConstants.siWarning)
			return
			
		if not CheckObjectAlreadyInList(prop, inname, pick.Name):
			prop.Parameters(inname).Value = prop.Parameters(inname).Value + pick.Name+"|"
	
		else:
			XSI.Logmessage(prop.Name + " : " + pick.Name + " already in list ---> skipped...")


# ----------------------------------------------------
# Check Object Already In List
# ----------------------------------------------------
def CheckObjectAlreadyInList(prop, inParamName, inName):
	names = prop.Parameters(inParamName).Value
	if names == "":
		return False
	
	split = names.split("|")
	for s in split:
		if s == inName:
			return True
	return False


# ----------------------------------------------------
# Remove Object Already From List
# ----------------------------------------------------
def RemoveObjectFromList(prop, inname):
	idx = prop.Parameters(inname+"List").Value
	sin = prop.Parameters(inname).Value
	
	split = sin.split("|")
	split.pop()
	keep = [s for i, s in enumerate(split) if not i == idx]
	sout = "|".join(keep)

	prop.Parameters(inname).Value = sout
	prop.Parameters(inname+"List").Value = 0


# ----------------------------------------------------
# Check Parameter Exist
# ----------------------------------------------------
def CheckParameterExist(prop, paramname, paramtype, mode):
	if not prop.Parameters(paramname) and mode == 1:
		item = prop.AddParameter3(paramname, paramtype)
		item.Animatable = False

	elif prop.Parameters(paramname) and not mode:
		prop.RemoveParameter(prop.Parameters(paramname))


# ----------------------------------------------------
# Get Nb Objects in List
# ----------------------------------------------------
def GetNbObjectsInList(prop, listname):
	v = prop.Parameters(listname).Value
	if not v:
		return 0
	split = v.split("|")
	split.pop()
	return len(split)


# ----------------------------------------------------
# Add Collider Object
# ----------------------------------------------------
def AddColliderObject(prop):
	nb = GetNbObjectsInList(prop, "ColliderMeshes")
	idx = str(nb)
	pntcls = "CollidePointCluster"+idx
	polycls = "ColliderPolygonCluster"+idx
	
	CheckParameterExist(prop, pntcls, siConstants.siString, 1)
	CheckParameterExist(prop, polycls, siConstants.siString, 1)


# ----------------------------------------------------
# Remove Collider Object
# ----------------------------------------------------
def RemoveColliderObject(prop):
	nb = GetNbObjectsInList(prop, "ColliderMeshes")
	idx = prop.Parameters("ColliderMeshesList").Value+1

	if idx < nb:
		# transfer infos from next collider if any
		for i in xrange(idx, nb):
			prop.Parameters("CollidePointCluster"+str(i)).Value = prop.Parameters("CollidePointCluster"+str(i+1)).Value
			prop.Parameters("ColliderPolygonCluster"+str(i)).Value = prop.Parameters("ColliderPolygonCluster"+str(i+1)).Value
		
	pntcls = "CollidePointCluster"+str(nb)
	polycls = "ColliderPolygonCluster"+str(nb)
	
	CheckParameterExist(prop, pntcls, siConstants.siString, 0)
	CheckParameterExist(prop, polycls, siConstants.siString, 0)


