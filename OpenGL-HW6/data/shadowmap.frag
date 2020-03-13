﻿// текстурный самплер карты глубины (z-буфера)
uniform sampler2DShadow depthTexture;

// параметры точеченого источника освещения
uniform struct Light
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
} light;

// параметры материала
uniform struct Material
{
	sampler2D texture;

	vec4  ambient;
	vec4  diffuse;
	vec4  specular;
	vec4  emission;
	float shininess;
} material;

// параметры полученные из вершинного шейдера
in Vertex
{
	vec2 texcoord;
	vec4 smcoord;
	vec3 normal;
	vec3 lightDir;
	vec3 viewDir;
} Vert;

layout(location = FRAG_OUTPUT0) out vec4 color;

// сглаживание краев тени
float PCF(in vec4 smcoord)
{
	float res = 0.0;

	res += textureProjOffset(depthTexture, smcoord, ivec2(-1,-1));
	res += textureProjOffset(depthTexture, smcoord, ivec2( 0,-1));
	res += textureProjOffset(depthTexture, smcoord, ivec2( 1,-1));
	res += textureProjOffset(depthTexture, smcoord, ivec2(-1, 0));
	res += textureProjOffset(depthTexture, smcoord, ivec2( 0, 0));
	res += textureProjOffset(depthTexture, smcoord, ivec2( 1, 0));
	res += textureProjOffset(depthTexture, smcoord, ivec2(-1, 1));
	res += textureProjOffset(depthTexture, smcoord, ivec2( 0, 1));
	res += textureProjOffset(depthTexture, smcoord, ivec2( 1, 1));

	return (res / 9.0);
}

void main(void)
{
	// нормализуем полученные данные для коррекции интерполяции
	vec3 normal   = normalize(Vert.normal);
	vec3 lightDir = normalize(Vert.lightDir);
	vec3 viewDir  = normalize(Vert.viewDir);

	// коэффициент затенения
	float shadow  = PCF(Vert.smcoord);

	// добавим собственное свечение материала
	color = material.emission;

	// добавим фоновое освещение
	color += material.ambient * light.ambient;

	// добавим рассеянный свет
	float NdotL = max(dot(normal, lightDir), 0.0);
	color += material.diffuse * light.diffuse * NdotL;

	// добавим отраженный свет
	float RdotVpow = max(pow(dot(reflect(-lightDir, normal), viewDir), material.shininess), 0.0);
	color += material.specular * light.specular * RdotVpow;

	// вычислим итоговый цвет пикселя на экране с учетом текстуры
	color *= texture(material.texture, Vert.texcoord) * shadow;
}
