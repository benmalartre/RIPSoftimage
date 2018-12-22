
#ifndef FLAG_FUNCTIONS
#define FLAG_FUNCTIONS

#include "xsi_common.h"
#include "constants.h"
#include "xsi_3d.h"

namespace SOFTIMAGE {

// defines
#define b_sq(a) ((a)*(a))
#define b_lerp(a,b,x) ((a)+((b)-(a))*(x))

#define MAX(a,b)	((a)<(b) ? (b)  : (a))
#define MIN(a,b)	((a)>(b) ? (b)  : (a))
#define ABS(a)		((a)<(0) ? (-a) : (a))

// misc
unsigned long hash(long val);
void randomize_timer();
void randomize_seed(long val);
float random();
float random(float min, float max);
long round(double a);							// a simple rounding function
float round(const float& a, const int& iDigits);
double round(const double& a, const int& iDigits);
bool eqD(double v1, double v2);
bool eqF(float v1, float v2);
bool eq(CVector3& av1, CVector3& av2);

// reverse angles
float acos(float val);
float asin(float val);

// array transforms
void MapArrayToWorldSpace(CVector3Array& in_array, const CTransformation in_transform);
void MapArrayToObjectSpace(CVector3Array& in_array, const CTransformation in_transform);
void XFormArray(CVector3Array& pos, CTransformation& t);

// quaternion
CQuaternion slerp2(CQuaternion inQuatA, CQuaternion inQuatB, float inRoll);
double getDot(CQuaternion inQuatA,CQuaternion inQuatB);
void F3DLimit(double io_dVal,float in_dMin,float in_dMax);

// barycenter
CVector3 b_GetBarycenter(Facet& in_face);

// openGL drawer
void glDrawPoint(CVector3& pos, float r, float g, float b,float size);
void glDrawVector(CVector3& start,CVector3& end, float r, float g, float b);
void glDrawLine(CVector3& start,CVector3& end, float r, float g, float b);
void glDrawArrow(CVector3& start,CVector3& end, float r, float g, float b,float height, float width, long division);
void glFlushAll();

// normal to rotation
CMatrix3 NormalToRotation(CVector3& n, CVector3& up);

// Interpolation
double LinearInterpolate(double y1,double y2,double mu);
double CubicInterpolate(double y0,double y1,double y2,double y3,double mu);
double HermiteInterpolate(double y0,double y1,double y2,double y3,double mu,double tension,double bias);

float LinearInterpolate(float y1,float y2,double mu);
float CubicInterpolate(float y0,float y1,float y2,float y3,double mu);
float HermiteInterpolate(float y0,float y1,float y2,float y3,double mu,double tension,double bias);

//PickElement
bool PickElement
(
  const CString& strElementType,
  const CString& strLeftMessage,
  const CString& strMiddleMessage,
  const int nSelRegionMode,
  CValue& outPickedElement,
  long& outButtonPressed,
  long& outModifierPressed,
  SIObject& outParentObject        // If we'd like to know the parent object
);

//CreateFolder
bool CreateFolder(const char* path);

} // namespace SOFTIMAGE


#endif 
