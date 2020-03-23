uniform sampler2D colorTexture, depthTexture;
uniform float xPos;

in Vertex
{
	vec2 texcoord;
} Vert;

layout(location = FRAG_OUTPUT0) out vec4 color;

#define KERNEL_SIZE 25

const float div = 1;

const float kernel[KERNEL_SIZE] = float[](
0.012841,	0.026743,	0.03415,	0.026743,	0.012841,
0.026743,	0.055697,	0.071122,	0.055697,	0.026743,
0.03415,	0.071122,	0.090818,	0.071122,	0.03415,
0.026743,	0.055697,	0.071122,	0.055697,	0.026743,
0.012841,	0.026743,	0.03415,	0.026743,	0.012841
);

const vec2 offset[KERNEL_SIZE] = vec2[](
	vec2(-2.0,-2.0), vec2( -1.0,-2.0), vec2( 0.0,-2.0), vec2( 1.0,-2.0), vec2( 2.0,-2.0),
	vec2(-2.0, -1.0), vec2(-1.0, -1.0), vec2(0.0, -1.0), vec2(1.0, -1.0), vec2(2.0, -1.0),
	vec2(-2.0, 0.0), vec2( -1.0, 0.0), vec2( 0.0, 0.0), vec2( 1.0, 0.0), vec2( 2.0, 0.0),
	vec2(-2.0, 1.0), vec2( -1.0, 1.0), vec2( 0.0, 1.0), vec2( 1.0, 1.0), vec2( 2.0, 1.0),
	vec2(-2.0, 2.0), vec2( -1.0, 2.0), vec2( 0.0, 2.0), vec2( 1.0, 2.0), vec2( 2.0, 2.0)
);

vec3 filter1(in vec2 texcoord)
{
	vec2 pstep = vec2(1.0) / vec2(textureSize(colorTexture, 0));
	vec4 res   = vec4(0.0);

	for (int i = 0; i < KERNEL_SIZE; ++i)
		res += texture(colorTexture, texcoord + offset[i] * pstep) * kernel[i]/div;

	return vec3(res);
}

void main(void)
{
	vec3 texel = Vert.texcoord.x < xPos ? filter1(Vert.texcoord) : texture(colorTexture, Vert.texcoord).rgb;
	color = vec4(texel, 1.0);
}
