//
// Copyright 2020 benmalartre
//
// unlicensed
//
#pragma once

#include "pxr/pxr.h"
#include "pxr/imaging/glf/glew.h"

#include "topology.h"
#include "vertexBuffer.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/matrix4d.h"

class U2XVertexBuffer;

typedef std::shared_ptr<class U2XVertexArray> U2XVertexArraySharedPtr;

/// \class U2XVertexArray
///
///
class U2XVertexArray
{
public:
  // constructor
  U2XVertexArray();

  // destructor
  ~U2XVertexArray();

  // get GL VAO
  GLuint Get() const {return _vao;};

  // buffers
  bool HasBuffer(U2XAttributeChannel channel);
  U2XVertexBuffer* GetBuffer(U2XAttributeChannel channel);
  void SetBuffer(U2XAttributeChannel channel, U2XVertexBuffer* buffer);

  static U2XVertexBuffer* 
  CreateBuffer( U2XAttributeChannel channel, 
                uint32_t numInputElements, 
                uint32_t numOutputElements,
                U2XAttributeInterpolation interpolation);

  // channels
  inline void SetHaveChannel(U2XAttributeChannel channel) { 
    _channels |= channel;
  };
  inline bool GetHaveChannel(U2XAttributeChannel channel) { 
    return ((_channels & channel) == channel);
  };

  // state
  inline bool GetNeedReallocate() { return _needReallocate; };

  inline void SetNeedReallocate(bool needReallocate) {
    _needReallocate = needReallocate;
  };
  inline bool GetNeedUpdate() { return _needUpdate; };
  inline void SetNeedUpdate(bool needUpdate) {
    _needUpdate = needUpdate;
  };
  void UpdateState();

  // elements
  inline uint32_t GetNumElements() const{return _numElements;};
  inline void SetNumElements(uint32_t numElements){_numElements = numElements;};

  // topology
  inline void SetTopologyPtr(const U2XTopology* topology) {
    _topology = topology;
    _needReallocate = true;
  }
  const U2XTopology* GetTopologyPtr() const {return _topology;};

  // allocate
  void Reallocate();
  void Populate();
  void Bind();
  void Unbind();

  // draw
  void Draw();

private:
  // datas
  U2XVertexBufferPtrMap             _buffers;
  GLuint                            _vao;
  GLuint                            _ebo;

  // flags
  uint32_t                          _channels;
  uint32_t                          _numElements;
  bool                              _needReallocate;
  bool                              _needUpdate;

  // topology
  const U2XTopology*               _topology;

};

