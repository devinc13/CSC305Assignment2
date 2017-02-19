uniform vec3 CameraPos;

uniform vec3 Ambient;
uniform vec3 Diffuse;
uniform vec3 Specular;
uniform float Shininess;

uniform int HasDiffuseMap;
uniform sampler2D DiffuseMap;

in vec3 normal;
in vec3 vertPos;
in vec2 fragment_texcoord;

out vec4 FragColor;

const vec3 ambientColor = vec3(0.0, 0.0, 0.1);

void main()
{
	vec3 lightDir = normalize(CameraPos - vertPos);

	float lambertian = max(dot(lightDir, normal), 0.0);
	float specular = 0.0;

	if (lambertian > 0.0) {
		vec3 viewDir = normalize(-vertPos);
		vec3 halfDir = normalize(lightDir + viewDir);
		float specAngle = max(dot(halfDir, normal), 0.0);
		specular = pow(specAngle, Shininess);
	}

	vec3 diffuseMap;
	if (HasDiffuseMap != 0) {
		diffuseMap = texture(DiffuseMap, fragment_texcoord).rgb;
	} else {
		diffuseMap = Diffuse;
	}
	
	vec3 colorLinear = Ambient * ambientColor + lambertian * diffuseMap + specular * Specular;

	float screenGamma = 2.2;
	vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0 / screenGamma));

	FragColor = vec4(colorGammaCorrected, 1.0);
}