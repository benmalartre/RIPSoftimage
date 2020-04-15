//
// Copyright 2020 benmalartre
//
// unlicensed
//
#pragma once

#include <pxr/pxr.h>
#include <pxr/base/arch/hash.h>
#include <pxr/imaging/glf/glew.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/sdf/path.h>

#include <boost/functional/hash.hpp>
#include <iostream>

#include "attribute.h"
#include "topology.h"

class U2XVertexBuffer;

enum U2XAttributeChannel : short {
  CHANNEL_POSITION,
  CHANNEL_NORMAL,
  CHANNEL_TANGENT,
  CHANNEL_COLOR,
  CHANNEL_UV,
  CHANNEL_WIDTH,
  CHANNEL_ID,
  CHANNEL_SCALE,
  CHANNEL_SHAPE_POSITION,
  CHANNEL_SHAPE_NORMAL,
  CHANNEL_SHAPE_UV,
  CHANNEL_SHAPE_COLOR
};

enum U2XVertexBufferState : short {
  INVALID,
  TO_REALLOCATE,
  TO_UPDATE,
  TO_RECYCLE
};


typedef std::map<U2XAttributeChannel, U2XVertexBuffer*> U2XVertexBufferPtrMap;

/// \class U2XVertexBuffer
///
///
class U2XVertexBuffer
{
public:
  // constructor
  U2XVertexBuffer( U2XAttributeChannel channel, 
                    uint32_t numInputElements,
                    uint32_t numOutputElements,
    U2XAttributeInterpolation interpolation);

  // destructor
  ~U2XVertexBuffer();

  // hash
  size_t ComputeHash(const char* datas);
  inline size_t GetHash(){return _hash;};
  inline void SetHash(size_t hash){ _hash = hash;};

  // registry key
  size_t ComputeKey(const pxr::SdfPath& id);
  inline size_t GetKey(){return _key;};
  inline void SetKey(size_t key){_key = key;};

  // state
  inline bool GetNeedReallocate(){return _needReallocate;};
  inline void SetNeedReallocate(bool needReallocate) {
    _needReallocate = needReallocate;
  };
  inline bool GetNeedUpdate() { return  _needUpdate; };
  inline void SetNeedUpdate(bool needUpdate) {
    _needUpdate = needUpdate;
  };

  inline U2XAttributeInterpolation GetInterpolation(){return _interpolation;};
  inline void SetInterpolation(U2XAttributeInterpolation interpolation) {
    _interpolation = interpolation;
  };

  // raw input datas
  inline const void* GetRawInputDatas(){return _rawInputDatas;};
  inline void SetRawInputDatas(const char* datas, size_t numElements) {
      _rawInputDatas = datas;
      _numInputElements = numElements;
  };
  uint32_t GetNumInputElements() { return _numInputElements; };

  // compute output datas
  uint32_t GetNumOutputElements() {return _numOutputElements;};
  void SetNumOutputElements(uint32_t numOutputElements) {
    _numOutputElements = numOutputElements;
  };
  uint32_t ComputeOutputSize() {return _numOutputElements * _elementSize;};
  void ComputeOutputDatas(const U2XTopology* topo,
                          char* outputDatas);

  bool CheckTopoMatchDatas(const U2XTopology* topo);

  // opengl
  void Reallocate();
  void Populate(const void* triangulatedDatas);
  void Bind();
  GLuint Get() const {return _vbo;};

private: 
  // description
  std::string                       _name;
  short                             _channel;
  size_t                            _hash;
  size_t                            _key;
  uint32_t                          _numInputElements;
  uint32_t                          _numOutputElements;
  uint32_t                          _elementSize;
  
  bool                              _needReallocate;
  bool                              _needUpdate;
  U2XAttributeInterpolation         _interpolation;

  // datas
  const char*                       _rawInputDatas;

  // opengl
  GLuint                            _vbo;

};

