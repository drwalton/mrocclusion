#version 410

#define METRES_PER_MM 0.0001
#define SCALE_DEPTH 10.0

smooth in vec2 texCoord;

uniform usampler2D tex;

out vec4 color;

vec4 hueToRgb(float H)
{
	return vec4(
		abs(H*6.f - 3.f) - 1.f,
		2.f - abs(H*6.f - 2.f),
		2.f - abs(H*6.f - 4.f),
		1
	);
}

void main()
{
	uvec4 depthMM = texture(tex, texCoord);
	color = hueToRgb(mod(float(depthMM.x) * SCALE_DEPTH * METRES_PER_MM, 1.0));
	color = depthMM.x == 0 ? vec4(0.0, 0.0, 0.0, 1.0) : color;
}
