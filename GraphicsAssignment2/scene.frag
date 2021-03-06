uniform vec3 CameraPos;

uniform vec3 Ambient;
uniform vec3 Diffuse;
uniform vec3 Specular;
uniform float Shininess;

uniform int HasDiffuseMap;
uniform sampler2D DiffuseMap;
uniform sampler2DShadow ShadowMap;

in vec3 normal;
in vec3 vertPos;
in vec2 fragment_texcoord;
in vec4 shadowMapCoord;

out vec4 FragColor;

const vec3 ambientColor = vec3(0.1, 0.1, 0.1);

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
	
	vec3 colorLinear = lambertian * diffuseMap + specular * Specular;

	float screenGamma = 2.2;
	vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0 / screenGamma));

	float visibility = textureProj(ShadowMap, shadowMapCoord);
	FragColor = vec4(colorGammaCorrected * visibility +  Ambient * ambientColor * colorGammaCorrected, 1.0);
}