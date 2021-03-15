#ifndef __DELTA_MUSH__
#define __DELTA_MUSH__

#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>

#include <xsi_icenodecontext.h>
#include <xsi_icenodedef.h>
#include <xsi_command.h>
#include <xsi_factory.h>
#include <xsi_longarray.h>
#include <xsi_doublearray.h>
#include <xsi_math.h>
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
#include <xsi_iceportstate.h>
#include <xsi_indexset.h>
#include <xsi_dataarray.h>
#include <xsi_dataarray2D.h>

#include "vector"

using namespace XSI; 

//Declare XSI function.
SICALLBACK IRDeltaMushInit_BeginEvaluate ( ICENodeContext& in_ctxt );
SICALLBACK IRDeltaMushInit_Evaluate ( ICENodeContext&	in_ctxt );
SICALLBACK IRDeltaMushInit_EndEvaluate ( ICENodeContext& in_ctxt );
SICALLBACK IRDeltaMush_Init(ICENodeContext& in_ctxt);
SICALLBACK IRDeltaMush_Terminate(ICENodeContext& in_ctxt);
SICALLBACK IRDeltaMush_BeginEvaluate ( ICENodeContext& in_ctxt );
SICALLBACK IRDeltaMush_Evaluate ( ICENodeContext& in_ctxt );
SICALLBACK IRDeltaMush_EndEvaluate ( ICENodeContext& in_ctxt );

void IRDeltaMushInitializePoints();
void IRDeltaMushUpdatePoints();

//Declare Struct for data manipulation.
struct NeigborsEdgePointIndex
{
  LONG p1;
  LONG p2;
};

struct DMPoint_t
{
  bool                                switchResult;
  XSI::MATH::CVector3f                position;
  XSI::MATH::CVector3f                smoothPosition;
  XSI::MATH::CVector3f                smoothCache;
  XSI::MATH::CVector3f                offsetVector;
  XSI::MATH::CVector3f                normal;
  XSI::MATH::CVector3f                tangent;
  XSI::MATH::CVector3f                biNormal;
  XSI::MATH::CMatrix3f                referenceFrame;
  XSI::MATH::CMatrix4f                transform;
  std::vector<LONG>                   arrayNeigborsIndex;
  std::vector<XSI::MATH::CVector3f>   arrayNeigborsNormals;
  std::vector<NeigborsEdgePointIndex> arrayNeigborsEdgePointIndex;
};

struct DMData_t {
  std::vector<DMPoint_t>              arrayPoints;
  bool                                initialized;
};

//Declare help function.
void GetGeometryPointData ( CDoubleArray &in_GeoPointPosition, ULONG &in_PolyCount, 
  CLongArray	&in_PolyPointCount, CLongArray &in_PolyPointIndice,  std::vector<DMPoint_t> *io_ArrayPoint );
void UpdateGeometryPointData(CDoubleArray &in_GeoPointPosition, std::vector<DMPoint_t>& io_ArrayPoint);
void SmoothPointPositionData ( LONG in_SmoothIteration, std::vector<DMPoint_t> *io_ArrayPoint );
void ComputePointReferenceFrame ( std::vector<DMPoint_t> *io_ArrayPoint, ULONG &in_PointIndex );
void ComputePointMushInitData ( std::vector<DMPoint_t> *io_ArrayPoint );
void ComputePointMushData ( std::vector<DMPoint_t> *io_ArrayPoint, CDataArray2DMatrix4f &in_DmInitData );

#endif // __DELTA_MUSH__
