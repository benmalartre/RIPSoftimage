from Globals import *
import Utils2 as uti
import ICETree as tre
import collections

def IRFindClosestIDs(p,pos,IDs,range):
	delta = XSIMath.CreateVector3()
	l = 10000.0
	id = 0
	cnt = 0
	for p2 in pos:
		t = XSIMath.CreateVector3(p.X,p.Y,p.Z)
		delta.Sub(t,p2)
		length = delta.Length()
		if(length<l):
			l = length
			id = cnt
		cnt += 1
		
	return IDs[id]
	
def IRFindRedundant(a):
	return a[0]
		
def IRSort(toSort,guides):
	print "ToSort Length "+str(len(toSort))
	print "Guides Length "+str(len(guides))
	x = [None]*len(toSort)
	
	for z in range(len(toSort)):
		s = toSort[z]
		x[s[0]] = guides[z]
		print str(s[0])+":"+str(s[1])
	return x
	
def IRReorderGuideGroup(model,name):
	
	cloud = model.AddPrimitive("PointCloud","ICE_Correct")
	
	
	tree = tre.CreateICETree(cloud,"Emit",0)
	
	#emit = xsi.AddICECompoundNode("lmBuildSkeleton4", tree)
	emit = xsi.AddICECompoundNode("IRCorrectEmit", tree)
	xsi.ConnectICENodes(str(tree)+".port1", str(emit)+".Execute")
	xsi.SetValue(str(emit)+".Reference", "this_model.Muscle_Curves", "")

	# get point cloud data
	cloudgeom = cloud.ActivePrimitive.Geometry
	pAttr = cloudgeom.GetICEAttributeFromName("PointPosition")
	pos = pAttr.DataArray
	for p in pos:
		print str(p.X)+","+str(p.Y)+","+str(p.Z)
	iAttr = cloudgeom.GetICEAttributeFromName("ShowGuideID")
	IDs = iAttr.DataArray
	
	# get guide curve group
	guides = []
	
	children = model.FindChildren()
	for c in children:
		if not c.name.find("_Crv") == -1 and c.Properties("MuscleElement"):
			guides.append(c)
	grp = model.Groups(name)
	'''
	if not grp:
		grp = uti.GroupSetup(model,guides,name)
		
	if not len(guides) == grp.Members.Count:
		grp = uti.GroupSetup(model,guides,name)
	
		
	grp = model.Groups(name)
	guides = grp.Members
	'''
	xsi.ResetTransform(guides, "siCtr", "siSRT", "siXYZ")
	#xsi.FreezeModeling(guides, "", "")
		
	toSort = []
	for i in range(len(guides)):
		crv = guides[i]
		geom = crv.ActivePrimitive.Geometry
		pCount = 0
		closests = []
		for p in geom.Points:
			id = IRFindClosestIDs(p.Position,pos,IDs,0.1)
			xsi.LogMessage("Curve "+str(i)+", PNT "+str(pCount)+" ---> "+str(id))
			closests.append(id)
			pCount += 1
		
		r = IRFindRedundant(closests)
		xsi.LogMessage("Append ToSort "+crv.fullname)
		toSort.append((r,crv))
	
	sorted = IRSort(toSort,guides)
	

	if grp:
		xsi.DeleteObj(grp)
	for s in sorted:
		print s
	uti.GroupSetup(model,sorted,name)
	xsi.DeleteObj(cloud)
	
	
model = xsi.ActiveSceneRoot.FindChild("Rig")
IRReorderGuideGroup(model,"Muscle_Curves")
