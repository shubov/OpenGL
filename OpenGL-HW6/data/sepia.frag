﻿//F3
// текстуры
uniform sampler2D colorTexture, depthTexture;

// параметры полученные из вершинного шейдера
in Vertex
{
	vec2 texcoord;
} Vert;

layout(location = FRAG_OUTPUT0) out vec4 color;

const vec3 factor     = vec3(0.27, 0.67, 0.06);
const vec3 darkColor  = vec3(0.2, 0.05, 0.0);
const vec3 lightColor = vec3(1.0,  0.9, 0.5);

vec3 filter(in vec2 texcoord)
{
     return mix(darkColor, lightColor, 
            dot(factor, texture(colorTexture, texcoord).rgb));
}

void main(void)
{
	vec3 texel = Vert.texcoord.x < 0.5 ? filter(Vert.texcoord)
		: texture(colorTexture, Vert.texcoord).rgb;

	color = vec4(texel, 1.0);
}
