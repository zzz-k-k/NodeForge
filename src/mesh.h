#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.h>

#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};
struct Texture
{
    unsigned int id;
    string type;
    string path;
};


class Mesh
{
    public:
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;
        unsigned int indexCount = 0;
        bool valid = false;
        Mesh(vector<Vertex> vertices,vector<unsigned int> indices,vector<Texture> textures)
        {
            this->vertices=vertices;
            this->indices=indices;
            this->textures=textures;
            this->indexCount = static_cast<unsigned int>(this->indices.size());

            setupMesh();
        }
        void Draw(Shader &shader)
        {
            //假如具有空节点，直接return
            if(!valid || VAO == 0 || indexCount == 0)
                return;

            unsigned int diffuseNr=1;
            unsigned int specularNr=1;
            for(unsigned int i=0;i<textures.size();i++)
            {
                glActiveTexture(GL_TEXTURE0+i);
                string number;
                string name=textures[i].type;
                if(name=="texture_diffuse")
                    number=std::to_string(diffuseNr++);
                if(name=="texture_specular")
                    number=std::to_string(specularNr++);

                shader.setInt(("material."+name+number).c_str(),i);
                glBindTexture(GL_TEXTURE_2D,textures[i].id);
            }
            glActiveTexture(GL_TEXTURE0);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,0);
            glBindVertexArray(0);
        }
    private:
        unsigned int VAO = 0, VBO = 0, EBO = 0;
        void setupMesh()
        {
            if(vertices.empty() || indices.empty())
            {
                valid = false;
                return;
            }

            glGenVertexArrays(1,&VAO);
            glGenBuffers(1,&VBO);
            glGenBuffers(1,&EBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER,VBO);

            glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned int),indices.data(),GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,Normal));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,TexCoords));

            glBindVertexArray(0);
            valid = true;
        }
};

#endif
