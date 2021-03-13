from win32com.client import constants
from Globals import XSI
from Globals import Dispatch
import Utils
import ICETree


def Correct(obj, mode):
	cmdLog = XSI.Preferences.GetPreferenceValue("scripting.cmdlog")
	XSI.Preferences.SetPreferenceValue("scripting.cmdlog", False)
	cmdName = "Cloth_Corrective"+mode
	clsName = "Corrective"+mode+"Cls"

	# get datas
	if not obj or not obj.Type == "polymsh":
		XSI.LogMessage(cmdName + " ---> invalid Input", constants.siError)
		return
	
	current = int(Utils.GetCurrentFrame())
	
	# get Cluster
	cls = Utils.CreateAlwaysCompleteCluster(obj, constants.siVertexCluster, clsName)
	
	# freeze all weight painter op for performance reasons
	props = cls.Properties
	for p in props:
		if (mode == "Smooth" or mode == "Push") and p.Type == "wtmap":
			XSI.FreezeObj(p)
		elif mode == "Shape" and p.Type == "clskey":
			XSI.FreezeObj(p)

	# check if a weight map already exists for this frame
	exist = False
	wmap = None
	if not mode == "Shape":
		if cls.Properties("Frame"+str(current)):
			exist = True
			toolkit = Dispatch("XSI.UIToolkit")
			buttonPressed = toolkit.MsgBox("A corrective weight map already exists for this frame...",
										   constants.siMsgOkOnly, cmdName )

			if buttonPressed == constants.siMsgCancel:
				XSI.LogMessage("Store Corrective Map cancelled by you !", constants.siInfo)
				return

		else:
			wmap = cls.AddProperty("Weight Map Property", False, "Frame"+str(current))
	
	if mode == "Push":
		CorrectPush(obj, wmap, current, exist)
		
	elif mode == "Smooth":
		CorrectSmooth(obj, wmap, current, exist)
		
	elif mode == "Shape":
		CorrectShape(obj, cls, current)
		
	XSI.Preferences.SetPreferenceValue("scripting.cmdlog", cmdLog)


def CorrectPush(obj, wmap, frame, exist):
	# check if tree exists
	prim = obj.ActivePrimitive
	tree = prim.ICETrees.Find("CorrectivePush")
	if not tree:
		tree = ICETree.CreateIceTree(obj, "CorrectivePush", 2)
		t = str(tree)
		
		set1 = XSI.AddICENode("SetOneDataNode", t)
		set1.Parameters("Reference").Value = "Self.PointPosition"
		ifnode = XSI.AddIceNode("IfNode", t)

		XSI.ConnectICENodes(t + ".port1", str(ifnode) + ".Result")
		XSI.ConnectICENodes(str(ifnode) + ".IfFalse", str(set1) + ".Value")
		
		get = XSI.AddICENode("GetDataNode", t)
		get.Parameters("Reference").Value = "Self.PointPosition"
		get1 = XSI.AddICENode("GetDataNode", t)
		get1.Parameters("Reference").Value = "Self.PointNormal"
		
		add = XSI.AddICENode("AddNode", t)
		XSI.ConnectICENodes(str(add) + ".Value1", str(get) + ".Value")
		
		add1 = XSI.AddIceNode("AddNode", t)
		mult = XSI.AddIceNode("MultiplyByScalarNode", t)
		mult1 = XSI.AddIceNode("MultiplyByScalarNode", t)
		
		XSI.ConnectICENodes(str(add) + ".Value2", str(mult) + ".Result")
		XSI.ConnectICENodes(str(mult) + ".Value", str(get1) + ".Value")
		XSI.ConnectICENodes(str(mult) + ".Factor", str(mult1) + ".Result")
		XSI.ConnectICENodes(str(mult1) + ".Value", str(add1) + ".Result")
		XSI.ConnectICENodes(str(set1) + ".Source", str(add) + ".Result")

		nodes = [get, get1, ifnode, add, add1, set1]
		
		compound = XSI.CreateICECompoundNode(",".join(nodes), None)
		XSI.EditICECompoundProperties(compound, "CorrectivePush", "", "", "", "", "", 1, 0, "", 4, 6732954)
		XSI.AddExposedParamToICECompoundNode(str(ifnode) + ".Condition", str(compound), None, "Mute")
		XSI.AddExposedParamToICECompoundNode(str(mult1) + ".Factor", str(compound), None, "Factor")

	if not exist:
		compound = tree.CompoundNodes("CorrectivePush")
		add = compound.Nodes.Filter("AddNode")(0)
		
		mult = XSI.AddIceNode("MultiplyByScalarNode", compound)
		
		addInputs = add.InputPorts
		checkPorts = -1
		p = 0
		for i in addInputs:
			if not i.IsConnected:
				checkPorts = p
				break
			p += 1
				
		if not checkPorts == -1:
			XSI.ConnectIceNodes(str(add) + ".Value" + str(checkPorts + 1), str(mult) + ".Result")
		
		else:
			index = add.InputPorts.Count
			XSI.AddPortToICENode(str(add) + ".value" + str(index), constants.siNodePortDataInsertionLocationAfter)
			XSI.ConnectICENodes (str(add) + ".value" + str(index + 1), str(mult) + ".result")
		
		pushStr = "Self.cls.CorrectivePushCls."+wmap.Name+".Weights"
		getPushMap = XSI.AddIceNode("GetDataNode", str(tree))
		getPushMap.Parameters("Reference").Value = pushStr

		inputs = compound.InputPorts
		count = 0

		for i in inputs:
			if i.Name.find("Position")>-1:
				count += 1

		XSI.AddExposedParamToICECompoundNode(str(mult) + ".value", str(compound), None, "Push")
		XSI.EditExposedParamInICECompoundNode(str(compound) + ".Push", "Push" + str(frame), "0", "0", None, None, 0, "")
		XSI.AddExposedParamToICECompoundNode(str(mult) + ".Factor", str(compound), None, "Frame" + str(frame))
		XSI.EditExposedParamInICECompoundNode(str(compound) + ".Frame" + str(frame), "Frame" + str(frame), "0", "1", None, None, 0, "")
		XSI.ConnectICENodes(str(compound) + ".Push" + str(frame), str(getPushMap) + ".Value")
		
		XSI.InspectObj(compound, None, None, constants.siLock)
		XSI.SelectObj(wmap)
		XSI.PaintTool()


def CorrectSmooth(obj, wmap, frame, exist):
	# check if tree exists
	prim = obj.ActivePrimitive
	tree = prim.ICETrees.Find( "CorrectiveSmooth" )
	if not tree:

		above = prim.ICETrees.Find("CorrectivePush")
		if not above:
			above = prim.ICETrees.Find("CorrectiveShape")
		if above:
			XSI.DeactivateAbove(above, True)
			
		tree = ICETree.CreateIceTree(obj, "CorrectiveSmooth", 2)
		if above:
			XSI.DeactivateAbove(above, False)
		
		t = str(tree)
		set1 = XSI.AddICENode("SetOneDataNode", t)
		set1.Parameters("Reference").Value = "Self.PointPosition"
		ifnode = XSI.AddIceNode("IfNode", t)
		
		repeat = XSI.AddICENode("RepeatNode", t)
		XSI.ConnectICENodes(str(repeat) + ".port", str(set1) + ".value")
		repeat.InputPorts("iterations").Value = 10

		XSI.ConnectICENodes(t + ".port1", str(ifnode) + ".Result")
		XSI.ConnectICENodes(str(ifnode) + ".IfFalse", str(set1) + ".Value")
		XSI.ConnectICENodes(str(ifnode) + ".IfFalse", str(repeat) + ".Execute")
		
		get = XSI.AddICENode("GetDataNode", t)
		get.Parameters("Reference").Value = "Self.PointPosition"
		get1 = XSI.AddICENode("GetDataNode", t)
		get1.Parameters("Reference").Value = "Self.PointNeighbors"
		get2 = XSI.AddICENode("GetDataNode", t)
		get2.Parameters("Reference").Value = "PointPosition"
		XSI.ConnectICENodes(str(get2) + ".Source", str(get1) + ".Value")
		
		average = XSI.AddICENode("GetArrayAverageNode", t)
		XSI.ConnectICENodes(str(average) + ".Array", str(get2) + ".Value")
		
		interp = XSI.AddIceNode("LinearInterpolateNode", t)
		
		XSI.ConnectICENodes(str(interp) + ".First", str(get) + ".Value")
		XSI.ConnectICENodes(str(interp) + ".Second", str(average) + ".Result")
		XSI.ConnectIceNodes(str(set1) + ".Source", str(interp) + ".Result")
		
		clamp = XSI.AddICENode("ClampNode", t)
		XSI.ConnectICENodes(str(interp) + ".Blend", str(clamp) + ".Result")
		
		scalarZero = XSI.AddICENode("ScalarNode", t)
		scalarZero.InputPorts("Value").Value = 0
		scalarOne = XSI.AddICENode("ScalarNode", t)
		scalarOne.InputPorts("Value").Value = 1
		add = XSI.AddICENode("AddNode", t)
		
		XSI.ConnectICENodes(str(clamp) + ".Limit1", str(scalarZero) + ".Result")
		XSI.ConnectICENodes(str(clamp) + ".Limit2", str(scalarOne) + ".Result")
		XSI.ConnectICENodes(str(clamp) + ".Value", str(add) + ".Result")

		nodes = [get, set1, get1, get2, average, interp, repeat, ifnode, clamp, add]
		
		compound = XSI.CreateICECompoundNode(",".join(nodes), None)
		XSI.EditICECompoundProperties(compound, "CorrectiveSmooth", "", "", "", "", "", 1, 0, "", 4, 6732954)
		XSI.AddExposedParamToICECompoundNode(str(ifnode) + ".Condition", str(compound), None, "Mute")
		XSI.AddExposedParamToICECompoundNode(str(repeat) + ".Iterations", str(compound), None, "Repeat")
	
	if not exist:
		compound = tree.CompoundNodes("CorrectiveSmooth")
		add = compound.Nodes.Filter("AddNode")(0)
		
		mult = XSI.AddIceNode("MultiplyByScalarNode", str(compound))
		
		addInputs = add.InputPorts
		checkPorts = -1
		i = 0
		for p in addInputs:
			if not p.IsConnected:
				checkPorts = i
				break
				
		if checkPorts > -1:
			XSI.ConnectIceNodes(str(add) + ".Value" + str(checkPorts + 1), str(mult) + ".Result")

		else:
			index = add.InputPorts.Count
			XSI.AddPortToICENode(str(add) + ".Value" + str(index), constants.siNodePortDataInsertionLocationAfter)
			XSI.ConnectICENodes(str(add) + ".Value" + str(index + 1), str(mult) + ".Result")
		
		t = str(tree)
		smoothStr = "Self.cls.CorrectiveSmoothCls." + wmap.Name + ".Weights"
		get = XSI.AddIceNode("GetDataNode", t)
		get.Parameters("Reference").Value = smoothStr

		inputs = compound.InputPorts
		count = 0
		
		for i in inputs:
			if not i.Name.find("Smooth") == -1:
				count += 1
		
		XSI.AddExposedParamToICECompoundNode(str(mult) + ".value", str(compound), None, "Smooth")
		XSI.EditExposedParamInICECompoundNode(str(compound) + ".Smooth", "Smooth" + str(frame), "0", "0", None, None, 0, "")
		XSI.AddExposedParamToICECompoundNode(str(mult) + ".Factor", str(compound), None, "Factor")
		XSI.EditExposedParamInICECompoundNode(str(compound) + ".Factor", "Frame" + str(frame), "0", "1", None, None, 0, "")
		
		XSI.ConnectICENodes(str(compound) + ".Smooth" + str(frame), str(get) + ".Value")
		
		XSI.InspectObj(compound, None, None, constants.siLock)
		XSI.SelectObj(wmap)
		XSI.PaintTool()


def CorrectShape(obj, cls, frame):
	prim = obj.ActivePrimitive
	tree = prim.ICETrees.Find("CorrectiveShape")
	exist = False
	basePos = None
	secondPos = None
	
	if tree:
		if cls.Properties("Frame"+str(frame)):
			exist = True
			toolkit = Dispatch("XSI.UIToolkit")
			buttonPressed = toolkit.MsgBox( "A corrective shape already exists for this frame\n Would you replace it??",
				constants.siMsgOkCancel, "Corrective Shape")

			if buttonPressed == constants.siMsgCancel:
				XSI.LogMessage("Store Secondary Shape cancelled by You, Fuck TYou!!", constants.siInfo)
				return
			
		compound = tree.CompoundNodes("CorrectiveShape")
		
	if not exist:
		compound.InputPorts("Mute").Value = True
		XSI.DeactivateAbove(tree, True)
		basePoints = prim.Geometry.Points
		basePos = basePoints.PositionArray
		XSI.DeactivateAbove(tree, False)
		secondPoints = prim.Geometry.Points
		secondPos = secondPoints.PositionArray
		if not exist:
			compound.InputPorts("Mute").Value = False
	
	else:
		op = prim.ConstructionHistory.Find("AnimationMarker")
		XSI.DeactivateAbove(op, True)
		basePoints = prim.Geometry.Points
		basePos = basePoints.PositionArray
		XSI.DeactivateAbove(op, False)
		secondPoints = prim.Geometry.Points
		secondPos = secondPoints.PositionArray
	
	l = len(basePos[0])
	delta = [0] * 3 * l
			
	for a in range(l):
		delta[a*3] = secondPos[0][a]-basePos[0][a]
		delta[a*3+1] = secondPos[1][a]-basePos[1][a]
		delta[a*3+2] = secondPos[2][a]-basePos[2][a]

	if not exist:
		shapeNode = Dispatch(cls.AddProperty("SimpleDeformShape", 0, "Frame"+str(frame)))
		shapeNode.Elements.Array = delta
		
	else:
		shapeNode = Dispatch(cls.Properties("Frame"+str(frame)))
		shapeNode.Elements.Array = delta

	if not tree:
		tree = ICETree.CreateIceTree(obj, "CorrectiveShape", 2)
		t = str(tree)
		set = XSI.AddICENode("SetOneDataNode", t)
		set.Parameters("Reference").Value = "Self.PointPosition"
		ifnode = XSI.AddIceNode("IfNode", t)
		
		XSI.ConnectICENodes(t + ".port1", str(ifnode) + ".Result")
		XSI.ConnectICENodes(str(ifnode) + ".IfFalse", str(set) + ".Value")
		
		get = XSI.AddICENode("GetDataNode", t)
		get.Parameters("Reference").Value = "Self.PointPosition"

		add = XSI.AddIceNode("AddNode", t)
		
		XSI.ConnectIceNodes(str(add) + ".Value1", str(get) + ".Value")
		XSI.ConnectIceNodes(str(set) + ".Source", str(add) + ".Result")
		
		compound = XSI.CreateICECompoundNode(str(get) + "," + str(set) + "," + str(add) + "," + str(ifnode), None)
		XSI.EditICECompoundProperties(str(compound), "CorrectiveShape", "", "", "", "", "", 1, 0, "", 4, 6732954)
		XSI.AddExposedParamToICECompoundNode(str(ifnode) + ".Condition", str(compound), None, "Mute")

	if not exist:
		compound = Dispatch(tree.Nodes("CorrectiveShape"))
		add = compound.Nodes.Filter("AddNode")(0)
		
		mult = XSI.AddIceNode("MultiplyByScalarNode", str(compound))
		
		addInputs = add.InputPorts
		checkPorts = -1
		p = 0
		for i in addInputs:
			if not i.IsConnected:
				checkPorts = p
				break
			p += 1
				
		if not checkPorts == -1:
			XSI.ConnectIceNodes(str(add) + ".Value" + str(checkPorts + 1), str(mult) + ".Result")

		else:
			index = add.InputPorts.Count
			XSI.AddPortToICENode(str(add) + ".Value" + str(index), constants.siNodePortDataInsertionLocationAfter)
			XSI.ConnectICENodes (str(add) + ".Value" + str(index + 1), str(mult) + ".Result")
		
		shapeStr = "Self.cls.CorrectiveShapeCls."+shapeNode.Name+".positions"
		getShape = XSI.AddIceNode("GetDataNode", str(tree))
		getShape.Parameters("Reference").Value = shapeStr
		
		inputs = compound.InputPorts
		count = 0

		for i in inputs:
			if i.Name.find("Position") > -1:
				count += 1
		
		XSI.AddExposedParamToICECompoundNode(str(mult) + ".value", str(compound), None, "Position")
		XSI.EditExposedParamInICECompoundNode(str(compound) + ".Position", "Position" + str(frame), "0", "0", None, None, 0, "")
		XSI.AddExposedParamToICECompoundNode(str(mult) + ".Factor", str(compound), None, "Factor")
		XSI.EditExposedParamInICECompoundNode(str(compound) + ".Factor", "Frame" + str(frame), "0", "1", None, None, 0, "")
		
		XSI.ConnectICENodes(str(compound) + ".Position" + str(frame), str(getShape) + ".value")
		
		XSI.InspectObj(compound, None, None, constants.siLock)
		
	# delete all nodes between secondary shape modeling and animation markers
	for h in prim.ConstructionHistory:
		if h.FullName.find('marker') == -1:
			XSI.DeleteObj(h)
		if h.Name == 'Animation':
			break


def Delete(obj, mode, frame):
	log_state = Utils.SetScriptingLogState(False)
	
	command_name = 'Cloth_CorrectiveSmooth'
	cluster_name = 'CorrectiveSmoothCls'
	if mode == 'Push':
		command_name = 'Cloth_CorrectivePush'
		cluster_name = 'CorrectivePushCls'
	elif mode == 'Shape':
		command_name = 'Cloth_CorrectiveShape'
		cluster_name = 'CorrectiveShapeCls'
	
	if not obj or not obj.Type == 'polymsh':
		XSI.LogMessage(command_name + " ---> Invalid Object", constants.siError)
		return
	
	prim = obj.ActivePrimitive
	
	# get Cluster
	cluster = Utils.CreateAlwaysCompleteCluster(obj, constants.siVertexCluster, cluster_name)

	# check if a weight map already exists for this frame
	exist = False
	if cluster.Properties('Frame{}'.format(frame)):
		exist = True
		toolkit = Dispatch('XSI.UIToolkit')
		button_pressed = toolkit.MsgBox(
			'Do you really want to delete delete Frame{}'.format(frame),
			constants.siMsgOkCancel, 'Delete Corrective Push'
		)
	
		if button_pressed == constants.siMsgCancel:
			XSI.LogMessage("Delete Secondary " + mode + " cancelled by the user!!", constants.siInfo)
			return
	
	if not exist:
		XSI.LogMessage("Frame" + str(frame) + " doesn't exist on " + str(obj) + " ---> Delete Corrective " + mode + " aborted...")
		return
	
	else:
		XSI.DeleteObj(cluster.Properties('Frame{}'.format(frame)))

	tree = prim.ICETrees('Corrective{}'.format(mode))
	compound = tree.CompoundNodes('Corrective{}'.format(mode))

	# first delete corresponding GetDataNode
	get_nodes = tree.DataProviderNodes
	for get_node in get_nodes:
		if not get_node.Parameters('Reference').Value.find(str(frame)) == -1:
			XSI.LogMessage('Delete Get Data Node "{}"'.format(get_node.Parameters('Reference').Value))
			XSI.DeleteObj(get_node)

	# delete Corresponding MultiplyByScalarNode
	multiply_nodes = compound.Nodes.Filter('MultiplyByScalarNode')
	for multiply_node in multiply_nodes:
		inputs = multiply_node.InputPorts
	
		if not inputs(0).IsConnected:	
			XSI.LogMessage('Delete MultiplyByScalar Node : {}'.format(multiply_node))
			XSI.DeleteObj(multiply_node)

	Utils.SetScriptingLogState(log_state)
