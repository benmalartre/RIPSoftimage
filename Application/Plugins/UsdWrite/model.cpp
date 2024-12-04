#include "model.h"
#include "mesh.h"
#include "curve.h"
#include "points.h"
#include "instancer.h"
#include "subset.h"
#include "skeleton.h"
#include "camera.h"
#include "scene.h"

X2UModel::X2UModel(const std::string& folder, const std::string& filename,
  const CRef& root, X2UModel* parent, bool isSkelRoot)
  : _folder(folder)
  , _filename(filename)
  , _root(root)
  , _parent(parent)
  , _selected(false)
  , _isSkelRoot(isSkelRoot)
{
  _rootName = "/";
  _rootName += X3DObject(root).GetName().GetAsciiString();
}

X2UModel::~X2UModel()
{
}

void X2UModel::Save()
{
  for (X2UModel& model : _models)
  {
    model.Save();
  }
  _stage->SetDefaultPrim(_rootXform->GetPrim());
  _stage->Save();
}


void X2UModel::Init()
{
  // Create Usd stage for writing
  _stage = UsdStage::CreateNew(_folder + "/" + _filename);
  if(_isSkelRoot) {
    _rootXform = X2UPrimSharedPtr(new X2USkeleton(_rootName, _root));
  } else {
    _rootXform = X2UPrimSharedPtr(new X2UXform(_rootName, _root));
  }
 
  _rootXform->Init(_stage);
}

X2UScene* X2UModel::GetScene()
{
  X2UModel* parent = GetParent();
  if (parent) return parent->GetScene();
  return (X2UScene*)this;
}

X2UModel* X2UModel::GetParent()
{
  return _parent;
}

void X2UModel::_Recurse(const CRef& ref, const std::string& parentPath, size_t options)
{
  if (ref.IsA(siX3DObjectID))
  {
    X3DObject obj(ref);

    CString type = obj.GetType();

    if (type == L"#model")
    {
      if (GetScene()->IsSelected(ref)) _selected = true;

      if (!(options & X2U_EXPORT_SELECTION) || _selected) {
        std::string modelFileName = ref.GetAsText().GetAsciiString();
        modelFileName += ".usda";
        X2UModel childModel(_folder, modelFileName, ref, this, options & X2U_EXPORT_SKELETONS);
        childModel.Init();

        std::string modelPath = childModel._GetRootName();

        CRefArray children = X3DObject(ref).GetChildren();
        for (int j = 0; j < children.GetCount(); ++j)
        {
          childModel._Recurse(children[j], modelPath, options);
        }

        // add reference to the file external model file
        childModel.SetPath(parentPath + childModel._GetRootName());
        UsdPrim refPrim = _stage->OverridePrim(childModel.GetPath());
        refPrim.GetReferences().AddReference(_folder + "/" + modelFileName);
        _models.push_back(childModel);
        _xObjPathMap[childModel.GetID()] = childModel.GetPath();
      }
    } 
    else 
    {
      std::string objPath = parentPath + "/" + obj.GetName().GetAsciiString();
      _selected = _selected || GetScene()->IsSelected(ref);

      if (!(options & X2U_EXPORT_SELECTION) || _selected) {
        if (type == L"null")
        {
          X2UXform* xform = new X2UXform(objPath, ref);;
          xform->Init(_stage);
          _prims.push_back(X2UXformSharedPtr(xform));
          _xObjPathMap[xform->GetID()] = xform->GetPath();
        }
        else if (type == L"CameraRoot" && (options & X2U_EXPORT_CAMERAS))
        {
          X2UXform* xform = new X2UXform(objPath, ref);;
          xform->Init(_stage);
          _prims.push_back(X2UXformSharedPtr(xform));
          _xObjPathMap[xform->GetID()] = xform->GetPath();
        }
        else if (type == L"camera" && (options & X2U_EXPORT_CAMERAS))
        {
          X2UCamera* camera = new X2UCamera(objPath, ref);
          camera->Init(_stage);
          _prims.push_back(X2UCameraSharedPtr(camera));
          _xObjPathMap[camera->GetID()] = camera->GetPath();
        }
        else if (type == L"polymsh" && (options & X2U_EXPORT_MESHES))
        {
          X2UMesh* mesh = new X2UMesh(objPath, ref);;
          mesh->Init(_stage);
          _prims.push_back(X2UMeshSharedPtr(mesh));
          _xObjPathMap[mesh->GetID()] = mesh->GetPath();
          if(options & X2U_EXPORT_SUBSETS) {
            mesh->BuildGeomSubsetsDescs(GetScene()->GetSubsetNames());
            for(const X2UGeomSubsetDesc& subsetDesc: mesh->GetSubsetDescs()) {
              X2USubset* subset = new X2USubset(subsetDesc.path.GetString(), subsetDesc.cluster);
              subset->Init(_stage);
              _prims.push_back(X2USubsetSharedPtr(subset));
              _xObjPathMap[subset->GetID()] = subset->GetPath();
            }
          }
        }
        else if (type == L"crvlist" && (options & X2U_EXPORT_CURVES))
        {
          X2UCurve* curve = new X2UCurve(objPath, ref, X2U_CURVE_NURBS);;
          curve->Init(_stage);
          _prims.push_back(X2UCurveSharedPtr(curve));
          _xObjPathMap[curve->GetID()] = curve->GetPath();
        }
        else if (type == L"pointcloud" && (options & X2U_EXPORT_POINTS))
        {
          bool isStrandPointCloud = X2UIsStrandPointCloud(obj);
          if (isStrandPointCloud) {
            X2UCurve* curve = new X2UCurve(objPath, ref, X2U_CURVE_STRANDS);
            curve->Init(_stage);
            _prims.push_back(X2UCurveSharedPtr(curve));
            _xObjPathMap[curve->GetID()] = curve->GetPath();
          }
          else {
            X2UPoints* points = new X2UPoints(objPath, ref);
            points->Init(_stage);
            _prims.push_back(X2UPointsSharedPtr(points));
            _xObjPathMap[points->GetID()] = points->GetPath();
            /*
            X2UInstancer* instancer = new X2UInstancer(objPath, ref);
            instancer->Init(_stage);
            _prims.push_back(X2UInstancerSharedPtr(instancer));
            _xObjPathMap[instancer->GetID()] = instancer->GetPath();
            */
          }
        }
        else if (type == L"root" && (options & X2U_EXPORT_SKELETONS))
        {
          X2USkeleton* skeleton = new X2USkeleton(objPath, ref);
          skeleton->Init(_stage);
          _prims.push_back(X2USkeletonSharedPtr(skeleton));
          _xObjPathMap[skeleton->GetID()] = skeleton->GetPath();
          return;
        }
      }

      CRefArray children = obj.GetChildren();
      for (int j = 0; j < children.GetCount(); ++j)
      {
        _Recurse(children[j], objPath, options);
      }
      
    }
  }
}

void X2UModel::_WriteSample(double t)
{
  _rootXform->WriteSample(t);
  UsdTimeCode timeCode(t);
  for (auto& prim : _prims)
  {
    prim->WriteSample(t);
  }
  for (X2UModel& model : _models)
  {
    model._WriteSample(t);
  }
}

void X2UModel::_Finalize()
{
  for (auto& prim : _prims)
  {
    prim->Term(_stage, _xObjPathMap);
  }
  for (X2UModel& model : _models)
  {
    model._Finalize();
  }
}