uniform sampler2D colorTexture, depthTexture;
uniform float amount;

in Vertex
{
	vec2 texcoord;
} Vert;

layout(location = FRAG_OUTPUT0) out vec4 color;

void main() {
   vec2 uv = Vert.texcoord-0.5;
   float angle = atan(uv.y,uv.x);
   float radius = length(uv);
   angle+= radius*amount;
   vec2 shifted = radius*vec2(cos(angle), sin(angle));
   color = texture(colorTexture, (shifted+0.5));
}