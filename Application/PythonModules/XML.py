# -------------------------------------------------------------------
# Simple XML read/write for storing compound values
# Only support for simple DataType (bool, int, float)
# -------------------------------------------------------------------
from win32com.client import constants as siConstants
from Globals import xsi
from Globals import Dispatch
import xml.etree.ElementTree as xml
import os
from os import path

def CheckFileExists(path):
	return os.path.exists(path)


def AddParameterToXML(root, parameter):
	param = xml.Element("parameter")
	param.attrib["datatype"] = str(parameter.ValueType)
	param.attrib["name"] = parameter.ScriptName
	param.attrib["value"] = str(parameter.Value)
	
	root.append(param)


def AddOperatorToXML(root, operator):
	op = xml.Element("operator")
	
	op.attrib["type"] = str(operator.Type)
	split = operator.FullName.split(".")
	s = split.pop()
	op.attrib["name"] = s
	
	for p in operator.Parameters:
		if p.Animatable:
			AddParameterToXML(op, p)
			
	root.append(op)


def AddInputPortToXML(root, port):
	dt =port.DataType
	
	elem = xml.Element("port")
	elem.attrib["datatype"] = str(dt)
	if dt<8:
		elem.attrib["name"] = port.Name
		elem.attrib["value"] = str(port.Value)
		
	root.append(elem)


def WriteSyflexXMLPreset(pth, clothop):
	xsi.LogMessage("Write Syflex XML Preset called :: "+pth)
	root = xml.Element("syCloth")
	
	for n in clothop.NestedObjects:
		if n.IsClassOf(siConstants.siParameterID):
			if not n.ReadOnly and n.Animatable:
				AddParameterToXML(root, n)
		elif n.IsClassOf(siConstants.siOperatorID):
			AddOperatorToXML(root, n)

	f= open(pth, 'w')
	xml.ElementTree(root).write(f)
	f.close()


def ReadSyflexXMLPreset(pth, clothop):
	if not CheckFileExists(pth) :
		xsi.LogMessage("[Read Syflex XML Preset] File : "+pth+" doesn't exists...")
		return
		
	if not clothop:
		xsi.LogMessage("[Read Syflex XML Preset] Invalid Output syCloth Operator ...")
		return
		
	tree = xml.parse(pth)
	root = tree.getroot()
	
	ops = root.findall("operator")
	params = root.findall("parameter")
	
	for n in clothop.NestedObjects:
		if n.IsClassOf(siConstants.siParameterID):
			if not n.ReadOnly and n.Animatable:
				value = GetParameterValueFromList(n.Name, params)
				if value:
					n.Value = value

		elif n.IsClassOf(siConstants.siOperatorID):
			op = GetOperatorFromList(n, ops)
			if op:
				params = op.findall("parameter")
				for p in params:
					SetParameterValueFromElement(p, n)


def GetParameterValueFromList(pName,pList):
	try:
		for p in pList:
			if p.attrib["name"] == pName:
				dt = int(p.attrib["datatype"])
				if dt == siConstants.siBool:
					if p.attrib["value"] == "False":
						return False
					else:
						return True
				elif dt == siConstants.siInt2 or dt == siConstants.siInt4:
					return int(p.attrib["value"])
				elif dt == siConstants.siFloat or dt == siConstants.siDouble:
					return float(p.attrib["value"])
				elif dt == siConstants.siString:
					return p.attrib["value"]
				else:
					return None
	except:
		return None
	return None


def SetParameterValueFromElement(elem, op):
	pName = elem.attrib["name"]
	param = op.Parameters(pName)
	
	if param:
		dt = int(elem.attrib["datatype"])
		if dt == siConstants.siBool:
			if elem.attrib["value"] == "True":
				param.Value = True
			else:
				param.Value = False
				
		elif dt == siConstants.siInt2 or dt == siConstants.siInt4:
			param.Value = int(elem.attrib["value"])
			
		elif dt == siConstants.siFloat or dt == siConstants.siDouble:
			param.Value = float(elem.attrib["value"])
		elif dt == siConstants.siString:
			param.Value = elem.attrib["value"]


def GetOperatorFromList(op, opList):
	split = op.FullName.split(".")
	opName = split.pop()
	try:
		for o in opList:
			if o.attrib["name"] == opName:
				return o
	except:
		return None
	return None


def WriteCompoundsXMLPreset(pth, compounds, title="Compound"):
	xsi.LogMessage("Write Compounds XML Preset called :: "+pth)
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
	xsi.LogMessage("Write Compound XML Preset called :: "+pth)
	root = xml.Element(compound.Name)
	ports = compound.InputPorts
	for p in ports:
		AddInputPortToXML(root, p)
	
	f = open(pth, 'w')
	xml.ElementTree(root).write(f)
	f.close()


def ReadCompoundXMLPreset(pth, compound):
	if not CheckFileExists(pth):
		xsi.LogMessage("[Read Compound XML Preset] File : "+pth+" doesn't exists...")
		return
		
	if not compound:
		xsi.LogMessage("[Read Compound XML Preset] Invalid Output Compound ...")
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
		xsi.LogMessage("[Read Compounds XML Preset] File : "+pth+" doesn't exists...")
		return
	
	tree = xml.parse(pth)
	root = tree.getroot()
		
	for c in compounds:
		if not c:
			xsi.LogMessage("[Read Compounds XML Preset] Invalid Output Compound ...")
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
			op = xsi.Dictionary.GetObject(n)
			target = op.Parent3DObject
			return target
	return None
