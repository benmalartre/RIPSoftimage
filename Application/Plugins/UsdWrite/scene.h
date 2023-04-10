#pragma once

#include "common.h"
#include "model.h"
#include "utils.h"

// Scene
class X2UScene : public X2UModel{
  public:
  
    X2UScene(const std::string& folder, const std::string& filename, const CRef& root);
    ~X2UScene();
    void Init() override;
    void Save() override;

    void SetTimeInfosFromScene(double rate);
    void SetTimeInfos(double startTime, double endTime, double rate);
    void SetOptions(size_t options);
    void SetAttributes(const CStringArray& attributes);
    void Process();

    const CStringArray& GetAttributes(){return _attributes;};
    bool IsSelected(const CRef& ref);

  private:
    void                _GetSelection();
    X2UTimeInfos        _timeInfos;
    size_t              _options;
    CRefArray           _selection;
    CStringArray        _attributes;
};

typedef std::vector<X2UScene> X2USceneList;
