#include "scene.h"

X2UExportScene::X2UExportScene(const std::string& folder, const std::string& filename,
  const CRef& root)
  : X2UExportModel(folder, filename, root)
{
}

X2UExportScene::~X2UExportScene()
{
}

void X2UExportScene::Save()
{
  for (X2UExportModel& model : _models)
  {
    model.Save();
  }

  _stage->SetStartTimeCode(_timeInfos.startFrame);
  _stage->SetEndTimeCode(_timeInfos.endFrame);
  _stage->SetFramesPerSecond(_timeInfos.framesPerSecond);
 
  _stage->SetDefaultPrim(_rootXform->GetPrim());
  _stage->Save();
}


void X2UExportScene::Init()
{
  TimeInfos();

  // Create Usd stage for writing
  _stage = UsdStage::CreateNew(_folder + "/" + _filename);
  UsdGeomXform rootXform = UsdGeomXform::Define(_stage, SdfPath(_rootName));
  _rootXform = X2UExportXformSharedPtr(new X2UExportXform(_rootName, _root));
  _rootXform->Init(_stage);
}


// Get time infos from scene
void X2UExportScene::TimeInfos()
{
  // Get the current project
  Application app;
  Project project = app.GetActiveProject();

  // The PlayControl property set is stored with scene data under the project
  Property playControl = project.GetProperties().GetItem(L"Play Control");

  _timeInfos.startFrame = playControl.GetParameterValue(L"In");
  _timeInfos.endFrame = playControl.GetParameterValue(L"Out");
  switch ((int)playControl.GetParameterValue(L"Format"))
  {
  case 13:
    _timeInfos.framesPerSecond = 23.976;
    break;
  case 7:
    _timeInfos.framesPerSecond = 24.0; // FILM
    break;
  case 8:
    _timeInfos.framesPerSecond = 25.0; // PAL
    break;
  case 10:
    _timeInfos.framesPerSecond = 29.97; // NTSC
    break;
  case 19:
    _timeInfos.framesPerSecond = 30;
    break;
  case 25:
    _timeInfos.framesPerSecond = 59.94;
    break;
  case 11:
    _timeInfos.framesPerSecond = playControl.GetParameterValue(L"Rate");
    break;
  default:
    _timeInfos.framesPerSecond = 24.0; // FILM
    break;
  }
  
  _timeInfos.sampleRate = 1;
}

void X2UExportScene::Process()
{
  // first build usd structure
  std::string rootPath = _rootName;
  CRefArray children = _root.GetChildren();
  for (int j = 0; j < children.GetCount(); ++j)
  {
    _Recurse(children[j], rootPath);
  }

  // then loop over time range writing samples
  Project project = Application().GetActiveProject();
  Property playControl = project.GetProperties().GetItem(L"Play Control");

  for (double t = _timeInfos.startFrame; t <= _timeInfos.endFrame; t += _timeInfos.sampleRate)
  {
    playControl.PutParameterValue("Current", t);
    Application().ExecuteCommand(L"Refresh", CValueArray(), CValue());
    _WriteSample(t);
  }
}
