#version 410

smooth in vec2 texCoord;

uniform sampler2D tex;
uniform usampler2D depth;

out vec4 color;

void main()
{
	uint depth = texture(depth, texCoord).r;
	if(depth == 0) discard;
	color = texture(tex, texCoord);
}
