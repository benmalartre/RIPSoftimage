#pragma once

#include "common.h"

// Attribute
class X2UAttribute {
public:
  X2UAttribute();

  X2UAttribute(
    const UsdAttribute& dst,
    X2UDataType type,
    X2UDataPrecision precision,
    bool isArray,
    bool isPrimvar=false,
    X2UPrimvarInterpolation primvarInterp=X2U_INTERPOLATION_CONSTANT
  );

  X2UAttribute(
    const UsdAttribute& dstAttr,
    const int srcAttrIndex,
    bool isArray = true
  );

  ~X2UAttribute();

  inline void SetHash(uint64_t hash) { _hash = hash; };
  inline uint64_t GetHash() { return _hash; };
  inline bool FromICE() { return _fromICE; };
  void SetVariability(SdfVariability variability);
  bool IsConstant() { return _isConstant;};
  
  void WriteSample(const void* datas, uint32_t numElements, const UsdTimeCode& timeCode);
  void WriteSample(const TfToken& token, const UsdTimeCode& timeCode);
  void WriteSample(const Geometry& geom, const UsdTimeCode& timeCode);
  void WriteEmptySample(const UsdTimeCode& timeCode);
  void WriteInterpolation();
  void SetSourceType(X2UDataType type, X2UDataPrecision precision);
  pxr::UsdAttribute& GetOutput() { return _dstAttribute; };

private:
  X2UDataType                     _srcDataType;
  X2UDataPrecision                _srcDataPrecision;
  SdfValueTypeName                _dstDataType;
  int                             _srcAttributeIndex;
  UsdAttribute                    _dstAttribute;
  bool                            _isArray;
  bool                            _isPrimvar;
  X2UPrimvarInterpolation         _primvarInterpolation;
  bool                            _primvarIsIndexed;
  bool                            _isConstant;
  bool                            _fromICE;    
  siICENodeDataType               _ICEDataType;
  siICENodeContextType            _ICEContextType;
  siICENodeStructureType          _ICEStructureType;
  uint64_t                        _hash;
};

typedef std::map<std::string, X2UAttribute> X2UAttributeMap;
typedef std::vector<X2UAttribute*> X2UAttributePtrList;