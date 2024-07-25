#version 330

in vec3 FragPos;
in vec2 TexCoord;
in vec3 FragNormal;

out vec4 FragColor;

struct Light
{
	vec4 color;
	float ambient;
	float diffuse;
};

struct DirectionalLight
{
	Light base;
	vec3 direction;
};

struct Material
{
	float specular;
	float shininess;
};

uniform sampler2D colorSampler;
uniform sampler2D normalSampler;
uniform DirectionalLight directionalLight;
uniform vec3 eyePosition;
uniform Material material;

vec3 normalFromTex;

vec4 CalcLightByDirection(Light light, vec3 direction, vec3 normal)
{
	direction = normalize(direction);

	vec4 ambientColor = (light.ambient * light.color);
	
	float diffuseFactor = max(dot(normalize(normal), direction), 0);
	vec4 diffuseColor = (light.diffuse * light.color) * diffuseFactor;

	vec3 reflectVec = normalize(reflect(-direction, normalize(normal)));
	vec3 fragToEye = normalize(eyePosition - FragPos);

	float specularFactor = max(dot(reflectVec, fragToEye), 0);
	vec4 specularColor = (material.specular * light.color) * pow(specularFactor, material.shininess);

	return ambientColor + diffuseColor + specularColor;
}

vec4 CalcDirectionalLight(vec3 normal)
{
	return CalcLightByDirection(directionalLight.base, directionalLight.direction, normal);
}

void main()
{	
	//normalFromTex = texture(normalSampler, TexCoord).xyz;

	vec4 texColor = texture(colorSampler, TexCoord);
	vec4 dirLightColor = CalcDirectionalLight(FragNormal);

	FragColor = texColor * dirLightColor;
}