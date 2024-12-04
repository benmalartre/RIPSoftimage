#include "subset.h"
#include "utils.h"

X2USubset::X2USubset(std::string path, const CRef& ref)
  : X2UPrim(path, ref)
{
  Cluster cluster(_ref);
  LOG("X2USubset constructor : "+ cluster.GetName());
}

X2USubset::~X2USubset()
{
}

void X2USubset::Init(UsdStageRefPtr& stage)
{
  UsdGeomSubset subset = UsdGeomSubset::Define(stage, SdfPath(_fullname));
  _prim = subset.GetPrim();

  Cluster cluster(_ref);

  LOG("Created subset : " + CString(_prim.GetPath().GetText()));

  // type attribute
  {
    _attributes["type"] =
      X2UAttribute(
        subset.CreateElementTypeAttr(VtValue(), true),
        X2U_DATA_TOKEN,
        false);

    // set default value
    CString type = cluster.GetType();
    if(type == siVertexCluster)
      _attributes["type"].WriteSample(UsdGeomTokens->vertex, UsdTimeCode::Default());
    else if(type == siEdgeCluster)
      _attributes["type"].WriteSample(UsdGeomTokens->edge, UsdTimeCode::Default());
    else if(type == siPolygonCluster)
      _attributes["type"].WriteSample(UsdGeomTokens->face, UsdTimeCode::Default());
        
  }

  // indices attribute
  {
    CLongArray indices = cluster.GetElements().GetArray();
    size_t numIndices = indices.GetCount();
    LOG("CLUSTER NUM INDICES : "+indices.GetCount());

    _attributes["indices"] =
      X2UAttribute(
        subset.CreateIndicesAttr(VtValue(), true),
        X2U_DATA_LONG,
        true);


    // set default value
    _attributes["indices"].WriteSample((const void*)&indices[0], 
      numIndices, UsdTimeCode::Default());
  }
}

void X2USubset::WriteSample(double t)
{
  UsdTimeCode timeCode(t);

  Cluster cluster(_ref);
  CLongArray indices = cluster.GetElements().GetArray();
  size_t numIndices = indices.GetCount();

  X2UAttribute& item = GetAttribute("indices");
  item.WriteSample((const void*)&indices[0], numIndices, UsdTimeCode::Default());

}


