#include "instancer.h"
#include "utils.h"

X2UInstancer::X2UInstancer(std::string path, const CRef& ref)
  : X2UPrim(path, ref)
  , _protoHash(0)
{
}

X2UInstancer::~X2UInstancer()
{ 
}

bool X2UInstancer::_GetNumPoints(const Geometry& geom)
{
  ICEAttribute nbPointsAttr = geom.GetICEAttributeFromName(L"NbPoints");

  CICEAttributeDataArrayLong nbPoints;
  nbPointsAttr.GetDataArray(nbPoints);
  bool topoChanged = false;
  if (nbPoints[0] != _numPoints)topoChanged = true;
  _numPoints = nbPoints[0];
  return topoChanged;
}

void X2UInstancer::_ComputeBoundingBox(const Geometry& geom)
{
  if (_numPoints)
  {

    ICEAttribute pointPositionAttr = geom.GetICEAttributeFromName(L"PointPosition");
    CICEAttributeDataArrayVector3f pointPositions;
    pointPositionAttr.GetDataArray(pointPositions);

    _bbox = X2UComputeBoundingBox<float>(&pointPositions[0][0], _numPoints);
  }
  else
    _bbox = GfBBox3d(GfRange3d(GfVec3d(0), GfVec3d(0)));
}


int X2UInstancer::AddInstanceShape(const XSI::MATH::CShape& shape)
{
  int index = 0;
  for (const X2UInstancerShape& instancerShape : _shapes)
  {
    XSI::siICEShapeType shapeType = instancerShape.type;
    if (shapeType == shape.GetType())
    {
      if (shapeType == XSI::siICEShapeReference)
      {
        XSI::MATH::CShape::ParameterReference desc = shape.GetReferenceDescription();
        if (desc.m_nObjectID == instancerShape.reference)return index;
      }
      else return index;
    }
    index++;
  }

  if (shape.GetType() == XSI::siICEShapeReference)
    _shapes.push_back({ shape.GetType(), (int)shape.GetReferenceDescription().m_nObjectID });
  else  _shapes.push_back({ shape.GetType(), -1 });
  return _shapes.size() - 1;
}

void X2UInstancer::AddPrototypesRelationship(UsdStageRefPtr& stage, const std::map<ULONG, SdfPath>& objPathMap)
{
  UsdPrim protoRoot = stage->DefinePrim(SdfPath(_fullname + "/Prototypes"));
  SdfPath protoPath = protoRoot.GetPath();
  UsdRelationship protoRel = UsdGeomPointInstancer(_prim).GetPrototypesRel();

  for (const X2UInstancerShape& instancerShape : _shapes)
  {
    switch (instancerShape.type)
    {
    case XSI::siICEShapePoint:
    case XSI::siICEShapeSegment:
    case XSI::siICEShapeDisc:
    case XSI::siICEShapeRectangle:
    case XSI::siICEShapeBox:
    {
      UsdGeomCube protoBox = UsdGeomCube::Define(stage, protoRoot.GetPath().AppendChild(TfToken("Box")));
      protoRel.AddTarget(protoBox.GetPath());
      break;
    }
      
    case XSI::siICEShapeSphere:
    {
      UsdGeomSphere protoSphere = UsdGeomSphere::Define(stage, protoRoot.GetPath().AppendChild(TfToken("Sphere")));
      protoRel.AddTarget(protoSphere.GetPath());
      break;
    }
    case XSI::siICEShapeCapsule:
    {
      UsdGeomCapsule protoCapsule = UsdGeomCapsule::Define(stage, protoRoot.GetPath().AppendChild(TfToken("Capsule")));
      protoRel.AddTarget(protoCapsule.GetPath());
      break;
    }
    case XSI::siICEShapeCone:
    {
      UsdGeomCone protoCone = UsdGeomCone::Define(stage, protoRoot.GetPath().AppendChild(TfToken("Cone")));
      protoRel.AddTarget(protoCone.GetPath());
      break;
    }
    case XSI::siICEShapeInstance:
    case XSI::siICEShapeReference:
    {
      ULONG objectID = instancerShape.reference;
      const auto& objPathIt = objPathMap.find(objectID);
      if (objPathIt != objPathMap.end())
      {
        SdfPath oldPath = objPathIt->second;
        SdfPath newPath = protoRoot.GetPath().AppendChild(oldPath.GetNameToken());
        SdfBatchNamespaceEdit edit;
        edit.Add(SdfNamespaceEdit::Reparent(oldPath, protoRoot.GetPath(), 0));
        stage->GetRootLayer()->Apply(edit);

        UsdPrim protoRef = stage->GetPrimAtPath(newPath);
        protoRel.AddTarget(protoRef.GetPath());
      }
      
      break;
    }
    }
  }
}

void X2UInstancer::Init(UsdStageRefPtr& stage)
{
  UsdTimeCode timeCode = UsdTimeCode::Default();
  UsdGeomPointInstancer instancer = UsdGeomPointInstancer::Define(stage, SdfPath(_fullname));

  _prim = instancer.GetPrim();

  Geometry geom = _xPrim.GetGeometry();
  CRefArray iceAttributes = geom.GetICEAttributes();

  // get num points
  _GetNumPoints(geom);

  // points
  InitAttributeFromICE(geom, "PointPosition", "positions", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "positions");

  // velocities
  InitAttributeFromICE(geom, "PointVelocity", "velocities", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "velocities");

  // ids
  InitAttributeFromICE(geom, "ID", "ids", SdfValueTypeNames->Int64Array);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "ids");

  // orientation
  InitAttributeFromICE(geom, "Orientation", "orientations", SdfValueTypeNames->QuathArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "orientations");

  // scale
  InitAttributeFromICE(geom, "Scale", "scales", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "scales");

  // colors
  //instancer.CreatePrimvar(TfToken("color"), SdfValueTypeNames->Vector3fArray, UsdGeomTokens->vertex, _numPoints);
  InitAttributeFromICE(geom, "Color", "colors", SdfValueTypeNames->Vector3fArray);
  WriteSampleFromICE(geom, UsdTimeCode::Default(), "colors");

  // extent attribute
  _ComputeBoundingBox(geom);
  InitExtentAttribute();

  // xform attribute
  InitTransformAttribute();

  // visibility attribute
  InitVisibilityAttribute();

  // prototypes attribute
  UsdAttribute protoIndicesAttr = instancer.CreateProtoIndicesAttr();
  UsdRelationship protoRel = instancer.CreatePrototypesRel();
  WritePrototypesSample(UsdTimeCode::Default());

}

void X2UInstancer::WriteSample(double t)
{
  UsdTimeCode timeCode(t);

  Geometry geom = _xPrim.GetGeometry(t, siConstructionModeSecondaryShape);

  bool topoChanged = _GetNumPoints(geom);

  WriteSampleFromICE(geom, timeCode, "positions");
  WriteSampleFromICE(geom, timeCode, "velocities");
  WriteSampleFromICE(geom, timeCode, "accelerations");
  WriteSampleFromICE(geom, timeCode, "ids");
  WriteSampleFromICE(geom, timeCode, "orientations");
  WriteSampleFromICE(geom, timeCode, "scales");
  WriteSampleFromICE(geom, timeCode, "colors");

  // prototypes attribute
  WritePrototypesSample(timeCode);

  // extent attribute
  _ComputeBoundingBox(geom);
  WriteExtentSample(t);

  // xform attribute
  WriteTransformSample(t);

  // visibility attribute
  WriteVisibilitySample(t);
}

void X2UInstancer::WritePrototypesSample(const UsdTimeCode& timeCode)
{
  Geometry geom = _xPrim.GetGeometry(timeCode.GetValue());
  ICEAttribute shapeAttr = geom.GetICEAttributeFromName("Shape");
  if (shapeAttr.IsDefined() && shapeAttr.GetElementCount() > 0)
  {
    CICEAttributeDataArray<XSI::MATH::CShape> shapes;
    shapeAttr.GetDataArray(shapes);
    VtArray<int> protoIndices(shapes.GetCount());
    for (int i = 0; i < shapes.GetCount(); ++i)
    {
      protoIndices[i] = AddInstanceShape(shapes[i]);
    }
    size_t hash = ArchHash((const char*)&protoIndices[0], protoIndices.size() * sizeof(int));
    if (hash != _protoHash)
    {
      UsdAttribute protoIndicesAttr = UsdGeomPointInstancer(_prim).GetProtoIndicesAttr();
      protoIndicesAttr.Set(protoIndices, timeCode);
      _protoHash = hash;
    }
  }
}

void X2UInstancer::Term(UsdStageRefPtr& stage, const X2UObjectPathMap& objPathMap)
{
  AddPrototypesRelationship(stage, objPathMap);
}
