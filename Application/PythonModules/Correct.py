# Cloth Correct Module
#----------------------------------
from win32com.client import constants as siConstants
from Globals import xsi
from Globals import Dispatch
import Utils as uti
import ICETree as ice


def Correct(obj, mode):
	cmdLog = xsi.Preferences.GetPreferenceValue("scripting.cmdlog")
	xsi.Preferences.SetPreferenceValue("scripting.cmdlog", False)
	cmdName = "Cloth_Corrective"+mode
	clsName = "Corrective"+mode+"Cls"

	# get datas
	if not obj or not obj.Type == "polymsh":
		xsi.LogMessage(cmdName+" ---> invalid Input", siConstants.siError)
		return
	
	current = int(uti.GetCurrentFrame())
	
	# get Cluster
	cls = uti.CreateAlwaysCompleteCluster(obj, siConstants.siVertexCluster, clsName)
	
	# freeze all weight painter op for performance reasons
	props = cls.Properties
	for p in props:
		if (mode == "Smooth" or mode == "Push") and p.Type == "wtmap":
			xsi.FreezeObj(p)
		elif mode == "Shape" and p.Type == "clskey":
			xsi.FreezeObj(p)

	# check if a weight map already exists for this frame
	exist = False
	wmap = None
	if not mode == "Shape":
		if cls.Properties("Frame"+str(current)):
			exist = True
			toolkit = Dispatch("XSI.UIToolkit")
			buttonPressed = toolkit.MsgBox("A corrective weight map already exists for this frame...",
										   siConstants.siMsgOkOnly, cmdName )

			if buttonPressed == siConstants.siMsgCancel:
				xsi.LogMessage("Store Corrective Map cancelled by You, Fuck You!!", siConstants.siInfo)
				return

		else:
			wmap = cls.AddProperty("Weight Map Property", False, "Frame"+str(current))
	
	if mode == "Push":
		CorrectPush(obj, wmap, current, exist)
		
	elif mode == "Smooth":
		CorrectSmooth(obj, wmap, current, exist)
		
	elif mode == "Shape":
		CorrectShape(obj, cls, current)
		
	xsi.Preferences.SetPreferenceValue("scripting.cmdlog", cmdLog)


def CorrectPush(obj, wmap, frame, exist):
	# check if tree exists
	prim = obj.ActivePrimitive
	tree = prim.ICETrees.Find("CorrectivePush")
	if not tree:
		tree = ice.CreateIceTree(obj, "CorrectivePush", 2)
		t = str(tree)
		
		set1 = xsi.AddICENode("SetOneDataNode", t)
		set1.Parameters("Reference").Value = "Self.PointPosition"
		ifnode = xsi.AddIceNode("IfNode", t)

		xsi.ConnectICENodes(t+".port1", str(ifnode)+".Result")
		xsi.ConnectICENodes(str(ifnode)+".IfFalse", str(set1)+".Value")
		
		get = xsi.AddICENode("GetDataNode", t)
		get.Parameters("Reference").Value = "Self.PointPosition"
		get1 = xsi.AddICENode("GetDataNode", t)
		get1.Parameters("Reference").Value = "Self.PointNormal"
		
		add = xsi.AddICENode("AddNode", t)
		xsi.ConnectICENodes(str(add)+".Value1", str(get)+".Value")
		
		add1 = xsi.AddIceNode("AddNode", t)
		mult = xsi.AddIceNode("MultiplyByScalarNode", t)
		mult1 = xsi.AddIceNode("MultiplyByScalarNode", t)
		
		xsi.ConnectICENodes(str(add)+".Value2", str(mult)+".Result")
		xsi.ConnectICENodes(str(mult)+".Value", str(get1)+".Value")
		xsi.ConnectICENodes(str(mult)+".Factor",str(mult1)+".Result")
		xsi.ConnectICENodes(str(mult1)+".Value",str(add1)+".Result")
		xsi.ConnectICENodes(str(set1)+".Source", str(add)+".Result")

		nodes = [get, get1, ifnode, add, add1, set1]
		
		compound = xsi.CreateICECompoundNode(",".join(nodes), None)
		xsi.EditICECompoundProperties(compound, "CorrectivePush", "", "", "", "", "", 1, 0, "", 4, 6732954)
		xsi.AddExposedParamToICECompoundNode(str(ifnode)+".Condition", str(compound), None, "Mute")
		xsi.AddExposedParamToICECompoundNode(str(mult1)+".Factor", str(compound), None, "Factor")

	if not exist:
		compound = tree.CompoundNodes("CorrectivePush")
		add = compound.Nodes.Filter("AddNode")(0)
		
		mult = xsi.AddIceNode("MultiplyByScalarNode", compound)
		
		addInputs = add.InputPorts
		checkPorts = -1
		p = 0
		for i in addInputs:
			if not i.IsConnected:
				checkPorts = p
				break
			p += 1
				
		if not checkPorts == -1:
			xsi.ConnectIceNodes(str(add)+".Value"+str(checkPorts+1), str(mult)+".Result")
		
		else:
			index = add.InputPorts.Count
			xsi.AddPortToICENode( str(add)+".value"+str(index), siConstants.siNodePortDataInsertionLocationAfter)
			xsi.ConnectICENodes (str(add)+".value"+str(index+1), str(mult)+".result")
		
		pushStr = "Self.cls.CorrectivePushCls."+wmap.Name+".Weights"
		getPushMap = xsi.AddIceNode("GetDataNode", str(tree))
		getPushMap.Parameters("Reference").Value = pushStr

		inputs = compound.InputPorts
		count = 0

		for i in inputs:
			if i.Name.find("Position")>-1:
				count += 1

		xsi.AddExposedParamToICECompoundNode(str(mult)+".value", str(compound), None, "Push")
		xsi.EditExposedParamInICECompoundNode(str(compound)+".Push", "Push"+str(frame), "0", "0", None, None, 0, "")
		xsi.AddExposedParamToICECompoundNode(str(mult)+".Factor", str(compound), None, "Frame"+str(frame))
		xsi.EditExposedParamInICECompoundNode(str(compound)+".Frame"+str(frame), "Frame"+str(frame), "0", "1", None, None, 0, "")
		xsi.ConnectICENodes(str(compound)+".Push"+str(frame), str(getPushMap)+".Value")
		
		xsi.InspectObj(compound, None, None, siConstants.siLock)
		xsi.SelectObj(wmap)
		xsi.PaintTool()


def CorrectSmooth(obj, wmap, frame, exist):
	# check if tree exists
	prim = obj.ActivePrimitive
	tree = prim.ICETrees.Find( "CorrectiveSmooth" )
	if not tree:

		above = prim.ICETrees.Find("CorrectivePush")
		if not above:
			above = prim.ICETrees.Find("CorrectiveShape")
		if above:
			xsi.DeactivateAbove(above, True)
			
		tree = ice.CreateIceTree(obj, "CorrectiveSmooth", 2)
		if above:
			xsi.DeactivateAbove(above, False)
		
		t = str(tree)
		set1 = xsi.AddICENode("SetOneDataNode", t)
		set1.Parameters("Reference").Value = "Self.PointPosition"
		ifnode = xsi.AddIceNode("IfNode",t)
		
		repeat = xsi.AddICENode("RepeatNode", t)
		xsi.ConnectICENodes(str(repeat)+".port", str(set1)+".value")
		repeat.InputPorts("iterations").Value = 10

		xsi.ConnectICENodes(t+".port1", str(ifnode)+".Result")
		xsi.ConnectICENodes(str(ifnode)+".IfFalse", str(set1)+".Value")
		xsi.ConnectICENodes(str(ifnode)+".IfFalse", str(repeat)+".Execute")
		
		get = xsi.AddICENode("GetDataNode", t)
		get.Parameters("Reference").Value = "Self.PointPosition"
		get1 = xsi.AddICENode("GetDataNode", t )
		get1.Parameters("Reference").Value = "Self.PointNeighbors"
		get2 = xsi.AddICENode("GetDataNode", t )
		get2.Parameters("Reference").Value = "PointPosition"
		xsi.ConnectICENodes(str(get2)+".Source", str(get1)+".Value")
		
		average = xsi.AddICENode("GetArrayAverageNode", t)
		xsi.ConnectICENodes(str(average)+".Array", str(get2)+".Value")
		
		interp = xsi.AddIceNode("LinearInterpolateNode", t)
		
		xsi.ConnectICENodes(str(interp)+".First", str(get)+".Value")
		xsi.ConnectICENodes(str(interp)+".Second", str(average)+".Result")
		xsi.ConnectIceNodes(str(set1)+".Source", str(interp)+".Result")
		
		clamp = xsi.AddICENode("ClampNode", t)
		xsi.ConnectICENodes(str(interp)+".Blend", str(clamp)+".Result")
		
		scalarZero = xsi.AddICENode("ScalarNode", t)
		scalarZero.InputPorts("Value").Value = 0
		scalarOne = xsi.AddICENode("ScalarNode", t)
		scalarOne.InputPorts("Value").Value = 1
		add = xsi.AddICENode("AddNode", t)
		
		xsi.ConnectICENodes(str(clamp)+".Limit1", str(scalarZero)+".Result")
		xsi.ConnectICENodes(str(clamp)+".Limit2", str(scalarOne)+".Result")
		xsi.ConnectICENodes(str(clamp)+".Value", str(add)+".Result")

		nodes = [get, set1, get1, get2, average, interp, repeat, ifnode, clamp, add]
		
		compound = xsi.CreateICECompoundNode(",".join(nodes), None)
		xsi.EditICECompoundProperties(compound, "CorrectiveSmooth", "", "", "", "", "", 1, 0, "", 4, 6732954)
		xsi.AddExposedParamToICECompoundNode(str(ifnode)+".Condition", str(compound), None, "Mute")
		xsi.AddExposedParamToICECompoundNode(str(repeat)+".Iterations", str(compound), None, "Repeat")
	
	if not exist:
		compound = tree.CompoundNodes("CorrectiveSmooth")
		add = compound.Nodes.Filter("AddNode")(0)
		
		mult = xsi.AddIceNode("MultiplyByScalarNode", str(compound))
		
		addInputs = add.InputPorts
		checkPorts = -1
		i = 0
		for p in addInputs:
			if not p.IsConnected:
				checkPorts = i
				break
				
		if checkPorts > -1:
			xsi.ConnectIceNodes(str(add)+".Value"+str(checkPorts+1), str(mult)+".Result")

		else:
			index = add.InputPorts.Count
			xsi.AddPortToICENode(str(add)+".Value"+str(index), siConstants.siNodePortDataInsertionLocationAfter)
			xsi.ConnectICENodes(str(add)+".Value"+str(index+1), str(mult)+".Result")
		
		t = str(tree)
		smoothStr = "Self.cls.CorrectiveSmoothCls." + wmap.Name + ".Weights"
		get = xsi.AddIceNode("GetDataNode", t)
		get.Parameters("Reference").Value = smoothStr

		inputs = compound.InputPorts
		count = 0
		
		for i in inputs:
			if not i.Name.find("Smooth") == -1:
				count += 1
		
		xsi.AddExposedParamToICECompoundNode(str(mult)+".value", str(compound), None, "Smooth")
		xsi.EditExposedParamInICECompoundNode(str(compound)+".Smooth", "Smooth"+str(frame), "0", "0", None, None, 0, "")
		xsi.AddExposedParamToICECompoundNode(str(mult)+".Factor", str(compound), None, "Factor")
		xsi.EditExposedParamInICECompoundNode(str(compound)+".Factor", "Frame"+str(frame), "0", "1", None, None, 0, "")
		
		xsi.ConnectICENodes(str(compound)+".Smooth"+str(frame), str(get)+".Value")
		
		xsi.InspectObj(compound, None, None, siConstants.siLock)
		xsi.SelectObj(wmap)
		xsi.PaintTool()


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
				siConstants.siMsgOkCancel, "Corrective Shape")

			if buttonPressed == siConstants.siMsgCancel:
				xsi.LogMessage("Store Secondary Shape cancelled by You, Fuck TYou!!", siConstants.siInfo)
				return
			
		compound = tree.CompoundNodes("CorrectiveShape")
		
	if not exist:
		compound.InputPorts("Mute").Value = True
		xsi.DeactivateAbove(tree, True)
		basePoints = prim.Geometry.Points
		basePos = basePoints.PositionArray
		xsi.DeactivateAbove(tree, False)
		secondPoints = prim.Geometry.Points
		secondPos = secondPoints.PositionArray
		if not exist:
			compound.InputPorts("Mute").Value = False
	
	else:
		op = prim.ConstructionHistory.Find("AnimationMarker")
		xsi.DeactivateAbove(op, True)
		basePoints = prim.Geometry.Points
		basePos = basePoints.PositionArray
		xsi.DeactivateAbove(op, False)
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
		tree = ice.CreateIceTree(obj, "CorrectiveShape", 2)
		t = str(tree)
		set = xsi.AddICENode("SetOneDataNode", t)
		set.Parameters("Reference").Value = "Self.PointPosition"
		ifnode = xsi.AddIceNode("IfNode", t)
		
		xsi.ConnectICENodes(t+".port1", str(ifnode)+".Result")
		xsi.ConnectICENodes(str(ifnode)+".IfFalse", str(set)+".Value")
		
		get = xsi.AddICENode("GetDataNode", t)
		get.Parameters("Reference").Value = "Self.PointPosition"

		add = xsi.AddIceNode("AddNode", t)
		
		xsi.ConnectIceNodes(str(add)+".Value1", str(get)+".Value")
		xsi.ConnectIceNodes(str(set)+".Source", str(add)+".Result")
		
		compound = xsi.CreateICECompoundNode(str(get)+","+str(set)+","+str(add)+","+str(ifnode), None)
		xsi.EditICECompoundProperties(str(compound), "CorrectiveShape", "", "", "", "", "", 1, 0, "", 4, 6732954)
		xsi.AddExposedParamToICECompoundNode(str(ifnode)+".Condition", str(compound), None, "Mute")

	if not exist:
		compound = Dispatch(tree.Nodes("CorrectiveShape"))
		add = compound.Nodes.Filter("AddNode")(0)
		
		mult = xsi.AddIceNode("MultiplyByScalarNode", str(compound))
		
		addInputs = add.InputPorts
		checkPorts = -1
		p = 0
		for i in addInputs:
			if not i.IsConnected:
				checkPorts = p
				break
			p += 1
				
		if not checkPorts == -1:
			xsi.ConnectIceNodes(str(add)+".Value"+str(checkPorts+1), str(mult)+".Result")

		else:
			index = add.InputPorts.Count
			xsi.AddPortToICENode( str(add)+".Value"+str(index), siConstants.siNodePortDataInsertionLocationAfter)
			xsi.ConnectICENodes (str(add)+".Value"+str(index+1), str(mult)+".Result")
		
		shapeStr = "Self.cls.CorrectiveShapeCls."+shapeNode.Name+".positions"
		getShape = xsi.AddIceNode("GetDataNode", str(tree))
		getShape.Parameters("Reference").Value = shapeStr
		
		inputs = compound.InputPorts
		count = 0

		for i in inputs:
			if i.Name.find("Position") > -1:
				count += 1
		
		xsi.AddExposedParamToICECompoundNode(str(mult)+".value", str(compound), None, "Position")
		xsi.EditExposedParamInICECompoundNode(str(compound)+".Position", "Position"+str(frame), "0", "0", None, None, 0, "")
		xsi.AddExposedParamToICECompoundNode(str(mult)+".Factor", str(compound), None, "Factor")
		xsi.EditExposedParamInICECompoundNode(str(compound)+".Factor", "Frame"+str(frame), "0", "1", None, None, 0, "")
		
		xsi.ConnectICENodes(str(compound)+".Position"+str(frame), str(getShape)+".value")
		
		xsi.InspectObj(compound, None, None, siConstants.siLock)
		
	# delete all nodes between secondary shape modeling and animation markers
	for h in prim.ConstructionHistory:
		if h.FullName.find("marker") == -1:
			xsi.DeleteObj(h)
		if h.Name == "Animation":
			break


def Delete(obj, mode, frame):
	cmdLog = xsi.Preferences.GetPreferenceValue("scripting.cmdlog")
	xsi.Preferences.SetPreferenceValue("scripting.cmdlog", False)
	
	cmdName = "Cloth_CorrectiveSmooth"
	clsName = "CorrectiveSmoothCls"
	if mode == "Push":
		cmdName = "Cloth_CorrectivePush"
		clsName = "CorrectivePushCls"
	elif mode == "Shape":
		cmdName = "Cloth_CorrectiveShape"
		clsName = "CorrectiveShapeCls"
	
	if not obj or not obj.Type == "polymsh":
		xsi.LogMessage(cmdName+" ---> Invalid Object", siConstants.siError)
		return
	
	prim = obj.ActivePrimitive
	
	# get Cluster
	cls = uti.CreateAlwaysCompleteCluster(obj, siConstants.siVertexCluster, clsName)

	# check if a weight map already exists for this frame
	exist = False
	if cls.Properties("Frame"+str(frame)):
		exist = True
		toolkit = Dispatch("XSI.UIToolkit")
		buttonPressed = toolkit.MsgBox("Do you really want to delete delete Frame"+str(frame),
									   siConstants.siMsgOkCancel, "Delete Corrective Push")
	
		if buttonPressed == siConstants.siMsgCancel:
			xsi.LogMessage("Delete Secondary "+mode+" cancelled by the user!!", siConstants.siInfo)
			return
	
	if not exist:
		xsi.LogMessage("Frame"+str(frame)+" doesn't exist on " + str(obj) +" ---> Delete Corrective "+mode+" aborted...")
		return
	
	else:
		xsi.DeleteObj(cls.Properties("Frame"+str(frame)))

	tree = prim.ICETrees("Corrective"+mode)
	compound = tree.Nodes("Corrective"+mode)
	correctcompound = tree.CompoundNodes("Corrective"+mode)

	# first delete corresponding GetDataNode
	gets = tree.DataProviderNodes
	for g in gets:
		if not g.Parameters("Reference").Value.find(str(frame)) == -1:
			xsi.LogMessage("Delete Get Data Node : " + g.Parameters("Reference").Value)
			xsi.DeleteObj(g)

	# delete Corresponding MultiplyByScalarNode
	multNodes = compound.Nodes.Filter("MultiplyByScalarNode")
	for m in multNodes:
		inputs = m.InputPorts
	
		if not inputs(0).IsConnected:	
			xsi.LogMessage("Delete MultiplyByScalar Node : " + str(m))
			xsi.DeleteObj(m)
	
	xsi.Preferences.SetPreferenceValue("scripting.cmdlog", cmdLog)