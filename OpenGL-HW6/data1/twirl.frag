uniform sampler2D colorTexture, depthTexture;
uniform float amount;

// ïàðàìåòðû ïîëó÷åííûå èç âåðøèííîãî øåéäåðà
in Vertex
{
	vec2 texcoord;
} Vert;

layout(location = FRAG_OUTPUT0) out vec4 color;

void main (void)
{
	if(Vert.texcoord.x < (amount/10)+0.5 && Vert.texcoord.y < 0.01 && Vert.texcoord.y > 0.0)
	{	
		color = vec4(0.0, 1.0, 0.0, 1.0);
		return;
	}

	vec2 uv = Vert.texcoord-0.5;
	uv.y *= 1.0; 
	float angle = atan(uv.y,uv.x);
	float radius = length(uv);
	angle+= radius * amount;
	vec2 shifted = radius*vec2(cos(angle), sin(angle));
	color = texture(colorTexture, shifted+0.5);
}