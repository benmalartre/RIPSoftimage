from win32com.client import constants


def GetClusterByName(obj, name):
    cls = obj.ActivePrimitive.Geometry.Clusters
    for c in cls:
        if c.Name == name:
            return c
    return None


def GetClusterPropertyByName(cls, prop_name):
    for local_prop in cls.LocalProperties:
        if local_prop.Name == prop_name:
            return local_prop
    return None


def TransferMaterial(source, target):
    src_material = source.Material
    target.SetMaterial(src_material)
    src_clusters = source.ActivePrimitive.Geometry.Clusters
    for src_cluster in src_clusters:
        if src_cluster.Type == "poly":
            if src_cluster.Material.FullName == src_material.FullName:
                continue
            tgt_cluster = GetClusterByName(target, src_cluster.Name)
            if not tgt_cluster:
                tgt_cluster = target.ActivePrimitive.Geometry.AddCluster(constants.siPolygonCluster, src_cluster.Name,
                                                                         src_cluster.Elements.Array)
            tgt_cluster.SetMaterial(src_cluster.Material)


def TransferUVs(source, target):
    src_clusters = source.ActivePrimitive.Geometry.Clusters.Filter("sample")

    for src_cluster in src_clusters:
        tgt_cluster = GetClusterByName(target, src_cluster.Name)
        if not tgt_cluster:
            tgt_cluster = target.ActivePrimitive.Geometry.AddCluster(
                constants.siSampledPointCluster, src_cluster.Name, None)

        for src_property in src_cluster.LocalProperties:
            if src_property.Type == "uvspace":
                tgt_property = GetClusterPropertyByName(tgt_cluster, src_property.Name)

                if not tgt_property:
                    tgt_property = tgt_cluster.AddProperty("Texture Projection")
                    tgt_property.Name = src_property.Name

                tgt_property.Elements.Array = src_property.Elements.Array
