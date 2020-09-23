
#version 410

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;
uniform mat3 normToWorld;

smooth out vec3 norm;
smooth out vec2 texCoord;
smooth out vec3 worldPos;

void main()
{
	texCoord = vTex;
	norm = normToWorld * vNorm;
	worldPos = vec3(modelToWorld * vec4(vPos, 1.0f));
	gl_Position = worldToClip *  vec4(worldPos, 1.0f);
}

