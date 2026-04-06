#ifndef BUILD_H
#define BUILD_H

#include<vector>
#include<glm/glm.hpp>

#include<glad/glad.h>

#include<memory>
#include<string>
#include"model.h"

#include <unordered_map>

enum class ObjType{Cube,Light,Model,Image};
enum class RenderMode{Phong,PBR,NPR};

inline const char* GetObjTypeLabel(ObjType type)
{
    switch(type)
    {
        case ObjType::Cube: return "Cube";
        case ObjType::Light: return "Light";
        case ObjType::Model: return "Model";
        case ObjType::Image: return "Image";
        default: return "Object";
    }
}

inline const char* GetRenderModeLabel(RenderMode mode)
{
    switch(mode)
    {
        case RenderMode::Phong: return "Phong";
        case RenderMode::PBR: return "PBR";
        case RenderMode::NPR: return "NPR";
        default: return "Phong";
    }
}

//材质结构体,0表示没有
struct Material
{
    RenderMode renderMode=RenderMode::Phong;
    std::string diffuseTexPath;
    std::string specularTexPath;
    std::string normalTexPath;
    glm::vec3 albedo=glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 shadowColor=glm::vec3(0.55f,0.62f,0.78f);
    glm::vec3 outlineColor=glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 rimLightColor=glm::vec3(1.0f,1.0f,1.0f);
    float metallic=0.0f;
    float roughness=0.5f;
    float ao=1.0f;
    float toonLevels=4.0f;
    float shadowThreshold=0.45f;
    float specularThreshold=0.6f;
    float rimLightIntensity=0.0f;
    float rimLightWidth=0.35f;
    float outlineWidth=0.05f;
    float shininess=32.0f;
};
struct SceneObject
{
    glm::mat4 model;
    int id=0;
    bool selected=false;
    ObjType type=ObjType::Cube;
    std::shared_ptr<Model> modelAsset;
    Material material;

};

struct CubeMesh
{
    unsigned int VAO=0,VBO=0;
    void Init()
    {
        float vertices[] = 
        {
             -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f, // 换上来
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f, // 换下去
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f, // 换上来
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f, // 换下去

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,0.0f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,0.0f,  0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,0.0f,  0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,0.0f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,0.0f,  0.0f, 1.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,-1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,-1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,-1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,-1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,-1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,-1.0f,  0.0f,  0.0f,

             // --- Right Face (修正为逆时针) ---
            // 交换了原来的第2、3行，以及第5、6行
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.0f,  0.0f,  0.0f, // 换上来
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,1.0f,  0.0f,  0.0f, // 换下去

            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.0f,  0.0f,  0.0f, // 换上来
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,1.0f,  0.0f,  0.0f, // 换下去

            // --- Bottom Face (本来就是逆时针，保持不变) ---
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,0.0f, -1.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,0.0f, -1.0f,  0.0f,

            // --- Top Face (修正为逆时针) ---
            // 交换了原来的第2、3行，以及第5、6行
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,0.0f,  1.0f,  0.0f, // 换上来
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,0.0f,  1.0f,  0.0f, // 换下去
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,0.0f,  1.0f,  0.0f, // 换上来
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,0.0f,  1.0f,  0.0f  // 换下去
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
    void Draw()
    {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES,0,36);
        glBindVertexArray(0);
    }
};

struct Quad
{
public:
    unsigned int VAO,VBO,EBO;
    void Init()
    {

        //手动计算切线和副切线
        #pragma region 手动计算切线和副切线
        //position
        glm::vec3 pos1(-1.0,1.0,0.0);
        glm::vec3 pos2(-1.0,-1.0,0.0);
        glm::vec3 pos3(1.0,-1.0,0.0);
        glm::vec3 pos4(1.0,1.0,0.0);
        //texture coordinates
        glm::vec2 uv1(0.0,1.0);
        glm::vec2 uv2(0.0,0.0);
        glm::vec2 uv3(1.0,0.0);
        glm::vec2 uv4(1.0,1.0);
        //normal vector
        glm::vec3 nm(0.0,0.0,1.0);

        glm::vec3 edge1=pos2-pos1;
        glm::vec3 edge2=pos3-pos1;
        glm::vec2 deltaUV1=uv2-uv1;
        glm::vec2 deltaUV2=uv3-uv1;

        //具体计算
        glm::vec3 tangent1,tangent2;
        glm::vec3 bitangent1,bitangent2;

        //tangent1
        GLfloat f=1.0f/(deltaUV1.x*deltaUV2.y-deltaUV2.x*deltaUV1.y);
        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);  

        //tangent2
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        #pragma endregion
        float vertices[] = 
        {
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        // 位置
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        // 法线
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
        // UV
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(GLfloat)));
        //切线
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,14*sizeof(GLfloat),(GLvoid*)(8*sizeof(GLfloat)));
        //副切线
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4,3,GL_FLOAT,GL_FALSE,14*sizeof(GLfloat),(GLvoid*)(11*sizeof(GLfloat)));
    }
    void Draw()
    {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES,0,6);
        glBindVertexArray(0);
    }
};
struct ScreenQuad
{
    public:
    unsigned int quadVAO,quadVBO;
    void Init()
    {
        float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
        };

        //screen quad vao
        glGenVertexArrays(1,&quadVAO);
        glGenBuffers(1,&quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER,quadVBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(quadVertices),&quadVertices,GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float)));

        //离屏渲染
        // glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
        // glClearColor(0.1f,0.1f,0.1f,1.0f);
        // glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        // glEnable(GL_DEPTH_TEST);
        
        //屏幕渲染
        // glBindFramebuffer(GL_FRAMEBUFFER,0);
        // glClearColor(1.0f,1.0f,1.0f,1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);

    }
    void Draw()
    {
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES,0,6);
    }
};


class BuildSystem
{
private:
    
public:
    std::vector<SceneObject> objects;
    int nextId=1;
    int selectedId=-1;
    CubeMesh cube;
    Quad quad;

    ScreenQuad screenquad;

    std::shared_ptr<Model> loadModel(const std::string& path)
    {
        return std::make_shared<Model>(path.c_str());
    }
    void ImportModel(const std::string& path)
    {
        SceneObject obj;
        obj.id=nextId++;
        obj.model=glm::mat4(1.0f);
        obj.selected=false;
        obj.type=ObjType::Model;
        obj.modelAsset=std::make_shared<Model>(path.c_str());
        obj.material.renderMode=RenderMode::Phong;
        obj.material.albedo=glm::vec3(1.0f,1.0f,1.0f);
        obj.material.shadowColor=glm::vec3(0.55f,0.62f,0.78f);
        obj.material.metallic=0.0f;
        obj.material.roughness=0.5f;
        obj.material.ao=1.0f;
        obj.material.toonLevels=4.0f;
        obj.material.shadowThreshold=0.45f;
        obj.material.specularThreshold=0.6f;
        obj.material.outlineColor=glm::vec3(0.0f,0.0f,0.0f);
        obj.material.rimLightColor=glm::vec3(1.0f,1.0f,1.0f);
        obj.material.rimLightIntensity=0.25f;
        obj.material.rimLightWidth=0.35f;
        obj.material.outlineWidth=0.05f;
        objects.push_back(obj);
    }
    void CreateImage(const char* imagePath)
    {
        SceneObject obj;
        obj.id=nextId++;
        obj.model=glm::mat4(1.0f);
        obj.selected=false;
        obj.type=ObjType::Image;
        obj.material.renderMode=RenderMode::Phong;
        obj.material.diffuseTexPath= imagePath;
        obj.material.specularTexPath = "./normalmap/brickwall.jpg";
        obj.material.normalTexPath = "normalmap/brickwall_normal.jpg";
        obj.material.albedo=glm::vec3(1.0f,1.0f,1.0f);
        obj.material.shadowColor=glm::vec3(0.55f,0.62f,0.78f);
        obj.material.metallic=0.0f;
        obj.material.roughness=0.9f;
        obj.material.ao=1.0f;
        obj.material.toonLevels=4.0f;
        obj.material.shadowThreshold=0.45f;
        obj.material.specularThreshold=0.55f;
        obj.material.outlineColor=glm::vec3(0.0f,0.0f,0.0f);
        obj.material.rimLightColor=glm::vec3(1.0f,1.0f,1.0f);
        obj.material.rimLightIntensity=0.2f;
        obj.material.rimLightWidth=0.3f;
        obj.material.outlineWidth=0.05f;
        objects.push_back(obj);
    }
    void CreateCube()
    {
        SceneObject obj;
        obj.id=nextId++;
        obj.model=glm::mat4(1.0f);
        obj.selected=false;
        obj.type=ObjType::Cube;
        obj.material.renderMode = RenderMode::Phong;
        obj.material.diffuseTexPath = "container2.png";
        obj.material.specularTexPath = "container2_specular.png";
        obj.material.normalTexPath = "";
        obj.material.albedo = glm::vec3(1.0f, 1.0f, 1.0f);
        obj.material.shadowColor = glm::vec3(0.55f, 0.62f, 0.78f);
        obj.material.metallic = 0.0f;
        obj.material.roughness = 0.35f;
        obj.material.ao = 1.0f;
        obj.material.toonLevels = 4.0f;
        obj.material.shadowThreshold = 0.45f;
        obj.material.specularThreshold = 0.6f;
        obj.material.outlineColor = glm::vec3(0.0f, 0.0f, 0.0f);
        obj.material.rimLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        obj.material.rimLightIntensity = 0.3f;
        obj.material.rimLightWidth = 0.35f;
        obj.material.outlineWidth = 0.05f;
        obj.material.shininess = 32.0f;
        objects.push_back(obj);
    }
    void CreateLight()
    {
        SceneObject obj;
        obj.id=nextId++;
        obj.model=glm::mat4(1.0f);
        obj.selected=false;
        obj.type=ObjType::Light;
        objects.push_back(obj);
    }
};

class TextureCache
{
private:
    std::unordered_map<std::string,unsigned int>cache;
    unsigned int defaultWhite=0;
    unsigned int defaultNormal=0;

public:
    void Init()
    {
        glGenTextures(1,&defaultWhite);
        glBindTexture(GL_TEXTURE_2D,defaultWhite);
        unsigned char white[] = {255, 255, 255, 255};   // RGBA 白色
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

        glGenTextures(1, &defaultNormal);
        glBindTexture(GL_TEXTURE_2D, defaultNormal);
        unsigned char normal[] = {128, 128, 255};        // RGB 法线默认
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, normal);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    }
    unsigned int Get(const std::string& path,bool srgb=false)
    {
        if(path.empty())
            return 0;

        auto it=cache.find(path);
        if(it !=cache.end())
            return it->second;

        unsigned int id=LoadTexture2D(path.c_str(),srgb);
        cache[path]=id;
        return id;
    }
    unsigned int GetDefault()
    {
        return defaultWhite;
    }
    unsigned int GetDefaultNormal()
    {
        return defaultNormal;
    }
    void Clear()
    {
        for(auto& pair:cache)
        {
            glDeleteTextures(1,&pair.second);
        }
        cache.clear();
    }
    //加载纹理对象
    //添加参数是否使用srgb
    unsigned int LoadTexture2D(const char* path,bool useSrgb)
    {
        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[]={1.0,1.0,1.0,1.0};
        glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderColor);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int w, h, c;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path, &w, &h, &c, 0);
        if (data)
        {
            if(useSrgb)
            {
                GLenum format = (c == 4) ? GL_RGBA : GL_RGB;
                GLenum internalFormat=(c==4)?GL_SRGB_ALPHA:GL_SRGB;
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                GLenum format = (c == 4) ? GL_RGBA : GL_RGB;
                glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }
        else
        {
            std::cout << "Failed to load texture: " << path
                    << " reason: " << stbi_failure_reason() << std::endl;
        }
        stbi_image_free(data);

        return tex;
    }
};

#endif
