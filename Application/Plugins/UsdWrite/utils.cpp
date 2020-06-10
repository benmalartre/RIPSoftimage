#include "utils.h"
#include <xsi_material.h>
#include <xsi_oglmaterial.h>
#include <xsi_shape.h>
#include <sys/stat.h>

void X2UTimeInfos::InitFromScene()
{
  // Get the current project
  Application app;
  Project project = app.GetActiveProject();

  // The PlayControl property set is stored with scene data under the project
  Property playControl = project.GetProperties().GetItem(L"Play Control");

  startFrame = playControl.GetParameterValue(L"In");
  endFrame = playControl.GetParameterValue(L"Out");
  switch ((int)playControl.GetParameterValue(L"Format"))
  {
  case 13:
    framesPerSecond = 23.976;
    break;
  case 7:
    framesPerSecond = 24.0; // FILM
    break;
  case 8:
    framesPerSecond = 25.0; // PAL
    break;
  case 10:
    framesPerSecond = 29.97; // NTSC
    break;
  case 19:
    framesPerSecond = 30;
    break;
  case 25:
    framesPerSecond = 59.94;
    break;
  case 11:
    framesPerSecond = playControl.GetParameterValue(L"Rate");
    break;
  default:
    framesPerSecond = 24.0; // FILM
    break;
  }

  sampleRate = 1;
}

void X2UTimeInfos::Set(double start, double end, double rate)
{
  startFrame = start;
  endFrame = end;
  sampleRate = rate;
  framesPerSecond = 24;
}

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

bool X2UIsStrandPointCloud(const X3DObject& obj)
{
  ICEAttribute attr = obj.GetActivePrimitive().GetICEAttributeFromName("StrandPosition");
  if (attr.IsValid() && attr.GetElementCount()) {
    return true;
  }
  else return false;
}

bool X2UGetObjectVisibility(const X3DObject& obj, double t)
{
  Property visibilityProp;
  bool visibility = true;
  SceneItem(obj).GetPropertyFromName(L"Visibility", visibilityProp);
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

void X2UGetLocalTransform(const X3DObject& obj, GfMatrix4d& ioMatrix)
{
  MATH::CTransformation xfo = obj.GetKinematics().GetLocal().GetTransform();
  MATH::CMatrix4 srcMatrix = xfo.GetMatrix4();
  X2UConvertMatrix4DoubleToDouble(srcMatrix, ioMatrix);
}

void X2UGetGlobalTransform(const X3DObject& obj, GfMatrix4d& ioMatrix)
{
  MATH::CTransformation xfo = obj.GetKinematics().GetGlobal().GetTransform();
  MATH::CMatrix4 srcMatrix = xfo.GetMatrix4();
  X2UConvertMatrix4DoubleToDouble(srcMatrix, ioMatrix);
}


ICEAttribute X2UGetICEAttributeFromArray(CRefArray& attributes, const CString& name, int& index)
{
  for (int i = 0; i < attributes.GetCount(); ++i)
  {
    ICEAttribute attribute(attributes[i]);
    if (attribute.GetName() == name)
    {
      index = i;
      return attribute;
    }
  }
  index = -1;
  return ICEAttribute();
}

bool X2UCheckFolder(const CString& folder)
{
  if (folder == "")return false;

  const char* path = folder.GetAsciiString();
  struct stat info;
  if (stat(path, &info) != 0)return false;
  return info.st_mode & S_IFDIR;
}

bool X2UCheckFilename(const CString& filename)
{
  return true;
}