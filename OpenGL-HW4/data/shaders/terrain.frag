#version 330

smooth in vec2 vTexCoord;
smooth in vec3 vNormal;
smooth in vec3 vWorldPos;
smooth in vec4 vEyeSpacePos;

uniform sampler2D gSampler[5];
uniform sampler2D shadowMap;

uniform vec4 vColor;

#include "dirLight.frag"
uniform DirectionalLight sunLight;
uniform float fRenderHeight;
uniform float fMaxTextureU;
uniform float fMaxTextureV;

out vec4 outputColor;

uniform vec3 vEyePosition;

uniform Material matActive;


void main()
{
	vec3 vNormalized = normalize(vNormal);
	
	vec4 vTexColor = vec4(0.0);
	
	float fScale = vWorldPos.y/fRenderHeight;

	const float fRange1 = 0.15f;
	const float fRange2 = 0.3f;
	const float fRange3 = 0.65f;
   const float fRange4 = 0.85f;

	if(fScale >= 0.0 && fScale <= fRange1)vTexColor = texture2D(gSampler[0], vTexCoord);
	else if(fScale <= fRange2)
	{
		fScale -= fRange1;
		fScale /= (fRange2-fRange1);
		
		float fScale2 = fScale;
		fScale = 1.0-fScale; 
		
		vTexColor += texture2D(gSampler[0], vTexCoord)*fScale;
		vTexColor += texture2D(gSampler[1], vTexCoord)*fScale2;
	}
	else if(fScale <= fRange3)vTexColor = texture2D(gSampler[1], vTexCoord);
	else if(fScale <= fRange4)
	{
		fScale -= fRange3;
		fScale /= (fRange4-fRange3);
		
		float fScale2 = fScale;
		fScale = 1.0-fScale; 
		
		vTexColor += texture2D(gSampler[1], vTexCoord)*fScale;
		vTexColor += texture2D(gSampler[2], vTexCoord)*fScale2;		
	}
	else vTexColor = texture2D(gSampler[2], vTexCoord);

	vec2 vPathCoord = vec2(vTexCoord.x/fMaxTextureU, vTexCoord.y/fMaxTextureV);
	vec4 vPathIntensity = texture2D(gSampler[4], vPathCoord);
	fScale = vPathIntensity.x;
  
	vec4 vPathColor = texture2D(gSampler[3], vTexCoord); // Black color means there is a path
	vec4 vFinalTexColor = fScale*vTexColor+(1-fScale)*vPathColor;

	vec4 vMixedColor = vFinalTexColor*vColor;
	vec4 vDirLightColor = GetDirectionalLightColor(sunLight, vNormal);
  vec4 vSpecularColor = GetSpecularColor(vWorldPos, vEyePosition, matActive, sunLight, vNormalized);


	outputColor = vMixedColor*(vDirLightColor+vSpecularColor);
  
}                      