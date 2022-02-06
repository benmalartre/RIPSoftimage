#version 330

uniform mat4 model;
uniform mat4 view;
uniform vec3 light;
in vec3 vertex_position;
in vec3 vertex_normal;
in vec3 vertex_color;
out vec4 outColor;
void main()
{
  vec3 light_dir = normalize(light);
 float NdotL = (dot(vertex_normal, light_dir) + 1.0) * 0.5;
// float NdotL = max(dot(vertex_normal, light_dir), 0.0);
//	outColor = vec4(vertex_color * NdotL ,1.0);
 if(NdotL >0.5) outColor = vec4(vertex_color,1.0);
 else outColor = vec4(vertex_color * 0.1, 1.0);
// outColor = vec4(vertex_normal, 1.0);
}