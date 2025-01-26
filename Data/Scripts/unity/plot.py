from win32com.client import constants
from win32com.client.dynamic import Dispatch


xsi = Dispatch('XSI.Application').Application


def GetStartFrame():
	control = xsi.Dictionary.GetObject("PlayControl")
	return control.In.Value


def GetEndFrame():
	control = xsi.Dictionary.GetObject("PlayControl")
	return control.Out.Value


def GetCurrentFrame():
	control = xsi.Dictionary.GetObject("PlayControl")
	return control.Current.Value


def GetRotParameters(objects):
	params = ""
	for o in objects:
		params = params + o.FullName+".kine.local.rotx,"
		params = params + o.FullName+".kine.local.roty,"
		params = params + o.FullName+".kine.local.rotz,"
	
	params = params[:-1]
	return params


def GetPosParameters(objects):
	params = ""
	for o in objects:
		params = params + o.FullName+".kine.local.posx,"
		params = params + o.FullName+".kine.local.posy,"
		params = params + o.FullName+".kine.local.posz,"
	
	params = params[:-1]
	return params


sel = xsi.Selection(0)
if sel:
	if sel.Type == "#model":
		model = sel
	else:
		model = sel.Model
	
	if model.Groups("Plot_Orientation") and model.Groups("Plot_Position"):
	
		prop = xsi.ActiveSceneRoot.AddProperty("CustomProperty", 0, "PlotCharacter")
		prop.AddParameter3("StartFrame", constants.siInt4, GetStartFrame(), -10000, 10000, 0, 0)
		prop.AddParameter3("EndFrame", constants.siInt4, GetEndFrame(), -10000, 10000, 0, 0)

		xsi.InspectObj(prop, "", "Plot Character", constants.siModal)

		start_frame = prop.Parameters("StartFrame").Value
		end_frame = prop.Parameters("EndFrame").Value
		xsi.DeleteObj(prop)

		ori = model.Groups("Plot_Orientation")
		pos = model.Groups("Plot_Position")
		xsi.RemoveAllAnimation(ori.members, 3, "siUnspecified", "siAnySource", "siTransformParam", "", "", "")
		xsi.PlotAndApplyActions(
			GetRotParameters(ori.Members), "plot", start_frame, end_frame, "", 20, 3, "", "", "", "", True, True)
		xsi.PlotAndApplyActions(
			GetPosParameters(pos.Members), "plot", start_frame, end_frame, "", 20, 3, "", "", "", "", True, True)

		# Get all animated parameters under this model
		marked = []
		for m in ori.Members:
			marked.append(m.Kinematics.Local.RotX)
			marked.append(m.Kinematics.Local.RotY)
			marked.append(m.Kinematics.Local.RotZ)
			
		for m in pos.Members:
			marked.append(m.Kinematics.Local.PosX)
			marked.append(m.Kinematics.Local.PosY)
			marked.append(m.Kinematics.Local.PosZ)

		# first we clean the FCurves(removing all unnecessary keys)
		for m in marked:
			source = m.Source
			if source.IsClassOf(constants.siFCurveID) :
				# we don't fit the fcurve if it's belong to the mixer
				if source.Parent.FullName.find("Mixer") > -1:
					continue
					
				if source.GetNumKeys()>2:
					source.Fit( source.GetMinKeyFrame(), source.GetMaxKeyFrame(), 0.1)

		# store and export action
		# action = xsi.StoreAction(model,marked.GetAsText(),2,name,False)
	else:
		xsi.LogMessage("Selected model doesn't have Plot Groups!! Aborted!!!", constants.siError)
		
else:
	xsi.LogMessage("Nothing Selected!!", constants.siError)
