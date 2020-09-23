
#version 410

layout(location = 0) in vec3 vPos;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;
uniform mat4 worldToCam;

smooth out vec3 camPos;

void main()
{
	vec4 worldPos = modelToWorld * vec4(vPos, 1.0f);
	camPos = (worldToCam * worldPos).xyz;
	gl_Position = worldToClip *  worldPos;
}

