#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;

in vec3 FragPos;

uniform vec3 viewPos;

//光源数组数量控制
#define MAX_POINT_LIGHT 8
uniform int numPointLights;


struct Material{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

uniform Material material;

struct Light {
    vec3 position;
    vec3  direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Light light;

//定向光
struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

uniform PointLight pointLights[MAX_POINT_LIGHT];

vec3 CalcDirLight(DirLight light,vec3 normal,vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // 属性
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 第一阶段：定向光照
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // 第二阶段：点光源
    for(int i = 0; i < numPointLights; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
    // 第三阶段：聚光
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    

    FragColor = vec4(result, 1.0);
    
}

vec3 CalcDirLight(DirLight light,vec3 normal,vec3 viewDir)
{
    vec3 lightDir=normalize(-light.direction);
    //漫反射着色
    float diff=max(dot(normal,lightDir),0.0);
    //镜面光着色
    vec3 reflectDir=reflect(-lightDir,normal);
    float spec=pow(max(dot(viewDir,reflectDir),0.0),material.shininess);
    //合并
    vec3 ambient=dirLight.ambient*vec3(texture(material.texture_diffuse1,TexCoord));
    vec3 diffuse=dirLight.diffuse*diff*vec3(texture(material.texture_diffuse1,TexCoord));
    vec3 specular = dirLight.specular * spec * vec3(texture(material.texture_specular1, TexCoord));
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 衰减
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));    
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, TexCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoord));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}