layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SCENE_TEXCOORD_ATTRIB_LOCATION)
in vec2 TexCoord;

layout(location = SCENE_NORMAL_ATTRIB_LOCATION)
in vec3 Normal;

in vec3 vertex_color;

out vec3 normal;
out vec3 vertPos;
out vec2 fragment_texcoord;
out vec4 shadowMapCoord;

uniform mat4 ModelWorld;
uniform mat4 ModelViewProjection;
uniform mat3 Normal_ModelWorld;
uniform mat4 lightMatrix;


void main()
{
    gl_Position = ModelViewProjection * Position;
    vertPos = vec3(ModelWorld * Position);
	normal = normalize(Normal_ModelWorld * Normal);
	fragment_texcoord = TexCoord;

	shadowMapCoord = lightMatrix * Position;
}