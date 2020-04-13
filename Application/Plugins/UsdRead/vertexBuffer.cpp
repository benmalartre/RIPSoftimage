//
// Copyright 2020 benmalartre
//
// Unlicensed
//
#include "pxr/imaging/glf/glew.h"
#include "utils.h"
#include "topology.h"
#include "vertexBuffer.h"

#include <pxr/base/tf/stringUtils.h>
#include <pxr/base/tf/diagnostic.h>

// constructor
U2XVertexBuffer::U2XVertexBuffer(U2XAttributeChannel channel,
  uint32_t numInputElements, uint32_t numOutputElements, 
  U2XAttributeInterpolation interpolation)
  : _channel(channel)
  , _hash(0)
  , _key(0)
  , _numInputElements(numInputElements)
  , _numOutputElements(numOutputElements) 
  , _needReallocate(true)
  , _needUpdate(true)
  , _interpolation(interpolation)
  , _vbo(0)
{
  switch(channel)
  {
    case CHANNEL_POSITION:
    case CHANNEL_NORMAL:
    case CHANNEL_COLOR:
      _elementSize = sizeof(pxr::GfVec3f);
      break;
    case CHANNEL_UV:
      _elementSize = sizeof(pxr::GfVec2f);
      break;
    default:
      _elementSize = 1;
      break;
  }
}

// destructor
U2XVertexBuffer::~U2XVertexBuffer()
{
  if(_vbo)glDeleteBuffers(1, &_vbo);
}

size_t U2XVertexBuffer::ComputeKey(const pxr::SdfPath& id)
{
  _key = id.GetHash();
  boost::hash_combine(_key, _channel);
  boost::hash_combine(_key, _numInputElements);
  boost::hash_combine(_key, _elementSize);
  return _key;
}

size_t U2XVertexBuffer::ComputeHash(const char* datas)
{
  _hash = pxr::ArchHash(datas, _numInputElements * _elementSize);
  boost::hash_combine(_hash, _key);
  return _hash;
}

// allocate
void U2XVertexBuffer::Reallocate()
{
  if(!_vbo)
  {
    glGenBuffers(1, &_vbo);
  } 

  glBindBuffer(GL_ARRAY_BUFFER, _vbo);

  glBufferData(
    GL_ARRAY_BUFFER, 
    _numOutputElements * _elementSize, 
    NULL, 
    GL_DYNAMIC_DRAW
  );

  glVertexAttribPointer(_channel, _elementSize / sizeof(float), GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(_channel);

  _needReallocate = false;
}

void U2XVertexBuffer::Populate(const void* datas)
{
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferSubData(
    GL_ARRAY_BUFFER, 
    0, 
    _numOutputElements * _elementSize,
    datas
  );
  _needUpdate = false;
}

bool U2XVertexBuffer::CheckTopoMatchDatas(const U2XTopology* topo)
{
  switch (topo->type)
  {
  case U2XTopology::Type::POINTS:
    break;
  case U2XTopology::Type::LINES:
    break;
  case  U2XTopology::Type::TRIANGLES:
    for (int i = 0; i < topo->numElements; ++i)
    {
      if (topo->samples[i] >= _numInputElements)return false;
    }
    break;
  }
  return true;
}


void U2XVertexBuffer::ComputeOutputDatas(const U2XTopology* topo,
                                          char* result)
{
  if(topo->type == U2XTopology::Type::POINTS)
  {
    //memcpy(result, topo->samples, _numInputElements * _elementSize);
  }
  else if(topo->type == U2XTopology::Type::LINES)
  {
    //memcpy(result, &topo->GetSamples()[0], _numInputElements * _elementSize);
  }
  else if(topo->type == U2XTopology::Type::TRIANGLES)
  {
    const int* samples = topo->samples;
    switch(_interpolation) 
    {
      case INTERPOLATION_CONSTANT:
      {
        for(size_t i=0;i<_numOutputElements;++i) 
        {
          memcpy(result + i * _elementSize, _rawInputDatas, _elementSize);
        }
        break;
      }
    
      case INTERPOLATION_UNIFORM:
      {
        for(size_t i=0;i<_numOutputElements;++i) 
        {
          memcpy(
            (void*)(result + i * _elementSize),
            (void*)(_rawInputDatas + samples[i * 3 + 1] * _elementSize), 
            _elementSize
          );
        }
        break;
      }

      case INTERPOLATION_VARYING:
      {
        for(size_t i=0;i<_numOutputElements;++i) 
        {
          memcpy(
            (void*)(result + i * _elementSize),
            (void*)(_rawInputDatas + samples[i * 3] * _elementSize), 
            _elementSize
          );
        }
        break;
      }
        
      case INTERPOLATION_VERTEX:
      {
        for(size_t i=0;i<_numOutputElements;++i) 
        {
          memcpy(
            (void*)(result + i * _elementSize),
            (void*)(_rawInputDatas + samples[i * 3] * _elementSize), 
            _elementSize
          );
        }
        break;
      }
        
      case INTERPOLATION_FACEVARYING:
      {
        for(size_t i=0;i<_numOutputElements;++i) 
        {
          memcpy(
            (void*)(result + i * _elementSize),
            (void*)(_rawInputDatas + samples[i * 3 + 2] * _elementSize), 
            _elementSize
          );
        }  
        break;
      } 
    }
  }
  
}

