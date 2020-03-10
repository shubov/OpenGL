#version 330

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec3 inNormal;

out vec3 vNormalPass;

void main()
{
   gl_Position = vec4(inPosition, 1.0);
   vNormalPass = inNormal;
}