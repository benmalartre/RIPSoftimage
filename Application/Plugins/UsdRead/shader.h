#pragma once

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <boost/functional/hash.hpp>

#include <GL/glew.h>


enum U2XGLSLShaderType
{
  SHADER_VERTEX,
  SHADER_GEOMETRY,
  SHADER_FRAGMENT
};
    
class U2XGLSLShader{
public:
  U2XGLSLShader():_id(0),_code(""){};
  U2XGLSLShader(const U2XGLSLShader& ) = delete;
  U2XGLSLShader(U2XGLSLShader&&) = delete;

  ~U2XGLSLShader()
  {
    if(_id)glDeleteShader(_id);
  }
  void Load(const char* filename, GLenum t=0);
  void Compile();
  void OutputInfoLog();
  GLuint Get(){return _id;};
  std::string& GetCode(){return _code;};
  void Set(const char* code, GLenum type);
  void _ComputeHash();
  size_t Hash(){return _hash;};

private:
  std::string         _code;
  GLenum              _type;
  GLuint              _id;
  size_t              _hash;
};

typedef std::shared_ptr<U2XGLSLShader> LoFiGLSLShaderSharedPtr;
    
class U2XGLSLProgram
{
friend U2XGLSLShader;
public:
  // constructor (empty program)
  U2XGLSLProgram():_vert(NULL),_geom(NULL),_frag(NULL),_pgm(0){};
  
  // destructor
  ~U2XGLSLProgram()
  {
    if(_pgm)glDeleteProgram(_pgm);
  }

  /// internal build of the glsl program
  void _Build();

  /// build glsl program from vertex and fragment code
  void Build(const char* name, const char* s_vert, const char* s_frag);

  /// build glsl program from vertex, geometry and fragment code
  void Build(const char* name, const char* s_vert, const char* s_geom, 
    const char* s_frag);

  /// build glsl program from vertex and fragment LoFiGLSLShader objects
  void Build(const char* name, U2XGLSLShader* vertex,
    U2XGLSLShader* fragment);

  /// build glsl program from vertex, geometry and fragment LoFiGLSLShader 
  /// objects
  void Build(const char* name, U2XGLSLShader* vertex,
    U2XGLSLShader* geom, U2XGLSLShader* fragment);

  /// output build program info log
  void OutputInfoLog();

  /// compute hash
  void _ComputeHash();
  size_t Hash(){return _hash;};

  /// get GL program id
  GLuint Get(){return _pgm;};

  /// this object is non-copyable
  U2XGLSLProgram(const U2XGLSLProgram&) = delete;
  U2XGLSLProgram(U2XGLSLProgram&&) = delete;

private:
  U2XGLSLShader *         _vert;
  U2XGLSLShader*         _geom;
  U2XGLSLShader*         _frag;
  GLuint                  _pgm;
  std::string             _name; 
  size_t                  _hash;
};

typedef std::shared_ptr<U2XGLSLShader> LoFiGLSLProgramSharedPtr;
