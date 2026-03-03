#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;

in vec3 FragPos;

//阴影
in vec4 FragPosLightSpace;

//TBN矩阵
in mat3 TBN;

//点光源阴影
uniform samplerCube depthMap;
uniform float far_plane;

uniform vec3 viewPos;

uniform bool blinn;

uniform sampler2D shadowMap;

//光源数组数量控制
#define MAX_POINT_LIGHT 8
uniform int numPointLights;

//法线贴图
uniform sampler2D normalMap;


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

vec3 CalcDirLight(DirLight light,vec3 normal,vec3 viewDir,mat3 TBN);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,mat3 TBN);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    //透视除法
    vec3 projCoords=fragPosLightSpace.xyz/fragPosLightSpace.w;
    projCoords=projCoords*0.5+0.5;
    float closestDepth=texture(shadowMap,projCoords.xy).r;
    float currentDepth=projCoords.z;

    float bias=max(0.05*(1.0-dot(normal,lightDir)),0.005);
    
    //paf实现采样深度贴图周边纹理像素并取平均值
    float shadow=0.0;
    vec2 texelSize=1.0/textureSize(shadowMap,0);
    for(int x=-1;x<=1;++x)
    {
        for(int y=-1;y<=1;++y)
        {
            float pcfDepth=texture(shadowMap,projCoords.xy+vec2(x,y)*texelSize).r;
            shadow+=currentDepth-bias>pcfDepth?1.0:0.0;
        }
    }
    shadow/=9;

    if(projCoords.z>1.0||projCoords.x>1.0||projCoords.y>1.0)
        shadow=0.0;

    return shadow;
}

float DirShadowCalculation(vec3 fragPos,vec3 lightPos)
{
    vec3 fragToLight=fragPos-lightPos;
    float closestDepth=texture(depthMap,fragToLight).r;
    closestDepth*=far_plane;
    float currentDepth=length(fragToLight);
    float bias=0.05;
    float shadow=currentDepth-bias>closestDepth?1.0:0.0;
    return shadow;
}

void main()
{
    //获取alpha
    vec4 texColor = texture(material.texture_diffuse1, TexCoord);
    float alpha = texColor.a;

    // 属性
    //vec3 norm = normalize(Normal);
    vec3 normal = texture(normalMap,TexCoord).rgb;
    // 将法线向量转换为范围[-1,1]
    vec3 norm = normalize(normal * 2.0 - 1.0);  
    //处理tbn
    vec3 tangentViewPos=TBN*viewPos;
    vec3 tangentFragPos=TBN*FragPos;
    
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);

    // 第一阶段：定向光照
    //vec3 tangentDirLight=TBN*dirLight.direction;
    vec3 result = CalcDirLight(dirLight, norm, viewDir,TBN);
    // 第二阶段：点光源
    for(int i = 0; i < numPointLights; i++)
    {
        //vec3 tangentposition=TBN*pointLights[i].position;
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir,TBN);    
    }
    // 第三阶段：聚光
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    

    FragColor = vec4(result, alpha);
    
}

//环境光和阴影
vec3 CalcDirLight(DirLight light,vec3 normal,vec3 viewDir,mat3 TBN)
{
    vec3 lightDir=normalize(-light.direction);
    vec3 tangentDirLight=TBN*lightDir;
    //漫反射着色
    float diff=max(dot(normal,tangentDirLight),0.0);
    //镜面光着色
    vec3 reflectDir=reflect(-tangentDirLight,normal);
    float spec=pow(max(dot(viewDir,reflectDir),0.0),material.shininess);
    //计算阴影
    float shadow=ShadowCalculation(FragPosLightSpace, normal, lightDir);
    //合并
    vec3 ambient=dirLight.ambient*vec3(texture(material.texture_diffuse1,TexCoord));
    vec3 diffuse=dirLight.diffuse*diff*vec3(texture(material.texture_diffuse1,TexCoord));
    vec3 specular = dirLight.specular * spec * vec3(texture(material.texture_specular1, TexCoord));
    return (ambient + (1.0-shadow)*(diffuse + specular));
}

//点光源和阴影
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,mat3 TBN)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 tangentDirLight=TBN*lightDir;
    // 漫反射着色
    float diff = max(dot(normal, tangentDirLight), 0.0);
    // 镜面光着色
    float spec=0.0;
    if(blinn)
    {
        vec3 halfwayDir=normalize(tangentDirLight+viewDir);
        spec=pow(max(dot(normal,halfwayDir),0.0),material.shininess);
    }
    else
    {
        vec3 reflectDir = reflect(-tangentDirLight, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
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

    //阴影
    float shadow=DirShadowCalculation(FragPos,light.position);

    return (ambient + (1.0-shadow)*(diffuse + specular));
}
