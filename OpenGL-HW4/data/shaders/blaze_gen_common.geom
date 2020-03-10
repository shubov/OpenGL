#version 330

#include_part

layout(points) in;
layout(points) out;
layout(max_vertices = 40) out;

in vec3 vPositionPass[];
in vec3 vVelocityPass[];
in vec3 vGravityPass[];
in vec3 vColorPass[];
in float fLifeTimePass[];
in float fSizePass[];
in int iTypePass[];

out vec3 vPositionOut;
out vec3 vVelocityOut;
out vec3 vGravityOut;
out vec3 vColorOut;
out float fLifeTimeOut;
out float fSizeOut;
out int iTypeOut;

uniform float fDeltaTime;
uniform vec3 vGenVelocityMin, vGenVelocityDif;
uniform vec3 vGenColor;
uniform vec3 vGenGravityVector;
uniform float fLifeMin, fLifeDif;
uniform float fTimeLeft;
uniform int iNumToGenerate;
uniform float fGenSize;
uniform int iGenerate; // It's used as bool

vec3 vLocalSeed;
uniform vec3 vSeed;

float rand()
{
    uint n = floatBitsToUint(vLocalSeed.y * 214013.0 + vLocalSeed.x * 2531011.0 + vLocalSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    vLocalSeed = vec3(vLocalSeed.x + 147158.0 * fRes, vLocalSeed.y*fRes  + 415161.0 * fRes, vLocalSeed.z + 324154.0*fRes);
    return fRes;
}