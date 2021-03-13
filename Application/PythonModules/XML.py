# -------------------------------------------------------------------
# Simple XML read/write for storing compound values
# Only support for simple DataType (bool, int, float)
# -------------------------------------------------------------------
from win32com.client import constants
from Globals import XSI
from Globals import Dispatch
import xml.etree.ElementTree as xml
import os


def CheckFileExists(path):
	return os.path.exists(path)


def AddParameterToXML(root, parameter):
	param = xml.Element('parameter')
	param.attrib['datatype'] = str(parameter.ValueType)
	param.attrib['name'] = parameter.ScriptName
	param.attrib['value'] = str(parameter.Value)
	
	root.append(param)


def AddOperatorToXML(root, operator):
	op = xml.Element('operator')
	
	op.attrib['type'] = str(operator.Type)
	split = operator.FullName.split(".")
	last = split.pop()
	op.attrib['name'] = last
	
	for parameter in operator.Parameters:
		if parameter.Animatable:
			AddParameterToXML(op, parameter)
			
	root.append(op)


def AddInputPortToXML(root, port):
	data_type =port.DataType
	
	elem = xml.Element("port")
	elem.attrib["datatype"] = str(data_type)
	if data_type < 8:
		elem.attrib["name"] = port.Name
		elem.attrib["value"] = str(port.Value)
		
	root.append(elem)


def WriteSyflexXMLPreset(filename, cloth):
	XSI.LogMessage('Write Syflex XML Preset called : {}'.format(filename))
	root = xml.Element('syCloth')
	
	for item in cloth.NestedObjects:
		if item.IsClassOf(constants.siParameterID):
			if not item.ReadOnly and item.Animatable:
				AddParameterToXML(root, item)
		elif item.IsClassOf(constants.siOperatorID):
			AddOperatorToXML(root, item)

	_file = open(filename, 'w')
	xml.ElementTree(root).write(_file)
	_file.close()


def ReadSyflexXMLPreset(filename, cloth):
	if not CheckFileExists(filename):
		XSI.LogMessage("[Read Syflex XML Preset] File : " + filename + " doesn't exists...")
		return
		
	if not cloth:
		XSI.LogMessage("[Read Syflex XML Preset] Invalid Output syCloth Operator ...")
		return
		
	tree = xml.parse(filename)
	root = tree.getroot()
	
	all_operators = root.findall("operator")
	all_parameters = root.findall("parameter")
	
	for item in cloth.NestedObjects:
		if item.IsClassOf(constants.siParameterID):
			if not item.ReadOnly and item.Animatable:
				value = GetParameterValueFromList(item.Name, all_parameters)
				if value:
					item.Value = value

		elif item.IsClassOf(constants.siOperatorID):
			op = GetOperatorFromList(item, all_operators)
			if op:
				op_parameters = op.findall("parameter")
				for op_parameter in op_parameters:
					SetParameterValueFromElement(op_parameter, item)


def GetParameterValueFromList(name, parameters):
	try:
		for parameter in parameters:
			if parameter.attrib['name'] == name:
				data_type = int(parameter.attrib['datatype'])
				if data_type == constants.siBool:
					if parameter.attrib['value'] == 'False':
						return False
					else:
						return True
				elif data_type == constants.siInt2 or data_type == constants.siInt4:
					return int(parameter.attrib['value'])
				elif data_type == constants.siFloat or data_type == constants.siDouble:
					return float(parameter.attrib['value'])
				elif data_type == constants.siString:
					return parameter.attrib['value']
				else:
					return None
	except RuntimeError:
		return None
	return None


def SetParameterValueFromElement(elem, op):
	name = elem.attrib['name']
	param = op.Parameters(name)
	
	if param:
		data_type = int(elem.attrib['datatype'])
		if data_type == constants.siBool:
			if elem.attrib['value'] == 'True':
				param.Value = True
			else:
				param.Value = False
				
		elif data_type == constants.siInt2 or data_type == constants.siInt4:
			param.Value = int(elem.attrib['value'])
			
		elif data_type == constants.siFloat or data_type == constants.siDouble:
			param.Value = float(elem.attrib['value'])
		elif data_type == constants.siString:
			param.Value = elem.attrib['value']


def GetOperatorFromList(op, operators):
	split = op.FullName.split(".")
	op_name = split.pop()
	try:
		for op in operators:
			if op.attrib["name"] == op_name:
				return op
	except RuntimeError:
		return None
	return None


def WriteCompoundsXMLPreset(pth, compounds, title="Compound"):
	XSI.LogMessage("Write Compounds XML Preset called :: " + pth)
	root = xml.Element(title)
	for c in compounds:
		compound = xml.Element(c.Name)
		ports = c.InputPorts
		for p in ports:
			AddInputPortToXML(compound, p)
		root.append(compound)

	f= open(pth, 'w')
	xml.ElementTree(root).write(f)
	f.close()


def WriteCompoundXMLPreset(pth, compound):
	XSI.LogMessage("Write Compound XML Preset called :: " + pth)
	root = xml.Element(compound.Name)
	ports = compound.InputPorts
	for p in ports:
		AddInputPortToXML(root, p)
	
	f = open(pth, 'w')
	xml.ElementTree(root).write(f)
	f.close()


def ReadCompoundXMLPreset(pth, compound):
	if not CheckFileExists(pth):
		XSI.LogMessage("[Read Compound XML Preset] File : " + pth + " doesn't exists...")
		return
		
	if not compound:
		XSI.LogMessage("[Read Compound XML Preset] Invalid Output Compound ...")
		return
		
	tree = xml.parse(pth)
	root = tree.getroot()
	
	# get list of port
	portlist = root.findall("port")
	if portlist != None:
		for p in portlist:
			dt = int(p.attrib["datatype"])
			if(dt < 8):
				value = None
				portName = p.attrib["name"]
				# boolean
				if dt == 1:
					value = False
					if p.attrib["value"] == "True":
						value = True
				# long
				elif dt == 2:
					value = int(p.attrib["value"])
				# float
				elif dt == 4:
					value = float(p.attrib["value"])
					
				if compound:
					try:
						compound.InputPorts(portName).Value = value
					except:
						pass


def ReadCompoundsXMLPreset(pth, compounds):
	if not CheckFileExists(pth):
		XSI.LogMessage("[Read Compounds XML Preset] File : " + pth + " doesn't exists...")
		return
	
	tree = xml.parse(pth)
	root = tree.getroot()
		
	for c in compounds:
		if not c:
			XSI.LogMessage("[Read Compounds XML Preset] Invalid Output Compound ...")
			continue

		compound = root.find(c.Name)
		if compound:
			#get list of port
			portlist = compound.findall("port")
			if portlist != None:
				for p in portlist:
					dt = int(p.attrib["datatype"])
					if(dt<8):
						value = None
						portName = p.attrib["name"]
						#boolean
						if dt == 1:
							value = False
							if p.attrib["value"]=="True":
								value = True
						#long
						elif dt == 2:
							value = int(p.attrib["value"])
						#float
						elif dt == 4:
							value = float(p.attrib["value"])
				
						c.InputPorts(portName).Value = value


def GetCageDeformedObjectFromConnectionStack(obj):
	prim = obj.ActivePrimitive
	utils = Dispatch("XSI.Utils")
	data = utils.DataRepository
	info = data.GetConnectionStackInfo( prim )

	tree = xml.XML(info)
	connections = tree.findall("connection")

	for c in connections:
		obj = c.find("object")
		n = obj.text
		if n.find("cagedeformop") > -1:
			op = XSI.Dictionary.GetObject(n)
			target = op.Parent3DObject
			return target
	return None
