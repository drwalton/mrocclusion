#version 410

smooth in vec2 texCoord;

uniform sampler2D realTex;
uniform sampler2D matte;
uniform sampler2D virtualTex;

out vec4 color;

void main()
{
	vec2 flipCoord = vec2(texCoord.x, 1 - texCoord.y);
	vec3 realSample = texture(realTex, flipCoord).rgb;
	float matteVal = texture(matte, flipCoord).r;
	vec3 virtSample = texture(virtualTex, texCoord).rgb;

	vec3 outRgb = matteVal*virtSample + (1-matteVal)*realSample;
	color = vec4(outRgb, 1.0);
}

