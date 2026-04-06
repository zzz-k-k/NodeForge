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
    vec3 shadowColor;
    vec3 outlineColor;
    vec3 rimLightColor;
    float metallic;
    float roughness;
    float ao;
    float toonLevels;
    float shadowThreshold;
    float specularThreshold;
    float rimLightIntensity;
    float rimLightWidth;
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
    return floor(value * (safeLevels - 1.0) + 0.5) / (safeLevels - 1.0);
}

float EvaluateToonBand(float diffuse)
{
    float threshold = clamp(material.shadowThreshold, 0.0, 0.95);
    if(diffuse <= threshold)
        return 0.0;

    float remapped = clamp((diffuse - threshold) / max(1.0 - threshold, 0.0001), 0.0, 1.0);
    float steppedDiffuse = QuantizeDiffuse(remapped, material.toonLevels);
    return mix(0.35, 1.0, steppedDiffuse);
}

vec3 EvaluateRimLight(vec3 normal, vec3 viewDir)
{
    float width = clamp(material.rimLightWidth, 0.02, 1.0);
    float rim = 1.0 - max(dot(normal, viewDir), 0.0);
    float rimMask = smoothstep(1.0 - width, 1.0, rim);
    return material.rimLightColor * rimMask * material.rimLightIntensity;
}

vec3 EvaluateToonLight(vec3 baseColor, vec3 normal, vec3 viewDir, vec3 lightDir, vec3 radiance)
{
    float diffuse = max(dot(normal, lightDir), 0.0);
    float toonBand = EvaluateToonBand(diffuse);
    vec3 shadedBase = mix(baseColor * material.shadowColor, baseColor, toonBand);

    vec3 halfDir = normalize(viewDir + lightDir);
    float specular = max(dot(normal, halfDir), 0.0);
    float steppedSpecular = smoothstep(
        clamp(material.specularThreshold, 0.0, 0.99),
        min(clamp(material.specularThreshold, 0.0, 0.99) + 0.03, 1.0),
        specular
    );
    steppedSpecular = steppedSpecular > 0.5 ? 1.0 : 0.0;

    vec3 diffuseColor = shadedBase * radiance;
    vec3 specularColor = steppedSpecular * 0.28 * radiance;
    return diffuseColor + specularColor;
}

void main()
{
    vec4 texColor = texture(material.texture_diffuse1, TexCoord);
    vec3 baseColor = GetBaseColor();
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 ambientBase = baseColor * material.shadowColor;
    vec3 color = max(dirLight.ambient * ambientBase * material.ao, ambientBase * 0.08);

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

    color += EvaluateRimLight(normal, viewDir);
    color = clamp(color, 0.0, 1.0);
    FragColor = vec4(color, texColor.a);
}
