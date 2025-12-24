#ifndef BUILD_H
#define BUILD_H

#include<vector>
#include<glm/glm.hpp>

#include<glad/glad.h>

#include<memory>
#include"model.h"

enum class ObjType{Cube,Light,Model};
struct SceneObject
{
    glm::mat4 model;
    int id=0;
    bool selected=false;
    ObjType type=ObjType::Cube;

    std::shared_ptr<Model> modelAsset;

};

struct CubeMesh
{
    unsigned int VAO=0,VBO=0;
    void Init()
    {
        float vertices[] = 
        {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,0.0f,  0.0f, -1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,0.0f,  0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,0.0f,  0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,0.0f,  0.0f, -1.0f,

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

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.0f,  0.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,1.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,0.0f, -1.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,0.0f,  1.0f,  0.0f
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
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

class BuildSystem
{
private:
    
public:
    std::vector<SceneObject> objects;
    int nextId=1;
    int selectedId=-1;
    CubeMesh cube;

    std::shared_ptr<Model> loadModel(const std::string& path)
    {
        return std::make_shared<Model>(path.c_str());
    }
};

#endif