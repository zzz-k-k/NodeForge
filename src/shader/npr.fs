#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;

#define MAX_POINT_LIGHT 8
uniform int numPointLights;

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    float toonLevels;
    float outlineWidth;
    float shininess;
};

uniform Material material;

struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirLight dirLight;

struct PointLight
{
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform PointLight pointLights[MAX_POINT_LIGHT];

vec3 GetBaseColor()
{
    vec4 texColor = texture(material.texture_diffuse1, TexCoord);
    return clamp(texColor.rgb * material.albedo, 0.0, 1.0);
}

float QuantizeDiffuse(float value, float levels)
{
    float safeLevels = max(levels, 2.0);
    return floor(value * safeLevels) / (safeLevels - 1.0);
}

vec3 EvaluateToonLight(vec3 baseColor, vec3 normal, vec3 viewDir, vec3 lightDir, vec3 radiance)
{
    float diffuse = max(dot(normal, lightDir), 0.0);
    float steppedDiffuse = clamp(QuantizeDiffuse(diffuse, material.toonLevels), 0.0, 1.0);

    vec3 halfDir = normalize(viewDir + lightDir);
    float specular = pow(max(dot(normal, halfDir), 0.0), 64.0);
    float steppedSpecular = specular > 0.45 ? 1.0 : 0.0;

    vec3 diffuseColor = baseColor * steppedDiffuse * radiance;
    vec3 specularColor = steppedSpecular * 0.2 * radiance;
    return diffuseColor + specularColor;
}

void main()
{
    vec4 texColor = texture(material.texture_diffuse1, TexCoord);
    vec3 baseColor = GetBaseColor();
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 color = dirLight.ambient * baseColor * material.ao;

    vec3 dirLightDir = normalize(-dirLight.direction);
    color += EvaluateToonLight(baseColor, normal, viewDir, dirLightDir, dirLight.diffuse);

    for(int i = 0; i < numPointLights; ++i)
    {
        vec3 lightVector = pointLights[i].position - FragPos;
        float distance = length(lightVector);
        vec3 lightDir = normalize(lightVector);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance +
                                   pointLights[i].quadratic * distance * distance);
        vec3 radiance = pointLights[i].diffuse * attenuation;
        color += EvaluateToonLight(baseColor, normal, viewDir, lightDir, radiance);
    }

    FragColor = vec4(color, texColor.a);
}
