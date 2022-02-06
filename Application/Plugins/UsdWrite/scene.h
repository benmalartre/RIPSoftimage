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
    void Process();

  private:
    X2UTimeInfos     _timeInfos;

};

typedef std::vector<X2UScene> X2USceneList;
