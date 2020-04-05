#include "attribute.h"
#include "utils.h"

X2UExportAttribute::X2UExportAttribute()
  : _dstAttribute()
  , _srcAttribute()
  , _fromICE(false)
  , _hash(0)
{
}

X2UExportAttribute::X2UExportAttribute(
  UsdAttribute& dst,
  X2UDataType type,
  X2UDataPrecision precision,
  bool isArray,
  bool isPrimvar,
  X2UPrimvarInterpolation interpolation
)
  : _dstAttribute(dst)
  , _srcDataType(type)
  , _srcDataPrecision(precision)
  , _isArray(isArray)
  , _isPrimvar(isPrimvar)
  , _primvarInterpolation(interpolation)
  , _hash(0)
  , _fromICE(false)
{
  _GetDestinationAttributeSpecs();
}

X2UExportAttribute::X2UExportAttribute(
  UsdAttribute& dst, 
  ICEAttribute& src)
  : _dstAttribute(dst)
  , _srcAttribute(src)
  , _fromICE(true)
  , _hash(0)
{
  _GetSourceAttributeSpecs();
  _GetDestinationAttributeSpecs();
}

X2UExportAttribute::~X2UExportAttribute()
{

}

void X2UExportAttribute::_GetDestinationAttributeSpecs()
{

}

void X2UExportAttribute::_GetSourceAttributeSpecs()
{
  if(_srcAttribute.IsDefined())
  {
    _srcDataType = X2UDataTypeFromICEType(_srcAttribute.GetDataType());
    _srcDataPrecision = X2U_PRECISION_SINGLE;
  }
}

void X2UExportAttribute::SetVariability(SdfVariability variability)
{
  if (variability == SdfVariability::SdfVariabilityUniform)
    _isConstant = true;
  else _isConstant = false;
}

void X2UExportAttribute::SetSourceType(X2UDataType type, X2UDataPrecision precision)
{
  _srcDataType = type;
  _srcDataPrecision = precision;
}

void X2UExportAttribute::SetSourceAttribute(const ICEAttribute& attribute)
{
  _srcAttribute = attribute;
  _GetSourceAttributeSpecs();
}


void X2UExportAttribute::WriteSample(const void* datas, uint32_t numElements, const UsdTimeCode& timeCode)
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

      case X2U_DATA_INT:
      {
        TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
        if (typeNameToken == SdfValueTypeNames->Int64Array)
        {
          VtArray<int64_t> vtArray(numElements);
          X2UCopyData<int, int64_t>((int*)datas, vtArray.data(), numElements);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        else if (typeNameToken == SdfValueTypeNames->IntArray)
        {
          VtArray<int> vtArray(numElements);
          X2UCopyData<int, int>((int*)datas, vtArray.data(), numElements);
          _dstAttribute.Set(VtValue(vtArray), timeCode);
        }
        break;
      }

      case X2U_DATA_FLOAT:
      {
        TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
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
          if (typeNameToken == SdfValueTypeNames->Double4Array)
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
          if (typeNameToken == SdfValueTypeNames->Double4Array)
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
          if (typeNameToken == SdfValueTypeNames->Double4Array)
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
      }
    }

    // single value attribute
    else
    {
      switch (_srcDataType)
      {

      case X2U_DATA_BOOL:
      {
        /*
        VtArray<bool> vtArray(numElements);
        X2UCopyData<bool, bool>((bool*)datas, vtArray.data(), numElements);
        */
        _dstAttribute.Set(VtValue(), timeCode);
        break;
      }

      case X2U_DATA_INT:
      {
        /*
        VtArray<int> vtArray(numElements);
        X2UCopyData<int, int>((int*)datas, vtArray.data(), numElements);
        */
        _dstAttribute.Set(VtValue(), timeCode);
        break;
      }

      case X2U_DATA_FLOAT:
      {
        /*
        VtArray<float> vtArray(numElements);
        X2UCopyData<float, float>((float*)datas, vtArray.data(), numElements);
        */
        _dstAttribute.Set(VtValue(), timeCode);
        break;
      }

      case X2U_DATA_DOUBLE:
      {
        /*
        VtArray<double> vtArray(numElements);
        X2UCopyData<double, double>((double*)datas, vtArray.data(), numElements);
        */
        _dstAttribute.Set(VtValue(), timeCode);
        break;
      }

      case X2U_DATA_VECTOR2:
      {
        TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
        if (typeNameToken == SdfValueTypeNames->Double2)
        {
          /*
          VtArray<GfVec2d> vtArray(numElements);
          X2UCastTuppledData<GfVec2f, GfVec2d>((GfVec2f*)datas, vtArray.data(), numElements, 2, 2);
          */
          _dstAttribute.Set(VtValue(), timeCode);
        }
        else
        {
          /*
          VtArray<GfVec2f> vtArray(numElements);
          X2UCopyData<GfVec2f, GfVec2f>((GfVec2f*)datas, vtArray.data(), numElements);
          */
          _dstAttribute.Set(VtValue(), timeCode);
        }
        break;
      }

      case X2U_DATA_VECTOR3:
      {
        // single precision
        if (_srcDataPrecision == X2U_PRECISION_SINGLE)
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3d ||
            typeNameToken == SdfValueTypeNames->Point3d ||
            typeNameToken == SdfValueTypeNames->Normal3d ||
            typeNameToken == SdfValueTypeNames->Color3d ||
            typeNameToken == SdfValueTypeNames->Double3)
          {
            /*
            VtArray<GfVec3d> vtArray(numElements);
            X2UCastTuppledData<GfVec3f, GfVec3d>((GfVec3f*)datas, vtArray.data(), numElements, 3, 3);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
          else
          {
            /*
            VtArray<GfVec3f> vtArray(numElements);
            X2UCopyData<GfVec3f, GfVec3f>((GfVec3f*)datas, vtArray.data(), numElements);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
        }
        // double precision
        else
        {
          TfToken typeNameToken = _dstAttribute.GetTypeName().GetAsToken();
          if (typeNameToken == SdfValueTypeNames->Vector3d ||
            typeNameToken == SdfValueTypeNames->Point3d ||
            typeNameToken == SdfValueTypeNames->Normal3d ||
            typeNameToken == SdfValueTypeNames->Color3d ||
            typeNameToken == SdfValueTypeNames->Double3)
          {
            /*
            VtArray<GfVec3d> vtArray(numElements);
            X2UCopyData<GfVec3d, GfVec3d>((GfVec3d*)datas, vtArray.data(), numElements);
            */
            _dstAttribute.Set(VtValue(), timeCode);
          }
          else
          {
            /*
            VtArray<GfVec3f> vtArray(numElements);
            X2UCastTuppledData<GfVec3d, GfVec3f>((GfVec3d*)datas, vtArray.data(), numElements, 3, 3);
            */
            _dstAttribute.Set(VtValue(), timeCode);
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
          if (typeNameToken == SdfValueTypeNames->Color4d ||
            typeNameToken == SdfValueTypeNames->Double4)
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
          if (typeNameToken == SdfValueTypeNames->Color4d ||
            typeNameToken == SdfValueTypeNames->Double4)
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
      }
    }
  }

   _hash = hash;

}

void X2UExportAttribute::WriteSample(const UsdTimeCode& timeCode)
{
  //if (_isArray)
  {
    
    // get ICE data array
    _srcDataType = X2UDataTypeFromICEType(_srcAttribute.GetDataType());
    _srcDataPrecision = X2U_PRECISION_SINGLE;

    switch (_srcDataType)
    {
    case X2U_DATA_BOOL:
    {
      CICEAttributeDataArrayBool datas;
      _srcAttribute.GetDataArray(datas);
      //WriteSample((void*)&datas[0], datas.GetCount(), timeCode);
      break;
    }
    case X2U_DATA_FLOAT:
    {
      CICEAttributeDataArrayFloat datas;
      _srcAttribute.GetDataArray(datas);
      WriteSample((void*)&datas[0], datas.GetCount(), timeCode);
      break;
    }
      
    case X2U_DATA_VECTOR2:
    {
      CICEAttributeDataArrayVector2f datas;
      _srcAttribute.GetDataArray(datas);
      WriteSample((void*)&datas[0], datas.GetCount(), timeCode);
      break;
    }

    case X2U_DATA_VECTOR3:
    {
      CICEAttributeDataArrayVector2f datas;
      _srcAttribute.GetDataArray(datas);
      WriteSample((void*)&datas[0], datas.GetCount(), timeCode);
      break;
    }

    case X2U_DATA_VECTOR4:
    {
      CICEAttributeDataArrayVector2f datas;
      _srcAttribute.GetDataArray(datas);
      WriteSample((void*)&datas[0], datas.GetCount(), timeCode);
      break;
    }

    case X2U_DATA_COLOR4:
    {
      CICEAttributeDataArrayColor4f datas;
      _srcAttribute.GetDataArray(datas);
      WriteSample((void*)&datas[0], datas.GetCount(), timeCode);
      break;
    }

    case X2U_DATA_MATRIX3:
    {
      CICEAttributeDataArrayMatrix3f datas;
      _srcAttribute.GetDataArray(datas);
      WriteSample((void*)&datas[0], datas.GetCount(), timeCode);
      break;
    }

    case X2U_DATA_MATRIX4:
    {
      CICEAttributeDataArrayMatrix3f datas;
      _srcAttribute.GetDataArray(datas);
      WriteSample((void*)&datas[0], datas.GetCount(), timeCode);
      break;
    }
    }
  }
}

void X2UExportAttribute::WriteInterpolation()
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