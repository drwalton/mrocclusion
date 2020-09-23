
#version 410

smooth in vec3 camPos;

out uint color;

void main()
{
	float depth = -camPos.z;
	float depthMM = depth * 1000.f;
	//float depthMM = gl_FragCoord.x;
	uint depthMMU = uint(depthMM);
	color = depthMMU;
	//color = uvec4(depthMMU, depthMMU, depthMMU, depthMMU);
}

