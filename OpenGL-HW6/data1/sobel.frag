// ��������
uniform sampler2D colorTexture, depthTexture;
uniform float xPos;

// ���������, ���������� �� ���������� �������
in Vertex
{
	vec2 texcoord;
} Vert;

layout(location = FRAG_OUTPUT0) out vec4 color;

// ���� �������
#define KERNEL_SIZE 9

const float kernel1[KERNEL_SIZE] = float[](
	-3.0, 0.0, 3.0,
	-10.0, 0.0, 10.0,
	-3.0, 0.0, 3.0
);

const float kernel2[KERNEL_SIZE] = float[](
	-3.0, -10.0, -3.0,
	0.0, 0.0, 0.0,
	3.0, 10.0, 3.0
);

const float div = 6;

const vec2 offset[KERNEL_SIZE] = vec2[](
	vec2(-1.0,-1.0), vec2( 0.0,-1.0), vec2( 1.0,-1.0),
	vec2(-1.0, 0.0), vec2( 0.0, 0.0), vec2( 1.0, 0.0),
	vec2(-1.0, 1.0), vec2( 0.0, 1.0), vec2( 1.0, 1.0)
);

vec3 filter1(in vec2 texcoord)
{
	vec2 pstep = vec2(1.0) / vec2(textureSize(colorTexture, 0));
	vec4 res1   = vec4(0.0);

	for (int i = 0; i < KERNEL_SIZE; ++i)
		res1 += texture(colorTexture, texcoord + offset[i] * pstep) * kernel1[i]/div;

	vec4 res2   = vec4(0.0);

	for (int i = 0; i < KERNEL_SIZE; ++i)
		res2 += texture(colorTexture, texcoord + offset[i] * pstep) * kernel2[i]/div;

	return vec3(sqrt(res1*res1 + res2*res2));
}

void main(void)
{
	vec3 texel = Vert.texcoord.x < xPos ? filter1(Vert.texcoord)
		: texture(colorTexture, Vert.texcoord).rgb;

	color = vec4(texel, 1.0);
}
