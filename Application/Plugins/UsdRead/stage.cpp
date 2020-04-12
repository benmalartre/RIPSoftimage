#include "stage.h"
#include "utils.h"

U2XStage::U2XStage(const std::string& filename)
  :_filename(filename), _lastEvalID(-1)
{
  if (pxr::UsdStage::IsSupportedFile(_filename))
  {
    _stage = pxr::UsdStage::Open(_filename);
  }
  else LOG("Unsupported file for reading : " + CString(_filename.c_str()));
}

U2XStage::~U2XStage()
{
  for (int i = 0; i < _prims.size(); ++i) {
    if (_prims[i])delete _prims[i];
  }
}