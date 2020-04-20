#pragma once

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <boost/functional/hash.hpp>

#include <GL/glew.h>

static const char *VERTEX_SHADER =
"#version 330                                             \n"
"uniform mat4 model;                                      \n"
"uniform mat4 view;                                       \n"
"uniform mat4 projection;                                 \n"
"uniform mat4 normalMatrix;                               \n"
"                                                         \n"
"in vec3 position;                                        \n"
"in vec3 normal;                                          \n"
"in vec3 color;                                           \n"
"out vec3 vertex_position;                                \n"
"out vec3 vertex_normal;                                  \n"
"out vec3 vertex_color;                                   \n"
"void main(){                                             \n"
"    vertex_normal = normalize(mat3(normalMatrix) * normal); \n"
/*"    vertex_normal = normalize(mat3(transpose(inverse(model))) * normal); \n"*/
/*"    vertex_normal = normalize((model * vec4(normal, 0.0)).xyz); \n"*/
"    vertex_color = color;                                \n"
"    vertex_position = (model * vec4(position, 1.0)).xyz; \n"
"    gl_Position = projection * view * vec4(vertex_position, 1.0);        \n"
"}";

static const char* FRAGMENT_SHADER =
"#version 330                                             \n"
"uniform mat4 model;                                      \n"
"uniform mat4 view;                                       \n"
"uniform vec3 light;                                      \n"
"in vec3 vertex_position;                                 \n"
"in vec3 vertex_normal;                                   \n"
"in vec3 vertex_color;                                    \n"
"out vec4 outColor;                                       \n"
"void main()                                              \n"
"{                                                        \n"
" vec3 light_dir = normalize(light);                      \n"
" float NdotL = (dot(vertex_normal, light_dir) + 1.0) * 0.5;\n"
/*" float NdotL = max(dot(vertex_normal, light_dir), 0.0);  \n"*/
/*"	outColor = vec4(vertex_color * NdotL ,1.0);             \n"*/
" if(NdotL >0.5) outColor = vec4(vertex_color,1.0); \n"
" else outColor = vec4(vertex_color * 0.1, 1.0); \n"
//" outColor = vec4(vertex_normal, 1.0); \n"
"}";


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
  GLuint Get() const {return _pgm;};

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
