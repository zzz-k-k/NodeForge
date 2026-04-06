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

vec3 EvaluateLight(vec3 baseColor, vec3 normal, vec3 viewDir, vec3 lightDir, vec3 radiance)
{
    float roughness = clamp(material.roughness, 0.05, 1.0);
    float metallic = clamp(material.metallic, 0.0, 1.0);

    vec3 halfDir = normalize(viewDir + lightDir);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotH = max(dot(normal, halfDir), 0.0);
    float VdotH = max(dot(viewDir, halfDir), 0.0);

    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = max((NdotH * NdotH) * (alpha2 - 1.0) + 1.0, 0.0001);
    float distribution = alpha2 / (3.14159265 * denom * denom);

    float k = ((roughness + 1.0) * (roughness + 1.0)) / 8.0;
    float geometryV = NdotV / max(NdotV * (1.0 - k) + k, 0.0001);
    float geometryL = NdotL / max(NdotL * (1.0 - k) + k, 0.0001);
    float geometry = geometryV * geometryL;

    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    vec3 fresnel = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

    vec3 numerator = distribution * geometry * fresnel;
    float denominator = max(4.0 * NdotV * NdotL, 0.0001);
    vec3 specular = numerator / denominator;

    vec3 kS = fresnel;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * baseColor / 3.14159265;

    return (diffuse + specular) * radiance * NdotL;
}

void main()
{
    vec4 texColor = texture(material.texture_diffuse1, TexCoord);
    vec3 baseColor = GetBaseColor();
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 color = dirLight.ambient * baseColor * material.ao;

    vec3 dirLightDir = normalize(-dirLight.direction);
    vec3 dirRadiance = dirLight.diffuse + dirLight.specular;
    color += EvaluateLight(baseColor, normal, viewDir, dirLightDir, dirRadiance);

    for(int i = 0; i < numPointLights; ++i)
    {
        vec3 lightVector = pointLights[i].position - FragPos;
        float distance = length(lightVector);
        vec3 lightDir = normalize(lightVector);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance +
                                   pointLights[i].quadratic * distance * distance);
        vec3 radiance = (pointLights[i].diffuse + pointLights[i].specular) * attenuation;
        color += EvaluateLight(baseColor, normal, viewDir, lightDir, radiance);
    }

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, texColor.a);
}
