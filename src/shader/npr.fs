#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform bool useTextureAlpha;

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
    vec3 indirectLightMinColor;
    bool isFace;
    float metallic;
    float roughness;
    float ao;
    float toonLevels;
    float celShadeMidPoint;
    float celShadeSoftness;
    float directLightMultiplier;
    float additionalLightMultiplier;
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

float EvaluateCelShade(float noL)
{
    float midpoint = clamp(material.celShadeMidPoint, -1.0, 1.0);
    float softness = max(material.celShadeSoftness, 0.001);
    float litArea = smoothstep(midpoint - softness, midpoint + softness, noL);
    litArea = QuantizeDiffuse(litArea, material.toonLevels);

    if(material.isFace)
    {
        litArea = mix(0.5, 1.0, litArea);
    }

    return litArea;
}

vec3 EvaluateRimLight(vec3 normal, vec3 viewDir)
{
    float width = clamp(material.rimLightWidth, 0.02, 1.0);
    float rim = 1.0 - max(dot(normal, viewDir), 0.0);
    float rimMask = smoothstep(1.0 - width, 1.0, rim);
    return material.rimLightColor * rimMask * material.rimLightIntensity;
}

vec3 EvaluateToonLight(vec3 baseColor, vec3 normal, vec3 viewDir, vec3 lightDir, vec3 radiance, bool isAdditionalLight)
{
    float noL = dot(normal, lightDir);
    float litArea = EvaluateCelShade(noL) * clamp(material.ao, 0.0, 1.0);
    vec3 litOrShadowColor = mix(material.shadowColor, vec3(1.0), litArea);
    float lightMultiplier = isAdditionalLight ? material.additionalLightMultiplier : material.directLightMultiplier;
    vec3 lightColor = radiance * litOrShadowColor * lightMultiplier;

    vec3 halfDir = normalize(viewDir + lightDir);
    float specular = max(dot(normal, halfDir), 0.0);
    float steppedSpecular = smoothstep(
        clamp(material.specularThreshold, 0.0, 0.99),
        min(clamp(material.specularThreshold, 0.0, 0.99) + 0.03, 1.0),
        specular
    );
    steppedSpecular = steppedSpecular > 0.5 ? 1.0 : 0.0;

    vec3 diffuseColor = baseColor * lightColor;
    vec3 specularColor = steppedSpecular * 0.28 * radiance * lightMultiplier;
    return diffuseColor + specularColor;
}

void main()
{
    vec4 texColor = texture(material.texture_diffuse1, TexCoord);
    vec3 baseColor = GetBaseColor();
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 indirectLight = max(dirLight.ambient, material.indirectLightMinColor);
    vec3 color = baseColor * indirectLight * clamp(material.ao, 0.0, 1.0);

    vec3 dirLightDir = normalize(-dirLight.direction);
    color += EvaluateToonLight(baseColor, normal, viewDir, dirLightDir, dirLight.diffuse, false);

    for(int i = 0; i < numPointLights; ++i)
    {
        vec3 lightVector = pointLights[i].position - FragPos;
        float distance = length(lightVector);
        vec3 lightDir = normalize(lightVector);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance +
                                   pointLights[i].quadratic * distance * distance);
        vec3 radiance = pointLights[i].diffuse * attenuation;
        color += EvaluateToonLight(baseColor, normal, viewDir, lightDir, radiance, true);
    }

    color += EvaluateRimLight(normal, viewDir);
    color = clamp(color, 0.0, 1.0);
    float alpha = useTextureAlpha ? texColor.a : 1.0;
    FragColor = vec4(color, alpha);
}
