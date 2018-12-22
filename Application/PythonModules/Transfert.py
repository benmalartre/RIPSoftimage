# -------------------------------------------------------------------
# Transfer
# -------------------------------------------------------------------
from Globals import siConstants
import Utils as uti
reload(uti)


def GetClusterByName(obj, clsname):
	cls = obj.ActivePrimitive.Geometry.Clusters
	for c in cls:
		if c.Name == clsname:
			return c
	return None


def GetClusterPropertyByName(cls, clspropname):
	for p in cls.LocalProperties:
		if p.Name == clspropname:
			return p
	return None


def TransfertMaterial(source, target):
	sm = source.Material
	target.SetMaterial(sm)
	scls = source.ActivePrimitive.Geometry.Clusters
	tcls = target.ActivePrimitive.Geometry.Clusters
	for sc in scls:
		if sc.Type == "poly":
			if not sc.Material.FullName == sm.FullName:
				tc = GetClusterByName(target, sc.Name)
				if not tc:
					tc = target.ActivePrimitive.Geometry.AddCluster(siConstants.siPolygonCluster, sc.Name, sc.Elements.Array)
				tc.SetMaterial(sc.Material)


def TransfertUVs(source, target):
	scls = source.ActivePrimitive.Geometry.Clusters.Filter("sample")
	
	for sc in scls:
		tc = GetClusterByName(target, sc.Name)
		if not tc:
			tc = target.ActivePrimitive.Geometry.AddCluster(siConstants.siSampledPointCluster, sc.Name, None)
	
		for sp in sc.LocalProperties:
			if sp.Type =="uvspace":
				tp = GetClusterPropertyByName(tc,sp.Name)
		
				if not tp:
					tp = tc.AddProperty("Texture Projection")
					tp.Name = sp.Name
				
				tp.Elements.Array = sp.Elements.Array