import Utils
reload(Utils)
from Globals import *
from Utils import *

xsi = XSI

OFFSET_MATRIX = [
    0, 0, -1, 0,  
    0, 1, 0, 0,  
    1, 0, 0, 0,  
    0, 0, 0, 1,  
]


def CreateChainedCnsEnv(parent=None):
    selection = xsi.Selection(0)
    sub = selection.SubComponent
    mesh = sub.Parent3DObject
    geom = mesh.ActivePrimitive.Geometry
    points = geom.Points
    if not parent:
        parent = PickElement(constants.siGenericObjectFilter, "Pick Parent")
    for elem in sub.ElementArray:
        cluster = xsi.CreateCluster(points[elem])
        rivet = AddNull(parent, 2, 0.1)
        cns = rivet.Kinematics.AddConstraint("ObjectToCluster", cluster)
        cns.Parameters("tangent").Value = 1
        cns.Parameters("upvct_active").Value = 1
        parent = rivet


def EffToLastBone(eff):
    root = eff.Root
    return root.Bones[root.Bones.Count - 1]


def BoneInCollection(bone, collection):
    for item in collection:
        if item.FullName == bone.FullName:
            return 1
    return 0


def FindEnvParent(bone, collection):
    parent = None
    check = bone.Parent3DObject
    while not parent:
        if check.Type == "eff":
            parent = EffToLastBone(check)

        elif BoneInCollection(check, collection):
            parent = check
        else:
            check = check.Parent3DObject
            if check.Type == "#model":
                xsi.LogMessage("You reached Root Model...")
                parent = check
    xsi.LogMessage("Parent for " + bone.Name + " : " + parent.Name)
    return parent


def AddNullToBone(bone, parent, grp):
    xsi.LogMessage("Add null to bone : " + str(bone) + "," + str(parent) + "," + str(grp))

    if not bone.Type == "eff":
        check = parent.Model.FindChild(bone.Name)
        if check:
            return check

        null = AddNull(parent, 2, 0.2, bone.Name)
        MatchTransformWithOffset(null, bone, OFFSET_MATRIX)
        null.Kinematics.AddConstraint("Pose", bone, True)
        grp.AddMember(null)
        return null


def BuildNullRig(model, srt, meshes):
    for mesh in meshes:
        xsi.ResetActor(mesh)
        dup = xsi.Duplicate(mesh, 1, 0, 0, 0, 2)(0)

        model.AddChild(dup)
        dup.Name = mesh.Name

        grp = GroupSetup(model, None, "Envelope")
        envelope = mesh.Envelopes(0)
        if not envelope:
            xsi.LogMessage("Selected Mesh doesn 't have an envelope, aborted!!")
            continue

        deformers = envelope.Deformers

        nulls = XSIFactory.CreateActiveXObject("XSI.Collection")
        parentNames = {}
        for deformer in deformers:
            if deformer.Type == "eff":
                continue
            null = AddNullToBone(deformer, srt, grp)
            if null:
                nulls.Add(null)
            parent = FindEnvParent(deformer, deformers)
            parentNames[null.FullName] = parent.Name

        for null in nulls:
            parent = model.FindChild(parentNames[null.FullName])
            if parent:
                parent.AddChild(null)

        xsi.ApplyFlexEnv(str(dup) + ";" + nulls.GetAsText(), 0, constants.siConstructionModeAnimation)
        envelope2 = dup.Envelopes(0)

        weights = envelope.GetWeights2()
        weights2 = envelope2.GetWeights2()

        weights2.Array = weights.Array


model = xsi.ActiveSceneRoot.AddModel(None, "NullRig")
BuildNullRig(model, AddNull(model, 2, 0.5, "GlobalSRT"), xsi.Selection)
