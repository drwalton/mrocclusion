
#version 410

layout(location = 0) in vec2 vPos;
layout(location = 2) in vec2 vTex;

smooth out vec2 texCoord;

void main()
{
	texCoord = vTex;
	gl_Position = vec4(vPos, 0.f, 1.0f);
}

