#include "utils.h"
#include <xsi_material.h>
#include <xsi_oglmaterial.h>


bool X2UIsModelReferenced(Model& model)
{
  siModelKind kind = model.GetModelKind();
  if (kind == siModelKind_Reference)return true;
  else return false;
}

bool X2UIsModelInstanced(Model& model)
{
  siModelKind kind = model.GetModelKind();
  if (kind == siModelKind_Instance)return true;
  else return false;
}

bool X2UGetObjectVisibility(const CRef& ref, double t)
{
  X3DObject obj(ref);
  Property visibilityProp;
  bool visibility = true;
  obj.GetPropertyFromName(L"Visibility", visibilityProp);
  if(visibilityProp.IsValid())
  {
    visibility = visibilityProp.GetParameterValue(L"ViewVis", t);
  }

  return visibility;
}

GfVec3f X2UGetDisplayColorFromShadingNetwork(const X3DObject& obj)
{
  Material material = obj.GetMaterial();
  if (material.IsValid()) 
  {
    OGLMaterial oglMaterial(material.GetOGLMaterial());
    CColor color = oglMaterial.GetDiffuse();
    return GfVec3f(
      color.r,
      color.g,
      color.b
    );
  }

  return UNDEFINED_COLOR;
}

GfVec3f X2UGetDisplayColorFromWireframeColor(const X3DObject& obj)
{
  Property displayProp;
  SceneItem item(obj);
  item.GetPropertyFromName(L"Display", displayProp);
  if (displayProp.IsValid())
  {
    return GfVec3f(
      displayProp.GetParameterValue(L"WirecolorR"),
      displayProp.GetParameterValue(L"WirecolorG"),
      displayProp.GetParameterValue(L"WirecolorB")
    );
  }

  return UNDEFINED_COLOR;
}

void X2UGetObjectBoundingBox(const X3DObject& obj, GfBBox3d& bbox) 
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

void X2UGetLocalTransformAtTime(const X3DObject& obj, GfMatrix4d& ioMatrix, double t)
{
  MATH::CTransformation xfo = obj.GetKinematics().GetLocal().GetTransform(t);
  MATH::CMatrix4 srcMatrix = xfo.GetMatrix4();
  X2UConvertMatrix4DoubleToDouble(srcMatrix, ioMatrix);
}

void X2UGetGlobalTransformAtTime(const X3DObject& obj, GfMatrix4d& ioMatrix, double t)
{
  MATH::CTransformation xfo = obj.GetKinematics().GetGlobal().GetTransform(t);
  MATH::CMatrix4 srcMatrix = xfo.GetMatrix4();
  X2UConvertMatrix4DoubleToDouble(srcMatrix, ioMatrix);
}