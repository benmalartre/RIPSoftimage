//
// Copyright 2020 benmalartre
//
// Unlicensed
//
#include "pxr/imaging/glf/glew.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "vertexBuffer.h"
#include "vertexArray.h"
#include "utils.h"
#include <iostream>

// constructor
U2XVertexArray::U2XVertexArray()
: _vao(0)
, _channels(0)
, _needReallocate(true)
, _needUpdate(true)
{
}

// destructor
U2XVertexArray::~U2XVertexArray()
{
  for (auto& buffer : _buffers)
    if (buffer.second)delete buffer.second;
  _buffers.clear();
  if(_vao)glDeleteVertexArrays(1, &_vao);
}

// state
void 
U2XVertexArray::UpdateState()
{
  for(const auto& elem: _buffers)
  {
    U2XVertexBuffer* buffer = elem.second;
    if(buffer->GetNeedUpdate())
    {
      _needUpdate = true;
      break;
    }
  }
}

// allocate
void 
U2XVertexArray::Reallocate()
{
  if(!_vao)glGenVertexArrays(1, &_vao);

  //Bind();

  for(auto& elem: _buffers)
  {
    U2XVertexBuffer* buffer = elem.second;
    if (!buffer->GetNumInputElements())continue;
    buffer->Reallocate();
  }
  //Unbind();
  _needReallocate = false;
  _needUpdate = true;
}

void 
U2XVertexArray::Populate()
{
  //Bind();
  for(auto& elem: _buffers)
  {
    U2XVertexBuffer* buffer = elem.second;
    if (!buffer->GetNumInputElements())continue;
    if(buffer->GetNeedUpdate())
    {
      pxr::VtArray<char> datas(buffer->ComputeOutputSize());

      buffer->ComputeOutputDatas(_topology, datas.data());
      buffer->Populate(datas.cdata());
    }
  }
  _needUpdate = false;
  //Unbind();
}

// draw
void
U2XVertexArray::Draw()
{
  Bind();
  glDrawArrays(GL_TRIANGLES, 0, _numElements);
  Unbind();
}

// allocate
void 
U2XVertexArray::Bind()
{

  glBindVertexArray(_vao);

  for (auto& elem : _buffers)
  {
    U2XVertexBuffer* buffer = elem.second;
    buffer->Bind();
  }
}

void 
U2XVertexArray::Unbind() 
{
  glBindVertexArray(0);
}

bool 
U2XVertexArray::HasBuffer(U2XAttributeChannel channel)
{
  if(_buffers.find(channel) != _buffers.end())return true;
  else return false;
}

U2XVertexBuffer*
U2XVertexArray::GetBuffer(U2XAttributeChannel channel)
{
  return _buffers[channel];
}

void 
U2XVertexArray::SetBuffer(U2XAttributeChannel channel, U2XVertexBuffer* buffer)
{
  if(HasBuffer(channel))
  {
    U2XVertexBuffer* old = _buffers[channel];
    if (old)delete old;
    buffer->SetNeedReallocate(true);
  }
  _buffers[channel] = buffer;
}

U2XVertexBuffer*
U2XVertexArray::CreateBuffer(U2XAttributeChannel channel, 
  uint32_t numInputElements, uint32_t numOutputElements, U2XAttributeInterpolation interpolation)
{
  return new U2XVertexBuffer(channel, numInputElements, numOutputElements, interpolation);
}
