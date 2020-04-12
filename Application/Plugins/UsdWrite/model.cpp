#include "model.h"

X2UModel::X2UModel(const std::string& folder, const std::string& filename,
  const CRef& root)
  : _folder(folder)
  , _filename(filename)
  , _root(root)
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
  _rootXform = X2UXformSharedPtr(new X2UXform(_rootName, _root));
  _rootXform->Init(_stage);
}


void X2UModel::_Recurse(const CRef& ref, const std::string& parentPath)
{
  if (ref.IsA(siX3DObjectID))
  {
    X3DObject obj(ref);

    CString type = obj.GetType();
    if (type == L"#model")
    {
      std::string modelFileName = ref.GetAsText().GetAsciiString();
      modelFileName += ".usda";
      X2UModel childModel(_folder, modelFileName, ref);
      childModel.Init();

      std::string modelPath = childModel._GetRootName();

      CRefArray children = X3DObject(ref).GetChildren();
      for (int j = 0; j < children.GetCount(); ++j)
      {
        childModel._Recurse(children[j], modelPath);
      }

      // add reference to the file external model file
      std::string referenceName = parentPath + childModel._GetRootName();
      UsdPrim refPrim = _stage->OverridePrim(SdfPath(referenceName));
      refPrim.GetReferences().AddReference(_folder + "/" + modelFileName);

      _models.push_back(childModel);
    }
    else
    {
      std::string objPath = parentPath + "/" + obj.GetName().GetAsciiString();
      if (type == L"null")
      {
        X2UXform* xform = new X2UXform(objPath, ref);;
        xform->Init(_stage);
        _prims.push_back(X2UXformSharedPtr(xform));
      }
      else if (type == L"polymsh")
      {
        X2UMesh* mesh = new X2UMesh(objPath, ref);;
        mesh->Init(_stage);
        _prims.push_back(X2UMeshSharedPtr(mesh));
      }
      else if (type == L"crvlist")
      {
        X2UCurve* curve = new X2UCurve(objPath, ref);;
        curve->Init(_stage);
        _prims.push_back(X2UCurveSharedPtr(curve));
      }
      else if (type == L"pointcloud")
      {
        X2UPoints* point = new X2UPoints(objPath, ref);;
        point->Init(_stage);
        _prims.push_back(X2UPointSharedPtr(point));
      }
      else if (type == L"camera")
      {
        UsdGeomCamera camera = UsdGeomCamera::Define(_stage, SdfPath(objPath));
        //_prims.push_back({ ref, camera.GetPrim(), X2U_CAMERA, objPath });
      }

      CRefArray children = obj.GetChildren();
      for (int j = 0; j < children.GetCount(); ++j)
      {
        _Recurse(children[j], objPath);
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

/*
void X2UExportModel::Process()
{
  // first build usd structure
  std::string rootPath = _rootName;
  CRefArray children = _root.GetChildren();
  for (int j = 0; j < children.GetCount(); ++j)
  {
    Recurse(children[j], rootPath);
  }

  // then loop over time range writing samples
  Project project = Application().GetActiveProject();
  Property playControl = project.GetProperties().GetItem(L"Play Control");

  for (double t = _timeInfos.startFrame; t <= _timeInfos.endFrame; t += _timeInfos.sampleRate)
  {
    playControl.PutParameterValue("Current", t);
    for (X2UExportModel& model: _models)model.WriteSample(t);
    WriteSample(t);
  }
}
*/
