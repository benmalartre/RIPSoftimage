 #pragma once
#include <xsi_application.h>
#include <xsi_uitoolkit.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_factory.h>
#include <xsi_status.h>
#include <xsi_vector2f.h>
#include <xsi_vector3f.h>
#include <xsi_vector4f.h>
#include <xsi_matrix3f.h>
#include <xsi_matrix4f.h>
#include <xsi_rotationf.h>
#include <xsi_quaternionf.h>
#include <xsi_color4f.h>
#include <xsi_shape.h>
#include <xsi_icegeometry.h>
#include <xsi_icenodedef.h>
#include <xsi_iceportstate.h>
#include <xsi_indexset.h>
#include <xsi_iceattribute.h>
#include <xsi_iceattributedataarray.h>
#include <xsi_iceattributedataarray2D.h>
#include <xsi_icenodecontext.h>
#include <pxr/usd/usd/stage.h>

#define U2XNodeR 128
#define U2XNodeG 192
#define U2XNodeB 168

#define U2XLayerDataR 90
#define U2XLayerDataG 130
#define U2XLayerDataB 150

enum U2XReadStageNodeIDs
{
  ID_IN_Filename = 0,
  ID_IN_DefaultPrim = 1,
  ID_IN_LoadState = 2,

  ID_G_100 = 100,
  ID_OUT_Stage = 200,

  ID_TYPE_CNS = 400,
  ID_STRUCT_CNS,
  ID_CTXT_CNS,
  ID_UNDEF = ULONG_MAX
};

enum U2XAddPrimNodeIDs
{
  ID_IN_Stage = 0,
  ID_IN_Name = 1,
  ID_IN_Xform = 2
};

enum U2XAddSphereNodeIDs
{
  ID_IN_Radius = 3
};

enum U2XAddCubeNodeIDs
{
  ID_IN_Width = 3,
  ID_IN_Depth = 4,
  ID_IN_Height = 5
};

struct U2XPrimNodeDatas_t {

};

struct U2XStageNodeDatas_t {
  pxr::UsdStageWeakPtr _stage;
  std::string _uri;

  U2XStageNodeDatas_t() : _stage(nullptr), _uri("") {};
};


using namespace XSI;
SICALLBACK UsdStageNode_Evaluate(ICENodeContext& in_ctxt);
SICALLBACK UsdStageNode_Init(CRef& in_ctxt);
SICALLBACK UsdStageNode_Term(CRef& in_ctxt);

SICALLBACK UsdSphereNode_Evaluate(ICENodeContext& in_ctxt);
SICALLBACK UsdSphereNode_Init(CRef& in_ctxt);
SICALLBACK UsdSphereNode_Term(CRef& in_ctxt);

  