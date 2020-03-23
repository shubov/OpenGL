//F5
// текстуры
uniform sampler2D colorTexture, depthTexture;
uniform float xPos;

// параметры полученные из вершинного шейдера
in Vertex
{
	vec2 texcoord;
} Vert;

layout(location = FRAG_OUTPUT0) out vec4 color;

// инверсия цвета
vec3 filter1(in vec2 texcoord)
{
	return (vec3(1.0) - texture(colorTexture, texcoord).rgb);
}

void main(void)
{
	vec3 texel = Vert.texcoord.x < xPos ? filter1(Vert.texcoord)
		: texture(colorTexture, Vert.texcoord).rgb;

	color = vec4(texel, 1.0);
}
