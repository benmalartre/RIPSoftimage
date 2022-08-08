#include "scene.h"

X2UScene::X2UScene(const std::string& folder, const std::string& filename,
  const CRef& root)
  : X2UModel(folder, filename, root)
{
}

X2UScene::~X2UScene()
{
}

void X2UScene::Save()
{
  for (X2UModel& model : _models)
  {
    model.Save();
  }

  _stage->SetStartTimeCode(_timeInfos.startFrame);
  _stage->SetEndTimeCode(_timeInfos.endFrame);
  _stage->SetFramesPerSecond(_timeInfos.framesPerSecond);
 
  _stage->SetDefaultPrim(_rootXform->GetPrim());
  _stage->Save();
}


void X2UScene::Init(size_t options)
{
  // Create Usd stage for writing
  _stage = UsdStage::CreateNew(_folder + "/" + _filename);
  UsdGeomXform rootXform = UsdGeomXform::Define(_stage, SdfPath(_rootName));
  _rootXform = X2UXformSharedPtr(new X2UXform(_rootName, _root));
  _rootXform->Init(_stage, options);
}


// Get time infos from scene
void X2UScene::SetTimeInfosFromScene(double rate)
{
  _timeInfos.InitFromScene(rate);
}

void X2UScene::SetTimeInfos(double startTime, double endTime, double rate)
{
  _timeInfos.InitFromValues(startTime, endTime, rate);
}

void X2UScene::Process(size_t options)
{
  Application app;
  UIToolkit kit = app.GetUIToolkit();
 
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

  ProgressBar progressBar = kit.GetProgressBar();
  progressBar.PutMaximum((_timeInfos.endFrame - _timeInfos.startFrame)/_timeInfos.sampleRate);
  progressBar.PutStep(1);
  progressBar.PutVisible(true);
  progressBar.PutCaption(L"Exporting to USD");

  for (double t = _timeInfos.startFrame; t <= _timeInfos.endFrame; t += _timeInfos.sampleRate)
  {
    progressBar.Increment();
    playControl.PutParameterValue("Current", t);
    Application().ExecuteCommand(L"Refresh", CValueArray(), CValue());
    _WriteSample(t);
    if (progressBar.IsCancelPressed()) break;
    else progressBar.PutStatusText(L"Frame " + CString(t));
  }

  // write prototypes
  _Finalize();
}
