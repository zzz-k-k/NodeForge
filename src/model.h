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
#include <assimp/material.h>

#include <mesh.h>
#include <shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <map>
#include <vector>
using namespace std;
namespace fs = std::filesystem;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);
unsigned int CreateSolidColorTexture(const glm::vec4& color, bool gamma = false);
std::string FindTextureByMaterialName(const std::string& materialName, const std::string& directory, const std::vector<std::string>& suffixes);
std::vector<unsigned char> ReadBinaryFileBytes(const std::string& path);
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
                    aiString materialName;
                    material->Get(AI_MATKEY_NAME, materialName);

                    aiColor4D baseColor(1.0f, 1.0f, 1.0f, 1.0f);
                    const bool hasBaseColorFactor =
                        material->Get(AI_MATKEY_BASE_COLOR, baseColor) == aiReturn_SUCCESS ||
                        material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == aiReturn_SUCCESS;

                    vector<Texture> diffuseMaps = loadMaterialTextures(material, 
                                                        aiTextureType_BASE_COLOR, "texture_diffuse", true);
                    if(diffuseMaps.empty())
                    {
                        diffuseMaps = loadMaterialTextures(material,
                                                            aiTextureType_DIFFUSE, "texture_diffuse", true);
                    }
                    if(diffuseMaps.empty())
                    {
                        const std::string fallbackDiffusePath = FindTextureByMaterialName(
                            materialName.C_Str(), directory, {"_D", "_BaseColor", "_BC"});
                        if(!fallbackDiffusePath.empty())
                        {
                            Texture fallbackDiffuse;
                            fallbackDiffuse.id = TextureFromFile(fallbackDiffusePath.c_str(), directory, true);
                            fallbackDiffuse.type = "texture_diffuse";
                            fallbackDiffuse.path = fallbackDiffusePath;
                            diffuseMaps.push_back(fallbackDiffuse);
                        }
                    }
                    if(diffuseMaps.empty())
                    {
                        if(hasBaseColorFactor)
                        {
                            Texture solidDiffuse;
                            solidDiffuse.id = CreateSolidColorTexture(
                                glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a), true);
                            solidDiffuse.type = "texture_diffuse";
                            solidDiffuse.path = "__solid_diffuse__" + std::string(mesh->mName.C_Str());
                            diffuseMaps.push_back(solidDiffuse);
                        }
                    }
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
                if(mat->GetTexture(type, i, &str) != aiReturn_SUCCESS)
                {
                    continue;
                }

                const char* texturePath = str.C_Str();
                if(texturePath == nullptr || texturePath[0] == '\0')
                {
                    continue;
                }

                bool skip = false;
                for(unsigned int j = 0; j < textures_loaded.size(); j++)
                {
                    if(textures_loaded[j].path == texturePath)
                    {
                        textures.push_back(textures_loaded[j]);
                        skip = true; 
                        break;
                    }
                }
                if(!skip)
                {   // 如果纹理还没有被加载，则加载它
                    textures.push_back({TextureFromFile(texturePath, directory, gamma), typeName, texturePath});
                    textures_loaded.push_back({TextureFromFile(texturePath, directory, gamma), typeName, texturePath}); // 添加到已加载的纹理中
                }
            }
            return textures;
        }
};

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
    stbi_set_flip_vertically_on_load(true);

    auto fileExists = [](const fs::path& candidate) -> bool
    {
        std::error_code errorCode;
        return fs::exists(candidate, errorCode) && fs::is_regular_file(candidate, errorCode);
    };

    auto resolveTexturePath = [&](const char* texturePath) -> std::string
    {
        fs::path rawPath(texturePath);
        if(rawPath.is_absolute() && fileExists(rawPath))
            return rawPath.string();

        fs::path modelDirectory(directory);
        std::vector<fs::path> candidates;
        candidates.push_back(modelDirectory / rawPath);
        candidates.push_back(modelDirectory / rawPath.filename());

        const fs::path modelParent = modelDirectory.parent_path();
        if(!modelParent.empty())
        {
            candidates.push_back(modelParent / rawPath);
            candidates.push_back(modelParent / rawPath.filename());
            candidates.push_back(modelParent / "Textures" / rawPath);
            candidates.push_back(modelParent / "Textures" / rawPath.filename());
        }

        for(const auto& candidate : candidates)
        {
            if(fileExists(candidate))
                return candidate.string();
        }

        const fs::path searchRoot = modelParent.empty() ? modelDirectory : modelParent;
        std::error_code errorCode;
        for(const auto& entry : fs::recursive_directory_iterator(searchRoot, fs::directory_options::skip_permission_denied, errorCode))
        {
            if(errorCode)
                break;
            if(!entry.is_regular_file())
                continue;
            if(entry.path().filename() == rawPath.filename())
                return entry.path().string();
        }

        return (modelDirectory / rawPath.filename()).string();
    };

    string filename = resolveTexturePath(path);

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    const std::vector<unsigned char> fileBytes = ReadBinaryFileBytes(filename);
    unsigned char *data = nullptr;
    if(!fileBytes.empty())
    {
        data = stbi_load_from_memory(fileBytes.data(),
                                     static_cast<int>(fileBytes.size()),
                                     &width,
                                     &height,
                                     &nrComponents,
                                     0);
    }
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
        std::cout << "Texture failed to load at resolved path: " << filename
                  << " source: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int CreateSolidColorTexture(const glm::vec4& color, bool gamma)
{
    unsigned int textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    auto toByte = [](float value) -> unsigned char
    {
        const float clamped = glm::clamp(value, 0.0f, 1.0f);
        return static_cast<unsigned char>(clamped * 255.0f + 0.5f);
    };

    unsigned char pixel[] = {
        toByte(color.r),
        toByte(color.g),
        toByte(color.b),
        toByte(color.a)
    };

    const GLenum internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

std::string FindTextureByMaterialName(const std::string& materialName, const std::string& directory, const std::vector<std::string>& suffixes)
{
    if(materialName.empty())
        return "";

    fs::path modelDirectory(directory);
    fs::path searchRoot = modelDirectory.parent_path();
    if(searchRoot.empty())
        searchRoot = modelDirectory;

    std::string textureStem = materialName;
    if(textureStem.rfind("MI_", 0) == 0)
        textureStem.replace(0, 3, "T_");
    else if(textureStem.rfind("M_", 0) == 0)
        textureStem.replace(0, 2, "T_");

    const std::vector<std::string> extensions = {".png", ".tga", ".jpg", ".jpeg", ".dds"};
    for(const auto& suffix : suffixes)
    {
        for(const auto& extension : extensions)
        {
            fs::path candidate = searchRoot / "Textures" / (textureStem + suffix + extension);
            std::error_code errorCode;
            if(fs::exists(candidate, errorCode) && fs::is_regular_file(candidate, errorCode))
                return candidate.string();
        }
    }

    return "";
}

std::vector<unsigned char> ReadBinaryFileBytes(const std::string& path)
{
    std::ifstream file(fs::path(path), std::ios::binary);
    if(!file)
        return {};

    file.seekg(0, std::ios::end);
    const std::streampos endPos = file.tellg();
    if(endPos <= 0)
    {
        return {};
    }

    std::vector<unsigned char> buffer(static_cast<size_t>(endPos));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
    if(!file)
        return {};

    return buffer;
}

#endif
