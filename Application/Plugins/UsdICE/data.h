#ifndef U2I_DATA_H
#define U2I_DATA_H

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/layer.h>
#include "prim.h"

class U2IData
{
public:
  U2IData();
	~U2IData();
	
	pxr::UsdStageRefPtr _stage;
	pxr::SdfLayerRefPtr _layer;
};

#endif