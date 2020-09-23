
#version 410

smooth in vec2 texCoord;
smooth in vec3 norm;
smooth in vec3 worldPos;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform sampler2D tex;
uniform float specularity;
uniform float diffuseReflectance;
uniform float ambientLight;
uniform vec3 specularColor;
uniform vec3 lightWorldPos;

out vec4 color;

void main()
{
	vec3 albedo = texture(tex, texCoord).xyz;
	vec3 lightRay = normalize(worldPos - lightWorldPos);
	vec3 reflectRay = reflect(lightRay, norm);
	vec3 viewRay = vec3(cameraPos) - worldPos.xyz;
	float specularAmt = clamp(dot(viewRay, reflectRay), 0.0, 1.0);
	float diffuseAmt = clamp(dot(norm, -lightRay), 0.0, 1.0);
	vec3 specularComponent = specularColor * pow(specularAmt, specularity);
	color = vec4((ambientLight + diffuseAmt*diffuseReflectance)*albedo + specularComponent, 1.0);
}

