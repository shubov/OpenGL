uniform sampler2D colorTexture, depthTexture;
uniform float xPos;

in Vertex
{
	vec2 texcoord;
} Vert;

layout(location = FRAG_OUTPUT0) out vec4 color;

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

float luma(vec3 color) {
  return dot(color, vec3(0.299, 0.587, 0.114));
}

float luma(vec4 color) {
  return dot(color.rgb, vec3(0.299, 0.587, 0.114));
}

vec3 filter1(in vec2 texcoord)
{
	vec2 pstep = vec2(1.0) / vec2(textureSize(colorTexture, 0));
	vec2 res = vec2(0.0);

	float lum;
	for (int i = 0; i < KERNEL_SIZE; ++i)
	{
		lum = texture(colorTexture, texcoord + offset[i] * pstep).r;
		res.x += lum * kernel1[i]/div;
		res.y += lum * kernel2[i]/div;
	}

	float gradientMagnitude = length(res);
    vec2 normalizedDirection = normalize(res);
    normalizedDirection = sign(normalizedDirection) * floor(abs(normalizedDirection) + 0.617316); // Offset by 1-sin(pi/8) to set to 0 if near axis, 1 if away
    normalizedDirection = (normalizedDirection + 1.0) * 0.5; // Place -1.0 - 1.0 within 0 - 1.0

	return vec4(gradientMagnitude, normalizedDirection.x, normalizedDirection.y, 1.0);
}

void main(void)
{
	vec3 texel = Vert.texcoord.x < xPos ? filter1(Vert.texcoord) : texture(colorTexture, Vert.texcoord).rgb;
	color = vec4(texel, 1.0);
}
