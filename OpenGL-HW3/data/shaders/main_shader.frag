#version 330

smooth in vec2 vTexCoord;
smooth in vec3 vNormal;
smooth in vec3 vEyeSpacePos;
smooth in vec3 vWorldPos;
out vec4 outputColor;

uniform sampler2D gSampler;
uniform vec4 vColor;

#include "dirLight.frag"
#include "spotLight.frag"
#include "pointLight.frag"

uniform DirectionalLight sunLight;
uniform SpotLight spotLight;
uniform PointLight pointLight;
uniform SpotLight spotLight2;
uniform PointLight pointLight2;

void main()
{
	vec3 vNormalized = normalize(vNormal);
	
	vec4 vTexColor = texture(gSampler, vTexCoord);
	vec4 vMixedColor = vTexColor*vColor;
	vec4 vDirLightColor = getDirectionalLightColor(sunLight, vNormalized);
	vec4 vSpotlightColor = GetSpotLightColor(spotLight, vWorldPos);
	vec4 vSpotlightColor2 = GetSpotLightColor(spotLight2, vWorldPos);
	vec4 vPointlightColor = getPointLightColor(pointLight, vWorldPos, vNormalized);
	vec4 vPointlightColor2 = getPointLightColor(pointLight2, vWorldPos, vNormalized);
	outputColor = vColor*vMixedColor*(vDirLightColor+vSpotlightColor+vPointlightColor+vSpotlightColor2+vPointlightColor2);
}