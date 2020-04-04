#pragma once

#include "common.h"

static const GfVec4f UNDEFINED_COLOR = { 1.f,0.25f,0.5f,1.f };

GfVec4f GetDisplayColorFromShadingNetwork(const X3DObject& obj);

void GetObjectBoundingBox(const X3DObject& obj, GfBBox3d& bbox);
