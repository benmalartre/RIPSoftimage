#version 330
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMatrix;

in vec3 position;
in vec3 normal;
in vec3 color;
out vec3 vertex_position;
out vec3 vertex_normal;
out vec3 vertex_color;

void main(){
  vertex_normal = normalize(mat3(normalMatrix) * normal);
  //vertex_normal = normalize(mat3(transpose(inverse(model))) * normal);
  //vertex_normal = normalize((model * vec4(normal, 0.0)).xyz);
  vertex_color = color;
  vertex_position = (model * vec4(position, 1.0)).xyz;
  gl_Position = projection * view * vec4(vertex_position, 1.0);
}