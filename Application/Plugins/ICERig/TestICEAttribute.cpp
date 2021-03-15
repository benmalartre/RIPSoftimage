#include <xsi_x3dobject.h>
#include <xsi_primitive.h>
#include <xsi_geometry.h>
#include <xsi_iceattribute.h>

using namespace XSI;

void Test(X3DObject& obj)
{
  Geometry geom = obj.GetActivePrimitive().GetGeometry();
  //ICEAttribute attr = geom.GetICEAttributes()
}