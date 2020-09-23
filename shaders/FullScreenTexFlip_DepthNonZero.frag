#version 410

smooth in vec2 texCoord;

uniform sampler2D tex;
uniform usampler2D depth;

out vec4 color;

void main()
{
	vec2 flipCoord = vec2(texCoord.x, 1 - texCoord.y);
	uint depth = texture(depth, flipCoord).r;
	if(depth == 0) discard;
	color = texture(tex, flipCoord);
}
