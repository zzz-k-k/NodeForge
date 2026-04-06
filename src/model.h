#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/config.h>

#include <mesh.h>
#include <shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);
vector<Texture> textures_loaded;

class Model
{
    public:
        Model(const char *path)
        {
            loadModel(path);
        }
        void Draw(Shader &shader)
        {
            for(unsigned int i=0;i<meshes.size();i++)
                meshes[i].Draw(shader);
        }

    private:
        vector<Mesh> meshes;
        string directory;
        void loadModel(string path)
        {
            Assimp::Importer import;
            import.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_WEIGHTS, false);
            import.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, false);

            const unsigned int importFlags =
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_JoinIdenticalVertices |
                aiProcess_GenSmoothNormals;

            const aiScene *scene=import.ReadFile(path,importFlags);

            if(!scene||scene->mFlags&AI_SCENE_FLAGS_INCOMPLETE||!scene->mRootNode)
            {
                cout<<"error::assimp::"<<import.GetErrorString()<<endl;
                return;
            }
            size_t separatorPos = path.find_last_of("/\\");
            directory = (separatorPos == std::string::npos) ? "." : path.substr(0, separatorPos);

            processNode(scene->mRootNode,scene);
        }
        void processNode(aiNode *node,const aiScene *scene)
        {
            for(unsigned int i=0;i<node->mNumMeshes;i++)
            {
                aiMesh *mesh=scene->mMeshes[node->mMeshes[i]];
                if(mesh == nullptr)
                {
                    std::cout << "warning::assimp::skip null mesh in node" << std::endl;
                    continue;
                }

                Mesh processedMesh = processMesh(mesh,scene);
                if(processedMesh.valid)
                {
                    meshes.push_back(std::move(processedMesh));
                }
            }
            for(unsigned int i=0;i<node->mNumChildren;i++)
            {
                processNode(node->mChildren[i],scene);
            }
        }
        Mesh processMesh(aiMesh *mesh,const aiScene *scene)
        {
            vector<Vertex> vertices;
            vector<unsigned int> indices;
            vector<Texture> textures;

            if(mesh->mNumVertices == 0)
            {
                std::cout << "warning::assimp::skip mesh with zero vertices: "
                          << mesh->mName.C_Str() << std::endl;
                return Mesh(vertices,indices,textures);
            }

            for(unsigned int i=0;i<mesh->mNumVertices;i++)
            {
                Vertex vertex;

                glm::vec3 vector; 
                vector.x = mesh->mVertices[i].x;
                vector.y = mesh->mVertices[i].y;
                vector.z = mesh->mVertices[i].z; 
                vertex.Position = vector;

                if(mesh->HasNormals())
                {
                    vector.x = mesh->mNormals[i].x;
                    vector.y = mesh->mNormals[i].y;
                    vector.z = mesh->mNormals[i].z;
                    vertex.Normal = vector;
                }
                else
                {
                    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                if(mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
                {
                    glm::vec2 vec;
                    vec.x = mesh->mTextureCoords[0][i].x; 
                    vec.y = mesh->mTextureCoords[0][i].y;
                    vertex.TexCoords = vec;
                }
                else
                    vertex.TexCoords = glm::vec2(0.0f, 0.0f);

                vertices.push_back(vertex);
            }

            for(unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for(unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }

            if(indices.empty())
            {
                std::cout << "warning::assimp::skip mesh with zero indices: "
                          << mesh->mName.C_Str() << std::endl;
                return Mesh(vertices,indices,textures);
            }

            if(mesh->mMaterialIndex>=0)
            {
                if(mesh->mMaterialIndex < scene->mNumMaterials && scene->mMaterials[mesh->mMaterialIndex] != nullptr)
                {
                    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                    vector<Texture> diffuseMaps = loadMaterialTextures(material, 
                                                        aiTextureType_DIFFUSE, "texture_diffuse", true);
                    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
                    vector<Texture> specularMaps = loadMaterialTextures(material, 
                                                        aiTextureType_SPECULAR, "texture_specular", false);
                    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
                }
                else
                {
                    std::cout << "warning::assimp::skip invalid material index on mesh: "
                              << mesh->mName.C_Str() << std::endl;
                }
            }
            return Mesh(vertices,indices,textures);
        }
        vector<Texture> loadMaterialTextures(aiMaterial *mat,aiTextureType type,string typeName,bool gamma = false)
        {
            vector<Texture> textures;
            for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
            {
                aiString str;
                mat->GetTexture(type, i, &str);
                bool skip = false;
                for(unsigned int j = 0; j < textures_loaded.size(); j++)
                {
                    if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                    {
                        textures.push_back(textures_loaded[j]);
                        skip = true; 
                        break;
                    }
                }
                if(!skip)
                {   // 如果纹理还没有被加载，则加载它
                    textures.push_back({TextureFromFile(str.C_Str(), directory, gamma), typeName, str.C_Str()});
                    textures_loaded.push_back(textures.back()); // 添加到已加载的纹理中
                }
            }
            return textures;
        }
};

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
    stbi_set_flip_vertically_on_load(true);

    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        GLenum internalFormat;
        if (nrComponents == 1)
        {
            format = GL_RED;
            internalFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            format = GL_RGB;
            internalFormat = gamma ? GL_SRGB : GL_RGB;
        }
        else if (nrComponents == 4)
        {
            format = GL_RGBA;
            internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
        }
        else
        {
            format = GL_RGB;
            internalFormat = gamma ? GL_SRGB : GL_RGB;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

#endif
