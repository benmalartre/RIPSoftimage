#include "attribute.h"
#include "utils.h"

X2UAttribute::X2UAttribute()
  : _dstAttribute()
  , _fromICE(false)
  , _hash(0)
{
}

X2UAttribute::X2UAttribute(
  const UsdAttribute& dstAttr,
  X2UDataType type,
  X2UDataPrecision precision,
  bool isArray,
  bool isPrimvar,
  X2UPrimvarInterpolation interpolation
)
  : _dstAttribute(dstAttr)
  , _srcDataType(type)
  , _srcDataPrecision(precision)
  , _isArray(isArray)
  , _isPrimvar(isPrimvar)
  , _primvarInterpolation(interpolation)
  , _hash(0)
  , _fromICE(false)
{
}

X2UAttribute::X2UAttribute(
  const UsdAttribute& dstAttr,
  const int srcAttrIndex,
  bool isArray)
  : _dstAttribute(dstAttr)
  , _srcAttributeIndex(srcAttrIndex)
  , _fromICE(true)
  , _isArray(isArray)
  , _hash(0)
{
}

X2UAttribute::~X2UAttribute()
{

}

void X2UAttribute::SetVariability(SdfVariability variability)
{
  if (variability == SdfVariability::SdfVariabilityUniform)
    _isConstant = true;
  else _isConstant = false;
}

void X2UAttribute::SetSourceType(X2UDataType type, X2UDataPrecision precision)
{
  _srcDataType = type;
  _srcDataPrecision = precision;
}

void X2UAttribute::WriteSample(const void* datas, uint32_t numElements, const UsdTimeCode& timeCode)
{
  uint64_t hash = ArchHash64((const char*)datas, numElements *  X2UGetDataSize(_srcDataType, _srcDataPrecision));
  if (hash != _hash)
  {
    // array value attribute
    if (_isArray)
    {
      switch (_srcDataType)
      {

      case X2U_DATA_BOOL:
      {
        VtArray<bool> vtArray(numElements);
        X2UCopyData<bool, bool>((bool*)datas, vtArray.data(), numElements);
        _dstAttribute.Set(VtValue(vtArray), timeCode);
        break;
      }

      case X2U_DATA_LONG:
      {
        TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
        if (typeNameToken == SdfValueTypeNames->Int64Array)
        {
          VtArray<int64_t> vtArray(numElements);
          X2UCopyData<LONG, int64_t>((LONG*)datas, vtArray.data(), numElements);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        else if (typeNameToken == SdfValueTypeNames->IntArray)
        {
          VtArray<int> vtArray(numElements);
          X2UCopyData<LONG, int>((LONG*)datas, vtArray.data(), numElements);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        break;
      }

      case X2U_DATA_FLOAT:
      {
        LOG("FLOAT DATA" + CString(GetOutput().GetName().GetText()));
        TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
        LOG(typeNameToken.GetText());
        if (typeNameToken == SdfValueTypeNames->DoubleArray)
        {
          VtArray<double> vtArray(numElements);
          X2UCopyData<float, double>((float*)datas, vtArray.data(), numElements);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        else if (typeNameToken == SdfValueTypeNames->FloatArray)
        {
          VtArray<float> vtArray(numElements);
          X2UCopyData<float, float>((float*)datas, vtArray.data(), numElements);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        break;
      }

      case X2U_DATA_DOUBLE:
      {
        TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
        if (typeNameToken == SdfValueTypeNames->DoubleArray)
        {
          VtArray<double> vtArray(numElements);
          X2UCopyData<double, double>((double*)datas, vtArray.data(), numElements);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        else if (typeNameToken == SdfValueTypeNames->FloatArray)
        {
          VtArray<float> vtArray(numElements);
          X2UCopyData<double, float>((double*)datas, vtArray.data(), numElements);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        break;
      }

      case X2U_DATA_VECTOR2:
      {
        TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
        if (typeNameToken == SdfValueTypeNames->Double2Array ||
          typeNameToken == SdfValueTypeNames->TexCoord2dArray)
        {
          VtArray<GfVec2d> vtArray(numElements);
          X2UCastTuppledData<GfVec2f, GfVec2d>((GfVec2f*)datas, vtArray.data(), numElements, 2, 2);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        else
        {
          VtArray<GfVec2f> vtArray(numElements);
          X2UCopyData<GfVec2f, GfVec2f>((GfVec2f*)datas, vtArray.data(), numElements);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        break;
      }

      case X2U_DATA_VECTOR3:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3dArray ||
            typeNameToken == SdfValueTypeNames->Point3dArray ||
            typeNameToken == SdfValueTypeNames->Normal3dArray ||
            typeNameToken == SdfValueTypeNames->Color3dArray ||
            typeNameToken == SdfValueTypeNames->Double3Array)
          {
            VtArray<GfVec3d> vtArray(numElements);
            X2UCastTuppledData<GfVec3f, GfVec3d>((GfVec3f*)datas, vtArray.data(), numElements, 3, 3);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else
          {
            VtArray<GfVec3f> vtArray(numElements);
            X2UCopyData<GfVec3f, GfVec3f>((GfVec3f*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        // double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3dArray ||
            typeNameToken == SdfValueTypeNames->Point3dArray ||
            typeNameToken == SdfValueTypeNames->Normal3dArray ||
            typeNameToken == SdfValueTypeNames->Color3dArray ||
            typeNameToken == SdfValueTypeNames->Double3Array)
          {
            VtArray<GfVec3d> vtArray(numElements);
            X2UCopyData<GfVec3d, GfVec3d>((GfVec3d*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else
          {
            VtArray<GfVec3f> vtArray(numElements);
            X2UCastTuppledData<GfVec3d, GfVec3f>((GfVec3d*)datas, vtArray.data(), numElements, 3, 3);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_VECTOR4:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Color4dArray ||
            typeNameToken == SdfValueTypeNames->Double4Array)
          {
            VtArray<GfVec4d> vtArray(numElements);
            X2UCastTuppledData<GfVec4f, GfVec4d>((GfVec4f*)datas, vtArray.data(), numElements, 4, 4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else
          {
            VtArray<GfVec4f> vtArray(numElements);
            X2UCopyData<GfVec4f, GfVec4f>((GfVec4f*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        //double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Color4dArray ||
            typeNameToken == SdfValueTypeNames->Double4Array)
          {
            VtArray<GfVec4d> vtArray(numElements);
            X2UCopyData<GfVec4d, GfVec4d>((GfVec4d*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else
          {
            VtArray<GfVec4f> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec4f>((GfVec4d*)datas, vtArray.data(), numElements, 4, 4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_COLOR4:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3dArray ||
            typeNameToken == SdfValueTypeNames->Color3dArray ||
            typeNameToken == SdfValueTypeNames->Double3Array)
          {
            VtArray<GfVec3d> vtArray(numElements);
            X2UCastTuppledData<GfVec4f, GfVec3d>((GfVec4f*)datas, vtArray.data(), numElements, 4, 3);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Double4Array ||
            typeNameToken == SdfValueTypeNames->Color4dArray)
          {
            VtArray<GfVec4d> vtArray(numElements);
            X2UCastTuppledData<GfVec4f, GfVec4d>((GfVec4f*)datas, vtArray.data(), numElements, 4, 4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Vector3fArray ||
            typeNameToken == SdfValueTypeNames->Color3fArray ||
            typeNameToken == SdfValueTypeNames->Float3Array)
          {
            VtArray<GfVec3f> vtArray(numElements);
            X2UCastTuppledData<GfVec4f, GfVec3f>((GfVec4f*)datas, vtArray.data(), numElements, 4, 3);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Color4fArray ||
            typeNameToken == SdfValueTypeNames->Float4Array)
          {
            VtArray<GfVec4f> vtArray(numElements);
            X2UCopyData<GfVec4f, GfVec4f>((GfVec4f*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        // double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3dArray ||
            typeNameToken == SdfValueTypeNames->Color3dArray ||
            typeNameToken == SdfValueTypeNames->Double3Array)
          {
            VtArray<GfVec3d> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec3d>((GfVec4d*)datas, vtArray.data(), numElements, 4, 3);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Double4Array ||
            typeNameToken == SdfValueTypeNames->Color4dArray)
          {
            VtArray<GfVec4d> vtArray(numElements);
            X2UCopyData<GfVec4d, GfVec4d>((GfVec4d*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Vector3fArray ||
            typeNameToken == SdfValueTypeNames->Color3fArray ||
            typeNameToken == SdfValueTypeNames->Float3Array)
          {
            VtArray<GfVec3f> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec3f>((GfVec4d*)datas, vtArray.data(), numElements, 4, 3);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Color4fArray ||
            typeNameToken == SdfValueTypeNames->Float4Array)
          {
            VtArray<GfVec4f> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec4f>((GfVec4d*)datas, vtArray.data(), numElements, 4, 4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_ROTATION:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Double4Array ||
            typeNameToken == SdfValueTypeNames->QuatdArray)
          {
            VtArray<GfQuatd> vtArray(numElements);
            X2UCastRotationData<XSI::MATH::CRotationf, GfQuatd>((XSI::MATH::CRotationf*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Float4Array ||
            typeNameToken == SdfValueTypeNames->QuatfArray)
          {
            VtArray<GfQuatf> vtArray(numElements);
            X2UCopyRotationData<XSI::MATH::CRotationf, GfQuatf>((XSI::MATH::CRotationf*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Half4Array ||
            typeNameToken == SdfValueTypeNames->QuathArray)
          {
            XSI::MATH::CRotationf rot;
            VtArray<GfQuath> vtArray(numElements);
            X2UCastRotationData<XSI::MATH::CRotationf, GfQuath>((XSI::MATH::CRotationf*)datas, (GfQuath*)vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        //double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Double4Array ||
            typeNameToken == SdfValueTypeNames->QuatdArray)
          {
            VtArray<GfVec4d> vtArray(numElements);
            X2UCopyData<GfVec4d, GfVec4d>((GfVec4d*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Float4Array ||
            typeNameToken == SdfValueTypeNames->QuatfArray)
          {
            VtArray<GfVec4f> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec4f>((GfVec4d*)datas, vtArray.data(), numElements, 4, 4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Half4Array ||
            typeNameToken == SdfValueTypeNames->QuathArray)
          {
            VtArray<GfVec4h> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec4h>((GfVec4d*)datas, vtArray.data(), numElements,4,4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_QUATERNION:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Double4Array ||
            typeNameToken == SdfValueTypeNames->QuatdArray)
          {
            VtArray<GfVec4d> vtArray(numElements);
            X2UCastTuppledData<GfVec4f, GfVec4d>((GfVec4f*)datas, vtArray.data(), numElements, 4, 4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Float4Array ||
            typeNameToken == SdfValueTypeNames->QuatfArray)
          {
            VtArray<GfVec4f> vtArray(numElements);
            X2UCopyData<GfVec4f, GfVec4f>((GfVec4f*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Half4Array ||
            typeNameToken == SdfValueTypeNames->QuathArray)
          {
            VtArray<GfVec4h> vtArray(numElements);
            X2UCastTuppledData<GfVec4f, GfVec4h>((GfVec4f*)datas, vtArray.data(), numElements, 4, 4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        //double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Double4Array ||
            typeNameToken == SdfValueTypeNames->QuatdArray)
          {
            VtArray<GfVec4d> vtArray(numElements);
            X2UCopyData<GfVec4d, GfVec4d>((GfVec4d*)datas, vtArray.data(), numElements);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Float4Array ||
            typeNameToken == SdfValueTypeNames->QuatfArray)
          {
            VtArray<GfVec4f> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec4f>((GfVec4d*)datas, vtArray.data(), numElements, 4, 4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Half4Array ||
            typeNameToken == SdfValueTypeNames->QuathArray)
          {
            VtArray<GfVec4h> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec4h>((GfVec4d*)datas, vtArray.data(), numElements, 4, 4);
            _dstAttribute.Set(VtValue(vtArray), timeCode);
          }
        }
        break;
      }
      }
    }

    // single value attribute
    else
    {
      switch (_srcDataType)
      {

      case X2U_DATA_BOOL:
      {
        _dstAttribute.Set(VtValue(*(bool*)datas), timeCode);
        break;
      }

      case X2U_DATA_LONG:
      {
        _dstAttribute.Set(VtValue(*(int*)datas), timeCode);
        break;
      }

      case X2U_DATA_FLOAT:
      {
        TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
        if (typeNameToken == SdfValueTypeNames->Float)
          _dstAttribute.Set(VtValue(*(float*)datas), timeCode);
        else if (typeNameToken == SdfValueTypeNames->Double)
          _dstAttribute.Set(VtValue((double)*(float*)datas), timeCode);
        break;
      }

      case X2U_DATA_DOUBLE:
      {
        TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
        if (typeNameToken == SdfValueTypeNames->Float)
          _dstAttribute.Set(VtValue((float)*(double*)datas), timeCode);
        else if (typeNameToken == SdfValueTypeNames->Double)
          _dstAttribute.Set(VtValue(*(double*)datas), timeCode);
        break;
      }

      case X2U_DATA_VECTOR2:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Float2)
          {
            _dstAttribute.Set(VtValue(*(GfVec2f*)datas), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Double2)
          {
            _dstAttribute.Set(VtValue((GfVec2d)*(GfVec2f*)datas), timeCode);
          }
        }
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Float2)
          {
            _dstAttribute.Set(VtValue((GfVec2f)*(GfVec2d*)datas), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Double2)
          {
            _dstAttribute.Set(VtValue(*(GfVec2d*)datas), timeCode);
          }
        }
       
        break;
      }

      case X2U_DATA_VECTOR3:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3f ||
            typeNameToken == SdfValueTypeNames->Point3f ||
            typeNameToken == SdfValueTypeNames->Normal3f ||
            typeNameToken == SdfValueTypeNames->Color3f ||
            typeNameToken == SdfValueTypeNames->Float3)
          {
            _dstAttribute.Set(VtValue(*(GfVec3f*)datas), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Vector3d ||
            typeNameToken == SdfValueTypeNames->Point3d ||
            typeNameToken == SdfValueTypeNames->Normal3d ||
            typeNameToken == SdfValueTypeNames->Color3d ||
            typeNameToken == SdfValueTypeNames->Double3)
          {
            _dstAttribute.Set(VtValue((GfVec3d)*(GfVec3f*)datas), timeCode);
          }
        }
        // double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3f ||
            typeNameToken == SdfValueTypeNames->Point3f ||
            typeNameToken == SdfValueTypeNames->Normal3f ||
            typeNameToken == SdfValueTypeNames->Color3f ||
            typeNameToken == SdfValueTypeNames->Float3)
          {
            _dstAttribute.Set(VtValue((GfVec3f)*(GfVec3d*)datas), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Vector3d ||
            typeNameToken == SdfValueTypeNames->Point3d ||
            typeNameToken == SdfValueTypeNames->Normal3d ||
            typeNameToken == SdfValueTypeNames->Color3d ||
            typeNameToken == SdfValueTypeNames->Double3)
          {
            _dstAttribute.Set(VtValue(*(GfVec3d*)datas), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_VECTOR4:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Float4 ||
            typeNameToken == SdfValueTypeNames->Quatf ||
            typeNameToken == SdfValueTypeNames->Color4f)
          {
            _dstAttribute.Set(VtValue(*(GfVec4f*)datas), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Double4 ||
            typeNameToken == SdfValueTypeNames->Quatd ||
            typeNameToken == SdfValueTypeNames->Color4d)
          {
            _dstAttribute.Set(VtValue((GfVec4d)*(GfVec4f*)datas), timeCode);
          }
        }
        //double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Float4 ||
            typeNameToken == SdfValueTypeNames->Quatf ||
            typeNameToken == SdfValueTypeNames->Color4f)
          {
            _dstAttribute.Set(VtValue((GfVec4f)*(GfVec4d*)datas), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Double4 ||
            typeNameToken == SdfValueTypeNames->Quatd ||
            typeNameToken == SdfValueTypeNames->Color4d)
          {
            _dstAttribute.Set(VtValue(*(GfVec4d*)datas), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_COLOR4:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3d ||
            typeNameToken == SdfValueTypeNames->Color3d ||
            typeNameToken == SdfValueTypeNames->Double3)
          {
            GfVec3d color;
            X2UCastTuppledData<GfVec4f, GfVec3d>((GfVec4f*)datas, &color, 1, 4, 3);
            _dstAttribute.Set(VtValue(color), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Double4 ||
            typeNameToken == SdfValueTypeNames->Color4d)
          {
            GfVec4d color;
            X2UCastTuppledData<GfVec4f, GfVec4d>((GfVec4f*)datas, &color, 1, 4, 4);
            _dstAttribute.Set(VtValue(color), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Vector3f ||
            typeNameToken == SdfValueTypeNames->Color3f ||
            typeNameToken == SdfValueTypeNames->Float3)
          {
            GfVec3f color = *((GfVec3f*)datas);
            _dstAttribute.Set(VtValue(color), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Color4f ||
            typeNameToken == SdfValueTypeNames->Float4)
          {
            GfVec4f color = *((GfVec4f*)datas);
            _dstAttribute.Set(VtValue(color), timeCode);
          }
        }
        // double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3d ||
            typeNameToken == SdfValueTypeNames->Color3d ||
            typeNameToken == SdfValueTypeNames->Double3)
          {
            GfVec3d color = *((GfVec3d*)datas);
            _dstAttribute.Set(VtValue(color), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Double4 ||
            typeNameToken == SdfValueTypeNames->Color4d)
          {
            GfVec4d color = *((GfVec4d*)datas);
            _dstAttribute.Set(VtValue(color), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Vector3f ||
            typeNameToken == SdfValueTypeNames->Color3f ||
            typeNameToken == SdfValueTypeNames->Float3)
          {
            GfVec3f color;
            X2UCastTuppledData<GfVec4d, GfVec3f>((GfVec4d*)datas, &color, 1, 4, 3);
            _dstAttribute.Set(VtValue(), timeCode);
          }
          else if (typeNameToken == SdfValueTypeNames->Color4f ||
            typeNameToken == SdfValueTypeNames->Float4)
          {
            GfVec4f color;
            X2UCastTuppledData<GfVec4d, GfVec4f>((GfVec4d*)datas, &color, 1, 4, 4);
            _dstAttribute.Set(VtValue(), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_ROTATION:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Double4)
          {
            /*
            VtArray<GfVec4d> vtArray(numElements);
            X2UCastTuppledData<GfVec4f, GfVec4d>((GfVec4f*)datas, vtArray.data(), numElements, 4, 4);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
          else
          {
            /*
            VtArray<GfVec4f> vtArray(numElements);
            X2UCopyData<GfVec4f, GfVec4f>((GfVec4f*)datas, vtArray.data(), numElements);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
        }
        //double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Double4)
          {
            /*
            VtArray<GfVec4d> vtArray(numElements);
            X2UCopyData<GfVec4d, GfVec4d>((GfVec4d*)datas, vtArray.data(), numElements);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
          else
          {
            /*
            VtArray<GfVec4f> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec4f>((GfVec4d*)datas, vtArray.data(), numElements, 4, 4);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_QUATERNION:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Double4 ||
            typeNameToken == SdfValueTypeNames->Quatd)
          {
            /*
            VtArray<GfVec4d> vtArray(numElements);
            X2UCastTuppledData<GfVec4f, GfVec4d>((GfVec4f*)datas, vtArray.data(), numElements, 4, 4);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
          else
          {
            /*
            VtArray<GfVec4f> vtArray(numElements);
            X2UCopyData<GfVec4f, GfVec4f>((GfVec4f*)datas, vtArray.data(), numElements);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
        }
        //double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Double4 ||
            typeNameToken == SdfValueTypeNames->Quatd)
          {
            /*
            VtArray<GfVec4d> vtArray(numElements);
            X2UCopyData<GfVec4d, GfVec4d>((GfVec4d*)datas, vtArray.data(), numElements);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
          else
          {
            /*
            VtArray<GfVec4f> vtArray(numElements);
            X2UCastTuppledData<GfVec4d, GfVec4f>((GfVec4d*)datas, vtArray.data(), numElements, 4, 4);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_MATRIX3:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Matrix3d)
          {
            GfMatrix3d dstMatrix;
            X2UConvertMatrix3FloatToDouble(*((MATH::CMatrix3f*)datas), dstMatrix);
            _dstAttribute.Set(VtValue(dstMatrix), timeCode);
          }
        }
        //double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Matrix3d)
          {
            GfMatrix3d dstMatrix;
            X2UConvertMatrix3DoubleToDouble(*((MATH::CMatrix3*)datas), dstMatrix);
            _dstAttribute.Set(VtValue(dstMatrix), timeCode);
          }
        }
        break;
      }

      case X2U_DATA_MATRIX4:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Matrix4d)
          {
            GfMatrix4d dstMatrix;
            X2UConvertMatrix4FloatToDouble(*((MATH::CMatrix4f*)datas), dstMatrix);
            _dstAttribute.Set(VtValue(dstMatrix), timeCode);
          }
        }
        //double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Matrix4d)
          {
            GfMatrix4d dstMatrix;
            X2UConvertMatrix4DoubleToDouble(*((MATH::CMatrix4*)datas), dstMatrix);
            _dstAttribute.Set(VtValue(dstMatrix), timeCode);
          }
        }
        break;
      }
      case X2U_DATA_TOKEN:
      {
        _dstAttribute.Set(*(TfToken*)datas, timeCode);
      }
      }
    }
  }
  WriteInterpolation();
  _hash = hash;
}

void X2UAttribute::WriteSample(const TfToken& token, const UsdTimeCode& timeCode)
{
  uint64_t hash = token.Hash();

  if (hash != _hash)
  {
    _dstAttribute.Set(token, timeCode);
    _hash = hash;
  }
}

template<typename T>
void _WriteSample(X2UAttribute* dstAttribute,
  const ICEAttribute& srcAttribute, const UsdTimeCode& timeCode)
{
  LOG("WRITE SAMPLE " + srcAttribute.GetName());
  CICEAttributeDataArray<T> datas;
  srcAttribute.GetDataArray(datas);
  LOG("DATA SIZE : " + CString(datas.GetCount()));
  dstAttribute->WriteSample((void*)&datas[0], datas.GetCount(), timeCode);
}

template<>
void _WriteSample<bool>(X2UAttribute* dstAttribute,
  const ICEAttribute& srcAttribute, const UsdTimeCode& timeCode)
{
  CICEAttributeDataArrayBool datas;
  srcAttribute.GetDataArray(datas);
  dstAttribute->WriteSample((void*)&datas, datas.GetCount(), timeCode);
}

template<typename T>
void _WriteArraySample(X2UAttribute* dstAttribute, 
  const ICEAttribute& srcAttribute, const UsdTimeCode& timeCode)
{
  CICEAttributeDataArray2D<T> datas;
  CICEAttributeDataArray<T> data;
  srcAttribute.GetDataArray2D(datas);
  if (datas.GetCount() == 1) {
    datas.GetSubArray(0, data);
    dstAttribute->WriteSample((void*)&data[0], data.GetCount(), timeCode);
  } else if (datas.GetCount() > 1) {
    std::vector<T> accum;
    size_t offset = 0;
    for (size_t x = 0; x < datas.GetCount(); ++x) {
      datas.GetSubArray(x, data);
      accum.resize(accum.size() + data.GetCount());
      for (size_t y = 0; y < data.GetCount(); ++y) {
        accum[offset++] = data[y];
      }
    }
    dstAttribute->WriteSample((void*)&accum[0], accum.size(), timeCode);
  }
}

template<>
void _WriteArraySample<bool>(X2UAttribute* dstAttribute,
  const ICEAttribute& srcAttribute, const UsdTimeCode& timeCode)
{
  CICEAttributeDataArray2D<bool> datas;
  CICEAttributeDataArray<bool> data;
  srcAttribute.GetDataArray2D(datas);

  if (datas.GetCount() == 1) {
    datas.GetSubArray(0, data);
    dstAttribute->WriteSample((void*)&data, data.GetCount(), timeCode);
  }
  else if (datas.GetCount() > 1) {
    std::vector<bool> accum;
    size_t offset = 0;
    for (size_t x = 0; x < datas.GetCount(); ++x) {
      datas.GetSubArray(x, data);
      accum.resize(accum.size() + data.GetCount());
      for (size_t y = 0; y < data.GetCount(); ++y) {
        accum[offset++] = data[y];
      }
    }
    dstAttribute->WriteSample((void*)&accum, accum.size(), timeCode);
  }
}

void X2UAttribute::WriteSample(const Geometry& geom, const UsdTimeCode& timeCode)
{
  
  CRefArray attributes = geom.GetICEAttributes();
  ICEAttribute srcAttribute = attributes[_srcAttributeIndex];
  siICENodeStructureType structType = srcAttribute.GetStructureType();

  // 1D
  if (structType == siICENodeStructureSingle) {
    if (srcAttribute.GetElementCount() == 0)return;

    _srcDataType = X2UDataTypeFromICEType(srcAttribute.GetDataType());
    _srcDataPrecision = X2U_PRECISION_SINGLE;

    switch (_srcDataType)
    {
    case X2U_DATA_BOOL:
      _WriteSample<bool>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_LONG:
      _WriteSample<LONG>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_FLOAT:
      _WriteSample<float>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_VECTOR2:
      _WriteSample<MATH::CVector2f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_VECTOR3:
      _WriteSample<MATH::CVector3f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_VECTOR4:
      _WriteSample<MATH::CVector4f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_COLOR4:
      _WriteSample<MATH::CColor4f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_ROTATION:
      _WriteSample<MATH::CRotationf>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_QUATERNION:
      _WriteSample<MATH::CQuaternionf>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_MATRIX3:
      _WriteSample<MATH::CMatrix3f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_MATRIX4:
      _WriteSample<MATH::CMatrix4f>(this, srcAttribute, timeCode);
      break;
    }
  }
  // 2D
  else if (structType == siICENodeStructureArray) {
    _srcDataType = X2UDataTypeFromICEType(srcAttribute.GetDataType());
    _srcDataPrecision = X2U_PRECISION_SINGLE;

    switch (_srcDataType)
    {
    case X2U_DATA_BOOL:
      _WriteArraySample<bool>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_LONG:
      _WriteArraySample<LONG>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_FLOAT:
      _WriteArraySample<float>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_VECTOR2:
      _WriteArraySample<MATH::CVector2f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_VECTOR3:
      _WriteArraySample<MATH::CVector3f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_VECTOR4:
      _WriteArraySample<MATH::CVector4f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_COLOR4:
      _WriteArraySample<MATH::CColor4f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_ROTATION:
      _WriteArraySample<MATH::CRotationf>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_QUATERNION:
      _WriteArraySample<MATH::CQuaternionf>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_MATRIX3:
      _WriteArraySample<MATH::CMatrix3f>(this, srcAttribute, timeCode);
      break;

    case X2U_DATA_MATRIX4:
      _WriteArraySample<MATH::CMatrix4f>(this, srcAttribute, timeCode);
      break;
    }
  }
}

void X2UAttribute::WriteInterpolation()
{
  if (_isPrimvar)
  {
    UsdGeomPrimvar dstPrimvar(_dstAttribute);
    switch (_primvarInterpolation)
    {
    case X2U_INTERPOLATION_UNIFORM:
      dstPrimvar.SetInterpolation(UsdGeomTokens->uniform);
      break;
    case X2U_INTERPOLATION_VARYING:
      dstPrimvar.SetInterpolation(UsdGeomTokens->varying);
      break;
    case X2U_INTERPOLATION_VERTEX:
      dstPrimvar.SetInterpolation(UsdGeomTokens->vertex);
      break;
    case X2U_INTERPOLATION_FACEVARYING:
      dstPrimvar.SetInterpolation(UsdGeomTokens->faceVarying);
      break;
    default:
      dstPrimvar.SetInterpolation(UsdGeomTokens->constant);
      break;
    }
  }
}