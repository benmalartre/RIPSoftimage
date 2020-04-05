#include "scene.h"
#include "xform.h"
#include "mesh.h"
#include "curve.h"

X2UExportScene::X2UExportScene(const std::string& folder, const std::string& filename,
  const CRef& root, bool isModel)
  : _folder(folder)
  , _filename(filename)
  , _root(root)
  , _isModel(isModel)
{
  _rootName = "/";
  _rootName += X3DObject(root).GetName().GetAsciiString();
}

X2UExportScene::~X2UExportScene()
{
}

void X2UExportScene::Save()
{
  for (X2UExportScene& model : _models)
  {
    model.Save();
  }

  if (!_isModel)
  {
    _stage->SetStartTimeCode(_timeInfos.startFrame);
    _stage->SetEndTimeCode(_timeInfos.endFrame);
  }
  _stage->SetDefaultPrim(_rootPrim);
  _stage->Save();
}


void X2UExportScene::Init()
{
  if (!_isModel)TimeInfos();

  // Create Usd stage for writing
  _stage = UsdStage::CreateNew(_folder + "/" + _filename);
  UsdGeomXform rootXform = UsdGeomXform::Define(_stage, SdfPath(_rootName));
  _rootPrim = rootXform.GetPrim();
}


// Get time infos from scene
void X2UExportScene::TimeInfos()
{
  // Get the current project
  Application app;
  Project project = app.GetActiveProject();

  // The PlayControl property set is stored with scene data under the project
  Property playControl = project.GetProperties().GetItem(L"Play Control");

  _timeInfos.startFrame = 1;// playControl.GetParameterValue(L"In");
  _timeInfos.endFrame = 24;// playControl.GetParameterValue(L"Out");
  _timeInfos.sampleRate = 1;
}

void X2UExportScene::Recurse(const CRef& ref, const std::string& parentPath)
{
  if (ref.IsA(siX3DObjectID))
  {
    X3DObject obj(ref);

    CString type = obj.GetType();
    if (type == L"#model")
    {
      
      std::string modelFileName = ref.GetAsText().GetAsciiString();
      modelFileName += ".usda";
      X2UExportScene modelScene(_folder, modelFileName, ref, true);
      modelScene.Init();

      std::string modelPath = "";

      CRefArray children = X3DObject(ref).GetChildren();
      for (int j = 0; j < children.GetCount(); ++j)
      {
        modelScene.Recurse(children[j], modelPath);
      }

      _models.push_back(modelScene);
    }
    else
    {
      std::string objPath = parentPath + "/" + obj.GetName().GetAsciiString();
      if (type == L"null")
      {
        X2UExportXform* xform = new X2UExportXform();;
        xform->Init(_stage, objPath, ref);
        _prims.push_back(X2UExportXformSharedPtr(xform));
      }
      else if (type == L"polymsh")
      {
        X2UExportMesh* mesh = new X2UExportMesh();;
        mesh->Init(_stage, objPath, ref);
        _prims.push_back(X2UExportMeshSharedPtr(mesh));
      }
      else if (type == L"crvlist")
      {
        X2UExportCurve* curve = new X2UExportCurve();;
        curve->Init(_stage, objPath, ref);
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
        Recurse(children[j], objPath);
      }
    }
  }
}

void X2UExportScene::WriteSample(double t)
{
  UsdTimeCode timeCode(t);
  for (auto& prim : _prims)
  {
    prim->WriteSample(t);
  }
}

void X2UExportScene::Process()
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
    for (X2UExportScene& model: _models)model.WriteSample(t);
    WriteSample(t);
  }
}
