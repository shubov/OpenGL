// F2
// текстуры
uniform sampler2D colorTexture, depthTexture;
uniform float xPos;

// параметры полученные из вершинного шейдера
in Vertex
{
	vec2 texcoord;
} Vert;

layout(location = FRAG_OUTPUT0) out vec4 color;

const vec3 factor = vec3(0.27, 0.67, 0.06);

vec3 filter1(in vec2 texcoord)
{
        // скалярное произведение

	return vec3(dot(factor, texture(colorTexture, texcoord).rgb));
}

void main(void)
{
	vec3 texel = Vert.texcoord.x <xPos ? filter1(Vert.texcoord)
		: texture(colorTexture, Vert.texcoord).rgb;

	color = vec4(texel, 1.0);
}
