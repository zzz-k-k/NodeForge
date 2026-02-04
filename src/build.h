#ifndef BUILD_H
#define BUILD_H

#include<vector>
#include<glm/glm.hpp>

#include<glad/glad.h>

#include<memory>
#include<string>
#include"model.h"

enum class ObjType{Cube,Light,Model,Image};
struct SceneObject
{
    glm::mat4 model;
    int id=0;
    bool selected=false;
    ObjType type=ObjType::Cube;

    std::shared_ptr<Model> modelAsset;

    std::string texturePath;
    unsigned int textureID; 

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
        // float vertices[]=
        // {
        //     0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        //     0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
        //     1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

        //     0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        //     1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
        //     1.0f,  0.5f,  0.0f,  1.0f,  0.0f
        // };
        float vertices[] = 
        {
            // 位置              法线              UV
            -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // 左上
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // 左下
            0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,  // 右下
            -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // 左上
            0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,  // 右下
            0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f   // 右上
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // glEnableVertexAttribArray(0);
        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        // glEnableVertexAttribArray(1);
        // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        // glBindVertexArray(0);
        
        // 位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // 法线
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // UV
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
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
        objects.push_back(obj);
    }
    void CreateImage(const char* imagePath)
    {
        SceneObject obj;
        obj.id=nextId++;
        obj.model=glm::mat4(1.0f);
        obj.selected=false;
        obj.type=ObjType::Image;
        obj.texturePath = imagePath;
        obj.textureID = 0; 
        objects.push_back(obj);
    }
};

#endif