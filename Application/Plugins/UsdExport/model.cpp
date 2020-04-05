#include "model.h"

X2UExportModel::X2UExportModel(const std::string& folder, const std::string& filename,
  const CRef& root)
  : _folder(folder)
  , _filename(filename)
  , _root(root)
{
  _rootName = "/";
  _rootName += X3DObject(root).GetName().GetAsciiString();
}

X2UExportModel::~X2UExportModel()
{
}

void X2UExportModel::Save()
{
  for (X2UExportModel& model : _models)
  {
    model.Save();
  }
  _stage->SetDefaultPrim(_rootXform->GetPrim());
  _stage->Save();
}


void X2UExportModel::Init()
{
  // Create Usd stage for writing
  _stage = UsdStage::CreateNew(_folder + "/" + _filename);
  _rootXform = X2UExportXformSharedPtr(new X2UExportXform(_rootName, _root));
  _rootXform->Init(_stage);
}


void X2UExportModel::_Recurse(const CRef& ref, const std::string& parentPath)
{
  if (ref.IsA(siX3DObjectID))
  {
    X3DObject obj(ref);

    CString type = obj.GetType();
    if (type == L"#model")
    {
      std::string modelFileName = ref.GetAsText().GetAsciiString();
      modelFileName += ".usda";
      X2UExportModel childModel(_folder, modelFileName, ref);
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
        X2UExportXform* xform = new X2UExportXform(objPath, ref);;
        xform->Init(_stage);
        _prims.push_back(X2UExportXformSharedPtr(xform));
      }
      else if (type == L"polymsh")
      {
        X2UExportMesh* mesh = new X2UExportMesh(objPath, ref);;
        mesh->Init(_stage);
        _prims.push_back(X2UExportMeshSharedPtr(mesh));
      }
      else if (type == L"crvlist")
      {
        X2UExportCurve* curve = new X2UExportCurve(objPath, ref);;
        curve->Init(_stage);
        _prims.push_back(X2UExportCurveSharedPtr(curve));
      }
      else if (type == L"pointcloud")
      {
        UsdGeomPoints points = UsdGeomPoints::Define(_stage, SdfPath(objPath));
        //_prims.push_back({ ref, points.GetPrim(), X2U_POINT, objPath });
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

void X2UExportModel::_WriteSample(double t)
{
  UsdTimeCode timeCode(t);
  for (auto& prim : _prims)
  {
    prim->WriteSample(t);
  }
  for (X2UExportModel& model : _models)model._WriteSample(t);
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
