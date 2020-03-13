#version 330

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in vec3 vNormalPass[];

uniform float fNormalLength;

uniform struct Matrices
{
	mat4 projMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;                                                                           
	mat4 normalMatrix;
} matrices;


void main()
{
  mat4 mVP = matrices.projMatrix*matrices.viewMatrix;
  
  vec3 vNormal = (matrices.normalMatrix*vec4(vNormalPass[0]*fNormalLength, 1.0)).xyz;
  vNormal = normalize(vNormal);
  vec3 vPos = (matrices.modelMatrix * gl_in[0].gl_Position).xyz;
  gl_Position = mVP*vec4(vPos, 1.0);
  EmitVertex();

  gl_Position = mVP*vec4(vPos+vNormal, 1.0);
  EmitVertex();

  EndPrimitive();  
}