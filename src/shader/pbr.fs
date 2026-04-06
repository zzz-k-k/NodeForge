#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;

#define MAX_POINT_LIGHT 8
uniform int numPointLights;
const float PI = 3.14159265359;

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

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;
    return a2 / max(denominator, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float k = roughness + 1.0;
    k = (k * k) / 8.0;

    float denominator = NdotV * (1.0 - k) + k;
    return NdotV / max(denominator, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 GetBaseColor()
{
    vec4 texColor = texture(material.texture_diffuse1, TexCoord);
    return clamp(texColor.rgb * material.albedo, 0.0, 1.0);
}

vec3 EvaluateCookTorrance(vec3 baseColor, vec3 N, vec3 V, vec3 L, vec3 radiance)
{
    float metallic = clamp(material.metallic, 0.0, 1.0);
    float roughness = clamp(material.roughness, 0.05, 1.0);

    vec3 H = normalize(V + L);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    vec3 F = FresnelSchlick(HdotV, F0);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specular = numerator / max(denominator, 0.0001);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * baseColor / PI;

    return (diffuse + specular) * radiance * NdotL;
}

vec3 EvaluateDirectionalLight(vec3 baseColor, vec3 N, vec3 V)
{
    vec3 L = normalize(-dirLight.direction);
    vec3 radiance = max(dirLight.diffuse, dirLight.specular);
    return EvaluateCookTorrance(baseColor, N, V, L, radiance);
}

vec3 EvaluatePointLight(PointLight light, vec3 baseColor, vec3 N, vec3 V)
{
    vec3 lightVector = light.position - FragPos;
    float distance = length(lightVector);
    vec3 L = normalize(lightVector);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                               light.quadratic * distance * distance);
    vec3 radiance = max(light.diffuse, light.specular) * attenuation;
    return EvaluateCookTorrance(baseColor, N, V, L, radiance);
}

vec3 GetAmbientTerm(vec3 baseColor)
{
    vec3 sceneAmbient = max(dirLight.ambient, vec3(0.03));
    return sceneAmbient * baseColor * clamp(material.ao, 0.0, 1.0);
}

void main()
{
    vec4 texColor = texture(material.texture_diffuse1, TexCoord);
    vec3 baseColor = GetBaseColor();
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 color = GetAmbientTerm(baseColor);
    color += EvaluateDirectionalLight(baseColor, N, V);

    for(int i = 0; i < numPointLights; i++)
    {
        color += EvaluatePointLight(pointLights[i], baseColor, N, V);
    }

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, texColor.a);
}
