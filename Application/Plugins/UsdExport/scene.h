#pragma once

#include "common.h"
#include "model.h"

// Scene
class X2UExportScene : public X2UExportModel{
  public:
    X2UExportScene(const std::string& folder, const std::string& filename, const CRef& root);
    ~X2UExportScene();
    void Init() override;
    void Save() override;

    void TimeInfos();
    void Process();

  private:
    X2UExportTimeInfos              _timeInfos;

};

typedef std::vector<X2UExportScene> X2UExportSceneList;
