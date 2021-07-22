#include <iostream>
#include <fstream>
#include <sstream>
#include "pxr/base/arch/hash.h"
#include "shader.h"


void U2XGLSLShader::_ComputeHash()
{
  _hash = pxr::ArchHash(_code.c_str(), _code.size(), 0);;
}

void U2XGLSLShader::OutputInfoLog()
{
  char buffer[512];
  glGetShaderInfoLog(_id, 512, NULL, &buffer[0]);
 
  std::cerr << "[U2X][Compile GLSL shader] Info log : " << 
  (std::string)buffer << std::endl;
}

void U2XGLSLShader::Load(const char* filename, GLenum type)
{
  std::ifstream file;
  file.open(filename, std::ios::in);
  if(file.is_open())
  {
    if(file.good())
    {
      file.seekg(0,std::ios::end);
      unsigned long len = file.tellg();
      file.seekg(std::ios::beg);

      if(len>0)
      {
        _code.assign((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
        _code += '\0';
      }
      else std::cerr << "[U2X][Load shader from file] File is empty : " << 
        filename << std::endl;
    }
    else std::cerr << "[U2X][Load shader from file] File is invalid : " <<
      filename << std::endl;
    
  }
  else std::cerr << "[U2X][Load shader from file] Failure open file : " << 
    filename << std::endl;

  _type = type;
}

void U2XGLSLShader::Set(const char* code, GLenum type)
{
  _code = code;
  _type = type;
}

void U2XGLSLShader::Compile()
{
  _id = glCreateShader(_type);
  const char* code[1] = { _code.c_str() };
  glShaderSource(_id, 1, code, NULL);
  glCompileShader(_id);
  
  GLint status;
  glGetShaderiv(_id,GL_COMPILE_STATUS,&status);
  if(status != GL_TRUE)
  {
    std::cerr << "[U2X][Compile Shader] Fail compiling code: \n" << 
      _code << std::endl;

    OutputInfoLog();
  }
  _ComputeHash();
}

void U2XGLSLProgram::_ComputeHash()
{
  _hash = 0;
  if(_vert)boost::hash_combine(_hash, _vert->Hash());
  if(_geom)boost::hash_combine(_hash, _geom->Hash());
  if(_frag)boost::hash_combine(_hash, _frag->Hash());
}

void U2XGLSLProgram::_Build()
{  
  _pgm = glCreateProgram();
  
  if(_vert)
  {
    _vert->Compile();
    glAttachShader(_pgm,_vert->Get());
  }

  if(_geom && _geom->Get())
  {
    _geom->Compile();
    glAttachShader(_pgm,_geom->Get());
  }
  
  if(_frag)
  {
    _frag->Compile();
    glAttachShader(_pgm,_frag->Get());
  }
  
  glBindAttribLocation(_pgm,0,"position");
  glLinkProgram(_pgm);  
  glUseProgram(_pgm);
  
  GLint status = 0;
  glGetProgramiv(_pgm, GL_LINK_STATUS, (int *)&status);
  if(status == GL_TRUE) 
  {
    std::cerr << "[U2X][Build GLSL program] Success : " << _name <<std::endl;
  } 
  else 
  {
    glDeleteProgram(_pgm);
    std::cerr << "[U2X][Build GLSL program] Fail : " << _name << std::endl;
    OutputInfoLog();
  }
  glUseProgram(0);
  _ComputeHash();
}

void U2XGLSLProgram::Build(const char* name, const char* vertex,
  const char* fragment)
{
  _name = name;
  U2XGLSLShader vertShader;
  vertShader.Set(vertex, GL_VERTEX_SHADER);
  _vert = &vertShader;

  _geom = NULL;

  U2XGLSLShader fragShader;
  fragShader.Set(fragment, GL_FRAGMENT_SHADER);
  _frag = &fragShader;

  _Build();
}

void U2XGLSLProgram::Build(const char* name, const char* vertex, 
  const char* geom, const char* fragment)
{
  _name = name;
  U2XGLSLShader vertShader;
  vertShader.Set(vertex, GL_VERTEX_SHADER);
  _vert = &vertShader;

  U2XGLSLShader geomShader;
  geomShader.Set(geom, GL_GEOMETRY_SHADER);
  _geom = &geomShader;

  U2XGLSLShader fragShader;
  fragShader.Set(fragment, GL_FRAGMENT_SHADER);
  _frag = &fragShader;

  _Build();
}

void U2XGLSLProgram::Build(const char* name, U2XGLSLShader* vertex,
  U2XGLSLShader* fragment)
{
  _name = name;
  _vert = vertex;
  _frag = fragment;

  _Build();
}

void U2XGLSLProgram::Build(const char* name, U2XGLSLShader* vertex,
  U2XGLSLShader* geom, U2XGLSLShader* fragment)
{
  _name = name;
  _vert = vertex;
  _geom = geom;
  _frag = fragment;

  _Build();
}

void U2XGLSLProgram::OutputInfoLog()
{
  char buffer[1024];
  GLsizei l;
  glGetProgramInfoLog(_pgm,1024,&l,&buffer[0]);
  std::cerr << "[U2X][Build GLSL program] Info log : " << 
    (std::string)buffer << std::endl;
}
