#include "functions.h"

namespace SOFTIMAGE
{
unsigned long hash(long val)
{
	unsigned char *p = (unsigned char *)&val;
	unsigned long lHash = 0;

	for ( int i = 0; i < sizeof(val); i++ )
	{
		lHash = lHash * ( UCHAR_MAX + 2U ) + p[i];
	}

	return lHash;
}

void randomize_timer()
{
	srand( hash( (long)time( NULL ) ) );
}

void randomize_seed(long val)
{
	srand( hash( val ) );
}

float random()
{
	return (float)rand() / (float)RAND_MAX;
}

float random(float min, float max)
{
	float range = max - min;
	return min + (float)rand() / (float)RAND_MAX * range;
}

long round(double a)
{
	return ( a > 0.0f - VERYSMALL ) ? long(a + 0.5f) : long(a - 0.5f);
}

float round(const float& a, const int& iDigits)
{
    return (float)b_round((double)a, iDigits);
}

double round(const double& a, const int& iDigits)
{
	double dDenom = pow(10.0, (double) (iDigits));
    double dMul = (double)b_round(a * dDenom);
    return dMul / dDenom;
}

bool eqD(double v1, double v2)
{
	bool eq_out = false;

	if ( ( v1 < ( v2 + VERYSMALL ) ) && ( v1 > ( v2 - VERYSMALL ) ) )
		eq_out = true;

	return eq_out;
}

bool eqF(float v1, float v2)
{
	bool eq_out = false;

	if ( ( v1 < ( v2 + VERYSMALL ) ) && ( v1 > ( v2 - VERYSMALL ) ) )
		eq_out = true;

	return eq_out;
}

bool eq(CVector3& av1, CVector3& av2)
{
	bool eq_out = false;

	if ( ( av1.GetX() < ( av2.GetX() + VERYSMALL ) ) &&
		 ( av1.GetX() > ( av2.GetX() - VERYSMALL ) ) &&
		 ( av1.GetY() < ( av2.GetY() + VERYSMALL ) ) &&
		 ( av1.GetY() > ( av2.GetY() - VERYSMALL ) ) &&
		 ( av1.GetZ() < ( av2.GetZ() + VERYSMALL ) ) &&
		 ( av1.GetZ() > ( av2.GetZ() - VERYSMALL ) ) )
		eq_out = true;

	return eq_out;
}

float acos(float val)
{
	if (val<=-1.0f) val = -1.0f+b_fVery_small;
	if (val>=1.0f) val = 1.0f-b_fVery_small;
//	return atanf(-val / sqrtf(-val * val + 1.0f)) + 2.0f * atanf(1.0f);
	return acosf(val);
}

float asin(float val)
{
	if (val<=-1.0f) val = -1.0f+b_fVery_small;
	if (val>=1.0f) val = 1.0f-b_fVery_small;
	return asinf(val);
}


void MapArrayToWorldSpace(CVector3Array& in_array, const CTransformation in_transform)
{
   for (long i = 0; i < in_array.GetCount(); i++)
   {
      in_array[i] = MapObjectPositionToWorldSpace(in_transform, in_array[i]);
   }
}

void MapArrayToObjectSpace(CVector3Array& in_array, const CTransformation in_transform)
{
   for (long i = 0; i < in_array.GetCount(); i++)
   {
      in_array[i] = MapWorldPositionToObjectSpace(in_transform, in_array[i]);
   }
}

void XFormArray(CVector3Array& pos, CTransformation& t)
{
	for(long p=0;p<pos.GetCount();p++)
	{
		pos[p].MulByTransformationInPlace(t);
	}
}

CMatrix3 NormalToRotation(CVector3& n, CVector3& up)
{
	CMatrix3 oMatrix;
	CVector3 v1, v2;
	CRotation r;

	n.NormalizeInPlace();
	// up.NormalizeInPlace();

	v1.Cross(n, up);
	v2.Cross(v1, n);

	v1.NormalizeInPlace();
	v2.NormalizeInPlace();

	oMatrix.Set(v1.GetX(), v1.GetY(), v1.GetZ(), n.GetX(), n.GetY(), n.GetZ(), v2.GetX(), v2.GetY(), v2.GetZ());

	return oMatrix;
}

CVector3 GetBarycenter(Facet& in_face)
{
	CPointRefArray points = in_face.GetPoints();
	CVector3Array pos = points.GetPositionArray();
	long nbp = in_face.GetNbPoints();
	CVector3 barycenter;

	for(long a=0;a<nbp;a++){barycenter.AddInPlace(pos[a]);}
	barycenter.ScaleInPlace(1/nbp);

	return barycenter;
}

void glDrawPoint(CVector3& pos, float r, float g, float b,float size)
{
	// some objects for drawing vectors
	Application gApp;
	CValueArray args(5);
	args[0] = pos;
	args[1] = r;
	args[2] = g;
	args[3] = b;
	args[4] = size;
	CValue retval;
	CStatus st = gApp.ExecuteCommand( L"eGLDrawPoint", args, retval );
}

void glDrawVector(CVector3& start, CVector3& end, float r, float g, float b)
{
	// some objects for drawing vectors
	Application gApp;
	CValueArray args(5);
	args[0] = start;
	args[1] = end;
	args[2] = r;
	args[3] = g;
	args[4] = b;
	CValue retval;
	CStatus st = gApp.ExecuteCommand( L"eGLDrawVector", args, retval );
}

void glDrawLine(CVector3& start, CVector3& end, float r, float g, float b)
{
	Application gApp;
	CValueArray args(5);
	args[0] = start;
	args[1] = end;
	args[2] = r;
	args[3] = g;
	args[4] = b;
	CValue retval;
	CStatus st = gApp.ExecuteCommand( L"eGLDrawLine", args, retval );
}

void glDrawArrow(CVector3& start, CVector3& end, float r, float g, float b, float height, float width, long division)
{
	Application gApp;
	CValueArray args(8);
	args[0] = start;
	args[1] = end;
	args[2] = r;
	args[3] = g;
	args[4] = b;
	args[5] = height;
	args[6] = width;
	args[7] = division;
	CValue retval;
	CStatus st = gApp.ExecuteCommand( L"eGLDrawArrow", args, retval );
}

void glFlushAll()
{
	// some objects for drawing vectors
	Application gApp;
	CValueArray args(0);
	CValue retval;
	CStatus st = gApp.ExecuteCommand( L"eGLFlushAll", args, retval );
}

float LinearInterpolate(float y1,float y2,double mu)
{
	 return((float)(y1*(1-mu)+y2*mu));
};

float CubicInterpolate(float y0,float y1,float y2,float y3,double mu)
{
   double a0,a1,a2,a3,mu2;

   mu2 = (mu*mu);
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return((float)(a0*mu*mu2+a1*mu2+a2*mu+a3));
};

float HermiteInterpolate(float y0,float y1,float y2,float y3,double mu,double tension,double bias)
{
   double m0,m1,mu2,mu3;
   double a0,a1,a2,a3;

	mu2 = mu * mu;
	mu3 = mu2 * mu;
   m0  = (y1-y0)*(1+bias)*(1-tension)/2;
   m0 += (y2-y1)*(1-bias)*(1-tension)/2;
   m1  = (y2-y1)*(1+bias)*(1-tension)/2;
   m1 += (y3-y2)*(1-bias)*(1-tension)/2;
   a0 =  2*mu3 - 3*mu2 + 1;
   a1 =    mu3 - 2*mu2 + mu;
   a2 =    mu3 -   mu2;
   a3 = -2*mu3 + 3*mu2;

   return((float)(a0*y1+a1*m0+a2*m1+a3*y2));
};

double LinearInterpolate(double y1,double y2,double mu)
{
	 return((double)(y1*(1-mu)+y2*mu));
};

double CubicInterpolate(double y0,double y1,double y2,double y3,double mu)
{
   double a0,a1,a2,a3,mu2;

   mu2 = (mu*mu);
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}

double HermiteInterpolate(double y0,double y1,double y2,double y3,double mu,double tension,double bias)
{
   double m0,m1,mu2,mu3;
   double a0,a1,a2,a3;

	mu2 = mu * mu;
	mu3 = mu2 * mu;
   m0  = (y1-y0)*(1+bias)*(1-tension)/2;
   m0 += (y2-y1)*(1-bias)*(1-tension)/2;
   m1  = (y2-y1)*(1+bias)*(1-tension)/2;
   m1 += (y3-y2)*(1-bias)*(1-tension)/2;
   a0 =  2*mu3 - 3*mu2 + 1;
   a1 =    mu3 - 2*mu2 + mu;
   a2 =    mu3 -   mu2;
   a3 = -2*mu3 + 3*mu2;

   return(a0*y1+a1*m0+a2*m1+a3*y2);
}

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
)
{
   CValue oResult;
   CValueArray oArgs;

   oArgs.Add( strElementType );
   oArgs.Add( strLeftMessage );
   oArgs.Add( strMiddleMessage );
   oArgs.Add( CValue() );
   oArgs.Add( CValue() );
   oArgs.Add( (LONG) nSelRegionMode );
   oArgs.Add( CValue() );

   CComAPIHandler comApp;
   comApp.CreateInstance( L"XSI.Application" );
   comApp = comApp.GetProperty( L"Application" );

   if ( comApp.Call( L"PickElement", oResult, oArgs ) == CStatus::OK )
   {
      CComAPIHandler comVTCollection( oResult );
      oArgs.Resize(1);


      oArgs[0] = L"ButtonPressed";
      comVTCollection.Invoke( L"Value", CComAPIHandler::PropertyGet, oResult, oArgs );
      outButtonPressed = oResult;

      oArgs[0] = L"ModifierPressed";
      comVTCollection.Invoke( L"Value", CComAPIHandler::PropertyGet, oResult, oArgs );
      outModifierPressed = oResult;

      if ( outButtonPressed > 0 )
      {
         oArgs[0] = L"PickedElement";
         comVTCollection.Invoke( L"Value", CComAPIHandler::PropertyGet, oResult, oArgs );

         CComAPIHandler comElement( oResult );
         CValue oSubComponent( comElement.GetProperty( L"SubComponent" ) );

         //-------------------------------------------------
         // X3DObject
         //-------------------------------------------------
         if( oSubComponent.m_t == CValue::siEmpty )
         {
            outPickedElement = outParentObject = oResult;
         }
         //-------------------------------------------------
         // Subcomponent
         //-------------------------------------------------
         else
         {
            CComAPIHandler comSubComponent( oSubComponent );
            CValueArray outElementArray = comSubComponent.GetProperty( L"ElementArray" );
            outParentObject = comSubComponent.GetProperty( L"Parent3DObject" );
            // It returns an index to an object not an object itself
            outPickedElement = outElementArray[ 0 ];
         }
      }
      return true;
   }
   return false;
}

// CreateFolder
bool CreateFolder(const char* path)
{
	#ifdef WIN32
		if(_mkdir(path) == 0)return true;
	#else
		if(mkdir(path ,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)return true;
	#endif
	return false;
}

/*
#define SMALL_NUM  0.00000001 // anything that avoids division overflow

// intersect_RayTriangle(): intersect a ray with a 3D triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 = disjoint (no intersect)
//             1 = intersect in unique point I1
//             2 = are in the same plane
int bIntersectRayTriangle( Ray R, Triangle T, Point* I )
{
    CVector3    u, v, n;             // triangle vectors
    CVector3    dir, w0, w;          // ray vectors
    float		r, a, b;             // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = T.V1 - T.V0;
    v = T.V2 - T.V0;
    n = u * v;             // cross product
    if (n == (Vector)0)            // triangle is degenerate
        return -1;                 // do not deal with this case

    dir = R.P1 - R.P0;             // ray direction vector
    w0 = R.P0 - T.V0;
    a = -dot(n,w0);
    b = dot(n,dir);
    if (fabs(b) < SMALL_NUM) {     // ray is parallel to triangle plane
        if (a == 0)                // ray lies in triangle plane
            return 2;
        else return 0;             // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                   // ray goes away from triangle
        return 0;                  // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect

    *I = R.P0 + r * dir;           // intersect point of ray and plane

    // is I inside T?
    float    uu, uv, vv, wu, wv, D;
    uu = dot(u,u);
    uv = dot(u,v);
    vv = dot(v,v);
    w = *I - T.V0;
    wu = dot(w,u);
    wv = dot(w,v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside T
        return 0;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;

    return 1;                      // I is in T
}
*/
