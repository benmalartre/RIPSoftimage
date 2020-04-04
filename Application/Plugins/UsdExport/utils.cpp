#include "utils.h"
#include <xsi_material.h>
#include <xsi_oglmaterial.h>

GfVec4f GetDisplayColorFromShadingNetwork(const X3DObject& obj)
{
  Material material = obj.GetMaterial();
  if (material.IsValid()) {
    OGLMaterial oglMaterial(material.GetOGLMaterial());
    CColor color = oglMaterial.GetDiffuse();
    return GfVec4f(
      color.r,
      color.g,
      color.b,
      color.a
    );
  }

  return UNDEFINED_COLOR;
}

void GetObjectBoundingBox(const X3DObject& obj, GfBBox3d& bbox) 
{
  GfVec3d bboxMin, bboxMax;
  obj.GetBoundingBox(
    bboxMin[0],
    bboxMin[1],
    bboxMin[2],
    bboxMax[0],
    bboxMax[1],
    bboxMax[2]
  );
  bbox.SetRange(GfRange3d(bboxMin, bboxMax));
}