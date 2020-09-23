
#version 410

layout(location = 0) in vec3 vPos;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;

uniform vec3 lightWorldPos;
uniform vec3 planeN;
uniform float planeD;

void main()
{
	vec3 worldPos = vec3(modelToWorld * vec4(vPos, 1.0f));
	vec3 rayDir = normalize(worldPos - lightWorldPos);
	float t = -(dot(lightWorldPos, planeN) + planeD) / dot(rayDir, planeN);
	vec3 onPlane = lightWorldPos + t*rayDir;
	gl_Position = worldToClip *  vec4(onPlane, 1.0f);
}

