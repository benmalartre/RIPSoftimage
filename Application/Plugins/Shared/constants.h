
#ifndef FLAG_CONSTANTS
#define FLAG_CONSTANTS

#include "xsi_common.h"

namespace SOFTIMAGE {

static const float PI = (float)MATH::PI;			// PI
static const float PI_INV = 1.0f / MATH::PI;			// 1/PI
static const float INV_180 = 1.0f / 180.0f;			// 1/180
static const float DEGTORAD = PI / 180.0f;			// degrees to radians conversion
static const float RADTODEG = 180.0f / PI;			// radians to degrees conversion

static const float VERYSMALL = 0.00001f;
static const float SQR2 = sqrtf(2.0f);
static const float ONETHIRD = 1.0f / 3.0f;

} // namespace SOFTIMAGE

#endif // FLAG_CONSTANTS
