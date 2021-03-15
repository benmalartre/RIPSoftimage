from win32com.client import constants
from Globals import XSI
from Globals import XSIFactory
import Utils


def GetProperties(_type, model=None):
	""" Get all custom properties of propType name under model (or scene)
	:param str _type: property type name
	:param Model model: SDK Model object
	:return XSI.Collection: SDK PPG collection
	"""
	filtered = XSIFactory.CreateActiveXObject('XSI.Collection')
	all_properties = XSI.FindObjects(None, '{76332571-D242-11d0-B69C-00AA003B3EA6}')
	for _property in all_properties:
		if _property.Type == _type:
			if model:
				if _property.Parent3DObject.Model.IsEqualTo(model):
					filtered.Add(_property)
			else:
				filtered.Add(_property)
	return filtered


def UpdateListItemsUI(item, param, elements):
	""" Update list items
	:param PPGItem item: SDK PPGItem object
	:param Parameter param: SDK Parameter object
	:param list elements: ui items list (label, value)
	"""
	item.UIItems = elements
	old_value = param.Value
	i = 0
	for i in range(0, len(elements), 2):
		if elements[i + 1] == old_value:
			break

	if i >= len(elements):
		new_value = ''
		if len(elements) > 0:
			new_value = elements[1]
		if not param.Value == new_value:
			param.Value = new_value
			return True

	return False


def BuildListWeightMaps(obj, clusterName):
	""" Build weight maps list
	:param X3DObject obj: SDK X3DObject
	:param str clusterName: cluster name
	:return list: weight maps list (name, value)
	"""
	geom = obj.ActivePrimitive.Geometry
	all_weight_maps = []
	idx = 0
	
	if clusterName:
		cluster = geom.Clusters(clusterName)
		if cluster:
			for weight_map in cluster.LocalProperties.Filter('wtmap'):
				all_weight_maps.append(weight_map.Name)
				all_weight_maps.append(idx)
				idx += 1
			return all_weight_maps
			
	# check all cluster for weight map already exists
	for cluster in geom.Clusters.Filter('pnt'):
		for weight_map in cluster.LocalProperties.Filter('wtmap'):
			all_weight_maps.append(weight_map.Name)
			all_weight_maps.append(idx)
			idx += 1
	
	return all_weight_maps


def GetLogo(prop):
	name = 'ClothLogo'
	path = 'E:\\Projects\\RnD\\bWorkgroup\\Application\\Plugins\\Cloth\\Icons\\Cloth.bmp'
	layout = prop.PPGLayout
	item = layout.AddItem(name, name, constants.siControlBitmap)
	item.SetAttribute(constants.siUIFilePath, path)
	item.SetAttribute(constants.siUINoLabel, True)


def FindPPGModel(prop):
	obj = prop.Parent3DObject
	if obj.Type == '#model':
		model = obj
	else:
		model = obj.Model
	return model


def FindChildFromParameterValue(prop, param):
	value = prop.Parameters(param).Value
	if not value:
		XSI.LogMessage('[FindChildFromParameterValue] Parameter "{}" is empty !'.format(param))
		return None
	
	model = FindPPGModel(prop)
	XSI.LogMessage(model)
	out = model.FindChild(value)
	return out


def FindPropertyFromParameterValue(prop, param, parent):
	value = prop.Parameters(param).Value
	if not value:
		XSI.LogMessage('[FindPropertyFromParameterValue] Parameter "{}" is empty !'.format(param))
		return None
	
	out = parent.Properties(value)
	return out


def FindClusterFromParameterValue(prop, obj, param):
	all_clusters = obj.ActivePrimitive.Geometry.Clusters
	value = prop.Parameters(param).Value
	if not value:
		return None
		
	for cluster in all_clusters:
		if cluster.Name == value:
			return cluster
	return None


def GetObjectFromStringParameter(prop, name):
	cluster_name = prop.Parameters(name).Value
	cluster = None
	
	if cluster_name == '':
		return cluster
	try:
		cluster = XSI.Dictionary.GetObject(cluster_name)
		return cluster
	except RuntimeError:
		return None


def GetObjectFromSelectedUIItem(prop, name):
	obj = prop.Parent3DObject
	if obj.Type == '#model':
		model = obj
	else:
		model = obj.Model
	_list = prop.Parameters('{}List'.format(name)).Value
	if _list == '':
		return None
		
	selected = prop.Parameters('{}Chooser'.format(name)).Value
	if selected:
		split = _list.split('|')
		split.pop()
		try:
			return model.FindChild(selected)
		except RuntimeError:
			XSI.LogMessage('[Property] Can\'t find Object "{}" !', constants.siWarning)

	return None


def GetIDFromSelectedUIItem(prop, name):
	idx = prop.Parameters('{}List'.format(name)).Value + 1
	return idx


def BuildListFromTabString(names):
	tokens = names.split('|')
	result = []

	for idx, token in enumerate(tokens):
		if token:
			result.append(token)
			result.append(idx)
	return result
	

def AddObjectList(prop, name):
	layout = prop.PPGLayout
	layout.AddGroup(name)
	value = prop.Parameters(name).Value
	elements = BuildListFromTabString(value)
	
	item = layout.AddEnumControl('{}List'.format(name), elements, '', constants.siControlListBox)
	item.SetAttribute(constants.siUINoLabel, True)

	item.UIItems = elements
	layout.AddRow()
	layout.AddButton('Add{}'.format(name), 'Add')
	layout.AddButton('Remove{}'.format(name), 'Remove')
	layout.EndRow()
	layout.EndGroup()


def AddObjectToList(prop, name, message):
	obj = prop.Parent3DObject
	model = obj.Model
	
	pick = Utils.PickElement(constants.siPolyMeshFilter, message)
	if pick:
		if not pick.Model.FullName == model.FullName:
			XSI.LogMessage(
				'Picked Object must be under same model as reference object',
				constants.siWarning
			)
			return
			
		if not CheckObjectAlreadyInList(prop, name, pick.Name):
			prop.Parameters(name).Value = '{}{}|'.format(prop.Parameters(name).Value, pick.Name)
	
		else:
			XSI.Logmessage('{} : {} already in list !'.format((prop.Name, pick.Name)))


def CheckObjectAlreadyInList(prop, parameter, name):
	tokens = prop.Parameters(parameter).Value.split("|")
	return name in tokens


def RemoveObjectFromList(prop, name):
	idx = prop.Parameters('{}List'.format(name)).Value
	tokens = prop.Parameters(name).Value.split('|')
	del tokens[idx]
	prop.Parameters(name).Value = '|'.join(tokens)
	prop.Parameters('{}List'.format(name)).Value = 0


def CheckParameterExist(prop, paramName, paramType, mode):
	if not prop.Parameters(paramName) and mode == 1:
		item = prop.AddParameter3(paramName, paramType)
		item.Animatable = False

	elif prop.Parameters(paramName) and not mode:
		prop.RemoveParameter(prop.Parameters(paramName))


def GetNbObjectsInList(prop, name):
	value = prop.Parameters(name).Value
	return value.count('|') + 1 if value else 0


def AddColliderObject(prop):
	num_colliders = GetNbObjectsInList(prop, 'ColliderMeshes')
	idx = str(num_colliders)
	pnt_cluster = 'CollidePointCluster{}'.format(idx)
	poly_cluster = 'ColliderPolygonCluster{}'.format(idx)
	
	CheckParameterExist(prop, pnt_cluster, constants.siString, 1)
	CheckParameterExist(prop, poly_cluster, constants.siString, 1)


def RemoveColliderObject(prop):
	num_colliders = GetNbObjectsInList(prop, 'ColliderMeshes')
	idx = prop.Parameters('ColliderMeshesList').Value+1

	if idx < num_colliders:
		for i in xrange(idx, num_colliders):
			prop.Parameters('CollidePointCluster{}'.format(i)).Value = \
				prop.Parameters('CollidePointCluster{}'.format(i+1)).Value
			prop.Parameters('ColliderPolygonCluster{}'.format(i)).Value = \
				prop.Parameters('ColliderPolygonCluster{}'.format(i+1)).Value
		
	pnt_cluster = 'CollidePointCluster{}'.format(num_colliders)
	poly_cluster = 'ColliderPolygonCluster{}'.format(num_colliders)
	
	CheckParameterExist(prop, pnt_cluster, constants.siString, 0)
	CheckParameterExist(prop, poly_cluster, constants.siString, 0)


