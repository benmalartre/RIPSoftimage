from Utils2 import*

def CreateChainedCnsEnv(parent=None):
	sel = xsi.Selection(0)
	sub = sel.SubComponent
	mesh = sub.Parent3DObject
	geom = mesh.ActivePrimitive.Geometry
	pnts = geom.Points
	if parent == None:
		parent = PickElement(constants.siGenericObjectFilter,"Pick Parent")
	for elem in sub.ElementArray:
		cls = xsi.CreateCluster(pnts[elem])
		rivet = AddNull(parent,2,0.1)
		cns = rivet.Kinematics.AddConstraint("ObjectToCluster",cls)
		cns.Parameters("tangent").Value = 1
		cns.Parameters("upvct_active").Value = 1
		parent = rivet

def EffToLastBone(eff):
	root = eff.Root
	return root.Bones[root.Bones.Count-1]
	
def BoneInCollection(bone,collection):
	for i in collection:
		if i.FullName == bone.FullName:
			return 1
	return 0
	
def FindEnvParent(bone,coll):
	parent = None
	check = bone.Parent3DObject
	while parent == None:
		if check.Type == "eff":
			parent = EffToLastBone(check)

		elif BoneInCollection(check,coll):
			parent = check
		else:
			check = check.Parent3DObject
			if check.Type == "#model":
				xsi.LogMessage("You reached Root Model...")
				parent = check
	xsi.LogMessage("Parent for "+bone.Name +" : "+parent.Name)
	return parent
		
def AddNullToBone(bone,parent,grp):
	
	if not bone.Type == "eff":
		check = parent.Model.FindChild(bone.Name)
		if check:
			return check
		
		null = AddNull(parent,2,0.2,bone.Name)
		MatchTransform(null,bone)
		null.Kinematics.AddConstraint("Pose",bone)
		grp.AddMember(null)
		return null

def BuildNullRig(model,srt,meshes):
	for mesh in meshes:
		xsi.ResetActor(mesh)
		dup = xsi.Duplicate(mesh,1,0,0,0,2)(0)
		
		model.AddChild(dup)
		dup.Name = mesh.Name

		grp = GroupSetup(model,None,"Envelope")
		stack = mesh.ActivePrimitive.ConstructionHistory
		envelope = mesh.Envelopes(0)
		if not envelope:
			xsi.LogMessage("Selected Mesh doesn 't have an envelope, aborted!!")
			continue
	
		deformers = envelope.Deformers

		nulls = XSIFactory.CreateActiveXObject("XSI.Collection")
		parentNames = []
		for d in deformers:
			n = AddNullToBone(d,srt,grp)
			if n:
				nulls.Add(n)
			parent = FindEnvParent(d,deformers)
			parentNames.append(parent.Name)
			
		i = 0
		for n in nulls:
			parent = model.FindChild(parentNames[i])
			i = i+1
			if parent:
				parent.AddChild(n)
				
				
		#Add Envelope
		xsi.ApplyFlexEnv(str(dup)+";"+nulls.GetAsText(),0,constants.siConstructionModeAnimation)
		envelope2 = dup.Envelopes(0)
		
		#Transfer Weights
		weights = envelope.GetWeights2()
		weights2 = envelope2.GetWeights2()
		
		weights2.Array = weights.Array

	
model = xsi.ActiveSceneRoot.AddModel(None,"NullRig")
srt = AddNull(model,2,0.5,"GlobalSRT")
meshes = []
for s in xsi.Selection:
	meshes.append(s)
BuildNullRig(model,srt,meshes)
	
#CreateChainedCnsEnv(parent=None)