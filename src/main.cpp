#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include<iostream>
#include<cmath>
#include<stb_image.h>

#include<ui.h>
#include<grid.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <camera.h>
#include<shader.h>
#include<model.h>
#include<build.h>
#include<raycaster.h>
#include "ImGuizmo.h"

#include<algorithm>
#include<vector>

void BindDefaultSurfaceTextures();
void BindObjectMaterialTextures(const SceneObject& obj);
void ApplySurfaceMaterialUniforms(Shader& shader,const SceneObject& obj,bool enableNormalMap);
void ApplyCommonSurfaceUniforms(Shader& shader,const glm::mat4& view,const glm::mat4& projection,const glm::mat4& lightSpaceMatrix,const glm::vec3& shadowCasterPos);
Shader& SelectSurfaceShader(const SceneObject& obj,Shader& phongShader,Shader& pbrShader,Shader& nprShader);
void DrawSceneObjectGeometry(Shader& shader,SceneObject& obj);
void drawOpaqueObject(Shader& lampShader,Shader& phongShader,Shader& pbrShader,Shader& nprShader,const glm::mat4& view,const glm::mat4& projection);
void drawDepthObjects(Shader& depthShader);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void drop_callback(GLFWwindow* window,int count, const char** paths);
unsigned int loadCubemap(vector<std::string> faces);
void resizeFramebufferAttachments(int width, int height);

float deltaTime = 0.0f;
float lastFrame = 0.0f; 
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
UI ui;
Grid grid;
BuildSystem build;
TextureCache TexCache;
Raycaster raycaster;

ImGuizmo::OPERATION gizmoOp=ImGuizmo::TRANSLATE;
ImGuizmo::MODE gizmoMode=ImGuizmo::WORLD;

static bool wireframe = false;

glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 pos =glm::vec3(0.0f,0.0f,0.0f);

//鼠标初始位置
float lastX = 400, lastY = 300;
bool firstMouse=true;

float cameraControl=false;

//灯的位置
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

static unsigned int gFBO = 0;
static unsigned int gColorTex = 0;
static unsigned int gRBO = 0;
static int gFbWidth = 0;
static int gFbHeight = 0;

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};



int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    stbi_set_flip_vertically_on_load(true);

    //初始化默认图形
    build.cube.Init();
    build.quad.Init();

    TexCache.Init();

    build.screenquad.Init();

    glm::mat4 trans;

    //设置灯的数据
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // 只需要绑定VBO不用再次设置VBO的数据，因为箱子的VBO数据中已经包含了正确的立方体顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, build.cube.VBO);
    // 设置灯立方体的顶点属性（对我们的灯来说仅仅只有位置数据）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    #pragma region 帧缓冲
    //创建一个帧缓冲
    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    unsigned int fbo;
    glGenFramebuffers(1,&fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    gFBO = fbo;

    //创建颜色纹理附件
    unsigned int colorTex;
    glGenTextures(1,&colorTex);
    glBindTexture(GL_TEXTURE_2D,colorTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,fbWidth,fbHeight,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,colorTex,0);
    gColorTex = colorTex;


    //glDeleteFramebuffers(1,&fbo);//删除帧缓冲对象

    //为帧缓冲创建深度纹理
    // unsigned int texture;
    // glGenTextures(1,&texture);
    // glBindTexture(GL_TEXTURE_2D,texture);
    // glTexImage2D(
    //     GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, 
    //     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL
    // );
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // //将纹理附加到帧缓冲上
    // glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_TEXTURE_2D,texture,0);

    #pragma endregion

    #pragma region 渲染缓冲对象
    unsigned int rbo;
    glGenRenderbuffers(1,&rbo);
    glBindRenderbuffer(GL_RENDERBUFFER,rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbWidth, fbHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    gRBO = rbo;
    
    //检查帧缓冲是否完整
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        std::cout<<"ERROR::FRAMEBUFFER::framebuffer is not complete"<<std::endl;
        
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    resizeFramebufferAttachments(fbWidth, fbHeight);
    #pragma endregion

    #pragma region 深度贴图
    unsigned int depthMapFBO;
    glGenFramebuffers(1,&depthMapFBO);
    //创建2d纹理提供给帧缓冲的深度缓冲使用
    const unsigned int SHADOW_WIDTH=1024,SHADOW_HEIGHT=1024;
    unsigned int depthMap;
    glGenTextures(1,&depthMap);
    glBindTexture(GL_TEXTURE_2D,depthMap);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,SHADOW_WIDTH,SHADOW_HEIGHT,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    
    //将生成的深度纹理作为帧缓冲的深度缓冲
    glCullFace(GL_FRONT);
    glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depthMap,0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glCullFace(GL_BACK);
    #pragma endregion

    #pragma region 立方体贴图
    //纹理加载
    vector<std::string> faces
    {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg"
    };
    stbi_set_flip_vertically_on_load(false);
    unsigned int cubemapTexture = loadCubemap(faces);
    stbi_set_flip_vertically_on_load(true);
    //创建立方体贴图
    unsigned int textureID;
    glGenTextures(1,&textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP,textureID);

    //遍历整个纹理目标
    int width,height,nrChannels;
    unsigned char *data;

    //遍历六个面纹理
    //faces是六个面的纹理组
    for(unsigned int i =0;i<faces.size();i++)
    {
        data =stbi_load(faces[i].c_str(),&width,&height,&nrChannels,0);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,
            0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data
        );
    }


    #pragma endregion
    
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader ourShader("../../src/shader/shader.vs", "../../src/shader/shader.fs");
    Shader pbrShader("../../src/shader/shader.vs", "../../src/shader/pbr.fs");
    Shader nprShader("../../src/shader/shader.vs", "../../src/shader/npr.fs");
    Shader lampShader("../../src/shader/lightShader.vs","../../src/shader/lightShader.fs");
    Shader gridShader("../../src/gridshader/grid.vs",
                  "../../src/gridshader/grid.fs");
    Shader framebuffers("../../src/shader/framebuffers.vs","../../src/shader/framebuffers.fs");
    Shader skyboxShader("../../src/shader/cubeshader.vs","../../src/shader/cubeshader.fs");
    Shader depthShader("../../src/shader/depthshader.vs","../../src/shader/depthshader.fs");

    Shader pointdepthShader("../../src/shader/depth/pointdepthshader.vs","../../src/shader/depth/pointdepthshader.fs","../../src/shader/depth/pointdepthshader.gs");
    
    ui.UIinit(window);

    Grid grid;
    grid.init(gridShader, 50.0f);

    //启用混合并设置对应的混合函数
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //启用深度缓冲
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0x00);

    //启用面剔除
    glEnable(GL_CULL_FACE);
    

    glm::mat4 view;
    view=glm::lookAt(glm::vec3(0.0f,0.0f,3.0f),
                     glm::vec3(0.0f,0.0f,0.0f),
                     glm::vec3(0.0f,1.0f,0.0f));

    float radius = 10.0f;
    
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    //有文件拖拽时
    glfwSetDropCallback(window, drop_callback);

    //------------创建生成纹理对象---------
    // unsigned int diffuseMap = LoadTexture2D("container2.png",true);
    // unsigned int specularMap = LoadTexture2D("./normalmap/brickwall.jpg",false);
    // unsigned int normalMap=LoadTexture2D("normalmap/brickwall_normal.jpg",false);

    //skyboxvao,skyboxvbo
    unsigned int skyboxVAO,skyboxVBO;
    glGenVertexArrays(1,&skyboxVAO);
    glGenBuffers(1,&skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER,skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(skyboxVertices),&skyboxVertices,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glBindVertexArray(0);
    
    //光源空间的变换矩阵
    float near_plane=1.0f,far_plane=7.5f;
    glm::mat4 lightProjection=glm::ortho(-10.0f,10.0f,-10.0f,10.0f,near_plane,far_plane);
    glm::mat4 lightview=glm::lookAt(
        glm::vec3(-2.0f,4.0f,-1.0f),
        glm::vec3(0.0f,0.0f,0.0f),
        glm::vec3(0.0f,1.0f,0.0f)
    );
    glm::mat4 lightSpaceMatrix=lightProjection*lightview;
    
    //点光源空间的变换矩阵
    GLfloat aspect=(GLfloat)SHADOW_WIDTH/(GLfloat)SHADOW_HEIGHT;
    GLfloat near=1.0f;
    GLfloat far=25.0f;
    glm::mat4 shadowProj=glm::perspective(glm::radians(90.0f),aspect,near,far);
    
    //生成深度立方体贴图
    GLuint depthCubemap;
    unsigned depthMapFBO_point;
    glGenFramebuffers(1,&depthMapFBO_point);
    glGenTextures(1,&depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP,depthCubemap);
    for(GLuint i=0;i<6;++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_DEPTH_COMPONENT,
            SHADOW_WIDTH,SHADOW_WIDTH,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO_point);
    glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,depthCubemap,0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    
    while(!glfwWindowShouldClose(window))
    {
        //检查是否按下右键
        bool rmbDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        if (rmbDown && !cameraControl)
        {
            cameraControl = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // 关键：防止第一次进入时镜头“猛跳”
        }
        else if (!rmbDown && cameraControl)
        {
            cameraControl = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        ImGuiIO& io=ImGui::GetIO();
        if(!io.WantCaptureKeyboard)
        {
            processInput(window);
        }

        glm::mat4 view = camera.GetViewMatrix(); 
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);

        //灯的模型
        glm::mat4 model=glm::mat4();
        model=glm::translate(model,lightPos);
        model=glm::scale(model,glm::vec3(0.2f));

        lampShader.use();

        lampShader.setMat4("view",view);
        lampShader.setMat4("projection",projection);
        lampShader.setMat4("model",model);
        
        #pragma region 生成深度贴图
        glEnable(GL_DEPTH_TEST);
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix",lightSpaceMatrix);
        depthShader.setMat4("model",model);
        
        glViewport(0,0,SHADOW_WIDTH,SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        drawDepthObjects(depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glViewport(0, 0, width, height);
        #pragma endregion

        #pragma region 生成深度立方体贴图
        
        glm::vec3 shadowCasterPos = lightPos; // fallback
        bool foundShadowCaster = false;

        for (const auto& obj : build.objects)
        {
            if (obj.type == ObjType::Light)
            {
                shadowCasterPos = glm::vec3(obj.model[3]); // 从模型矩阵取平移分量
                foundShadowCaster = true;
                break; // 先用第一盏灯作为投影灯
            }
        }
        //创建六个观察方向
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj*
                        glm::lookAt(shadowCasterPos,shadowCasterPos+glm::vec3(1.0,0.0,0.0),glm::vec3(0.0,-1.0,0.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(shadowCasterPos, shadowCasterPos + glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(shadowCasterPos, shadowCasterPos + glm::vec3(0.0,1.0,0.0), glm::vec3(0.0,0.0,1.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(shadowCasterPos, shadowCasterPos + glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,-1.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(shadowCasterPos, shadowCasterPos + glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,-1.0,0.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(shadowCasterPos, shadowCasterPos + glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,-1.0,0.0)));

        pointdepthShader.use();
        pointdepthShader.setVec3("lightPos",shadowCasterPos);
        pointdepthShader.setMat4("shadowMatrices[0]",shadowTransforms[0]);
        pointdepthShader.setMat4("shadowMatrices[1]",shadowTransforms[1]);
        pointdepthShader.setMat4("shadowMatrices[2]",shadowTransforms[2]);
        pointdepthShader.setMat4("shadowMatrices[3]",shadowTransforms[3]);
        pointdepthShader.setMat4("shadowMatrices[4]",shadowTransforms[4]);
        pointdepthShader.setMat4("shadowMatrices[5]",shadowTransforms[5]);
        pointdepthShader.setFloat("far_plane",far);

        glViewport(0,0,SHADOW_WIDTH,SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO_point);
        glClear(GL_DEPTH_BUFFER_BIT);
        drawDepthObjects(pointdepthShader);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glViewport(0, 0, width, height);

        #pragma endregion

        ApplyCommonSurfaceUniforms(ourShader,view,projection,lightSpaceMatrix,shadowCasterPos);
        ApplyCommonSurfaceUniforms(pbrShader,view,projection,lightSpaceMatrix,shadowCasterPos);
        ApplyCommonSurfaceUniforms(nprShader,view,projection,lightSpaceMatrix,shadowCasterPos);
        
        // 用 UI 控制渲染状态/参数（示例）
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        
        //此处修改背景颜色
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //清除深度缓冲和清屏
        glStencilMask(0xFF);
        //绑定fbo，相当于激活了fbo，后续的会渲染到fbo上
        glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        glEnable(GL_DEPTH_TEST);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D,depthMap);
        
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_CUBE_MAP,depthCubemap);
        
        #pragma region skybox
        if(useSkybox)
        {
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_LEQUAL);
            skyboxShader.use();
            skyboxShader.setMat4("projection",projection);
            glm::mat4 skyboxview=glm::mat4(glm::mat3(camera.GetViewMatrix()));
            skyboxShader.setMat4("view",skyboxview);
            skyboxShader.setInt("skybox",0);
    
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP,cubemapTexture);
            glDrawArrays(GL_TRIANGLES,0,36);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
        }

        #pragma endregion
        
        //绘制场景物体
        #pragma region sceneobjects

        // 第一遍：绘制所有不透明物体
        drawOpaqueObject(lampShader,ourShader,pbrShader,nprShader,view,projection);
        //绘制透明物体
        //收集透明物体
        std::vector<SceneObject*> transparentObjects;
        for(auto& obj:build.objects)
        {
            if(obj.type==ObjType::Image)
            {
                transparentObjects.push_back(&obj);
            }
        }
        //按离摄像机的距离从远到近排序
        std::sort(transparentObjects.begin(),transparentObjects.end(),
                [](SceneObject* a,SceneObject* b)
            {
                glm::vec3 posA=glm::vec3(a->model[3]);
                glm::vec3 posB=glm::vec3(b->model[3]);
                glm::vec3 diffA = camera.Position - posA;
                glm::vec3 diffB = camera.Position - posB;
                float distA = glm::length(diffA);
                float distB = glm::length(diffB);
                return distA>distB;
            });
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        //绘制排序后的透明物体
        for(auto* obj:transparentObjects)
        {
            glStencilMask(0x00);
            if(obj->selected)
            {
                glStencilFunc(GL_ALWAYS,1,0xFF);
                glStencilMask(0xFF);
            }
            //绘制
            Shader& surfaceShader = SelectSurfaceShader(*obj,ourShader,pbrShader,nprShader);
            surfaceShader.use();
            ApplySurfaceMaterialUniforms(surfaceShader,*obj,obj->material.renderMode==RenderMode::Phong);
            DrawSceneObjectGeometry(surfaceShader,*obj);
        }
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);

        // 绘制所有选中物体的轮廓
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDepthMask(GL_FALSE); // 禁止写入深度，但仍进行深度测试

        

        for (auto& obj : build.objects)
        {
            if (obj.selected)
            {
                lampShader.use();
                lampShader.setMat4("view",view);
                lampShader.setMat4("projection",projection);
                
                const float outlineScale = 1.0f + std::max(obj.material.outlineWidth, 0.0f);
                glm::mat4 outlineModel=obj.model;
                outlineModel=glm::scale(outlineModel,glm::vec3(outlineScale));
                lampShader.setMat4("model",outlineModel);
                
                if(obj.type==ObjType::Model&&obj.modelAsset)
                {
                    obj.modelAsset->Draw(lampShader);
                }
                else if(obj.type==ObjType::Image)
                {
                    build.quad.Draw();
                }
                else
                {
                    build.cube.Draw();
                }
            }
        }

        // 恢复状态
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS,0,0xFF);
        glDepthMask(GL_TRUE);

        //绘制所有物体
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glDisable(GL_DEPTH_TEST);
        framebuffers.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,colorTex);
        framebuffers.setInt("screenTexture",0);
        build.screenquad.Draw();

        #pragma endregion

        // ===== ImGui draw =====
        ui.BeginUI();
        ImGuizmo::BeginFrame();
        ui.DrawUI(build,gizmoOp,gizmoMode);
        
        //调用imguizmo
        if (build.selectedId != -1)
        {
            // 找到选中的对象
            for (auto& obj : build.objects)
            {
                if (obj.id == build.selectedId)
                {
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

                    // ImGuizmo 需要屏幕尺寸
                    ImGuiIO& io = ImGui::GetIO();
                    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

                    // 使用相机矩阵
                    ImGuizmo::Manipulate(
                        glm::value_ptr(view),
                        glm::value_ptr(projection),
                        gizmoOp,
                        gizmoMode,
                        glm::value_ptr(obj.model)
                    );

                    break;
                }
            }
        }
        ui.EndUI();

        lampShader.setMat4("model", glm::mat4(1.0f));
        grid.draw(view, projection, camera.Position);
        
        glfwSwapBuffers(window);//显示窗口
        glfwPollEvents(); 

        
    }

    ui.ReleaseUI();

    glfwTerminate();


    return 0;
}

void drawDepthObjects(Shader& depthShader)
{
    depthShader.use();

    for (auto& obj : build.objects)
    {
        // 透明物体一般不参与阴影深度图
        if (obj.type == ObjType::Image) continue;

        // 灯的可视化几何体通常不作为遮挡物（可按需删掉）
        if (obj.type == ObjType::Light) continue;

        depthShader.setMat4("model", obj.model);

        if (obj.type == ObjType::Model && obj.modelAsset)
        {
            obj.modelAsset->Draw(depthShader);
        }
        else
        {
            // 你当前场景里的基础几何体
            build.cube.Draw();
        }
    }
}

void BindDefaultSurfaceTextures()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TexCache.GetDefault());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, TexCache.GetDefault());
    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, TexCache.GetDefaultNormal());
}

void BindObjectMaterialTextures(const SceneObject& obj)
{
    BindDefaultSurfaceTextures();

    glActiveTexture(GL_TEXTURE0);
    unsigned int id = TexCache.Get(obj.material.diffuseTexPath, true);
    glBindTexture(GL_TEXTURE_2D, id != 0 ? id : TexCache.GetDefault());

    glActiveTexture(GL_TEXTURE1);
    id = TexCache.Get(obj.material.specularTexPath, false);
    glBindTexture(GL_TEXTURE_2D, id != 0 ? id : TexCache.GetDefault());

    glActiveTexture(GL_TEXTURE12);
    id = TexCache.Get(obj.material.normalTexPath, false);
    glBindTexture(GL_TEXTURE_2D, id != 0 ? id : TexCache.GetDefaultNormal());
}

void ApplySurfaceMaterialUniforms(Shader& shader,const SceneObject& obj,bool enableNormalMap)
{
    shader.setMat4("transform", obj.model);
    shader.setVec3("material.albedo", obj.material.albedo);
    shader.setFloat("material.metallic", obj.material.metallic);
    shader.setFloat("material.roughness", obj.material.roughness);
    shader.setFloat("material.ao", obj.material.ao);
    shader.setFloat("material.toonLevels", obj.material.toonLevels);
    shader.setFloat("material.outlineWidth", obj.material.outlineWidth);
    shader.setFloat("material.shininess", obj.material.shininess);
    shader.setInt("material.texture_diffuse1", 0);
    shader.setInt("material.texture_specular1", 1);
    shader.setInt("normalMap", 12);
    shader.setBool("useNormalMap", enableNormalMap);

    BindObjectMaterialTextures(obj);
}

void ApplyCommonSurfaceUniforms(Shader& shader,const glm::mat4& view,const glm::mat4& projection,const glm::mat4& lightSpaceMatrix,const glm::vec3& shadowCasterPos)
{
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.setVec3("viewPos", camera.Position);
    shader.setFloat("far_plane", 25.0f);
    shader.setInt("shadowMap", 10);
    shader.setInt("depthMap", 11);
    shader.setBool("blinn", useBlinnPhongShader);

    shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    if(enableDirlight)
    {
        shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    }
    else
    {
        shader.setVec3("dirLight.ambient", 0.0f, 0.0f, 0.0f);
        shader.setVec3("dirLight.diffuse", 0.0f, 0.0f, 0.0f);
        shader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);
    }

    int lightIndex = 0;
    const int maxLights = 8;
    for(const auto& obj : build.objects)
    {
        if(obj.type == ObjType::Light && lightIndex < maxLights)
        {
            glm::vec3 pos = glm::vec3(obj.model[3]);
            std::string base = "pointLights[" + std::to_string(lightIndex) + "].";
            shader.setVec3(base + "position", pos);
            shader.setVec3(base + "ambient", 0.25f, 0.25f, 0.25f);
            shader.setVec3(base + "diffuse", 0.8f, 0.8f, 0.8f);
            shader.setVec3(base + "specular", 1.0f, 1.0f, 1.0f);
            shader.setFloat(base + "constant", 1.0f);
            shader.setFloat(base + "linear", 0.09f);
            shader.setFloat(base + "quadratic", 0.032f);
            lightIndex++;
        }
    }
    shader.setInt("numPointLights", lightIndex);

    shader.setVec3("spotLight.position", camera.Position);
    shader.setVec3("spotLight.direction", camera.Front);
    shader.setVec3("spotLight.ambient", 1.0f, 1.0f, 1.0f);
    shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.09f);
    shader.setFloat("spotLight.quadratic", 0.032f);
    shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
    shader.setVec3("shadowCasterPos", shadowCasterPos);
}

Shader& SelectSurfaceShader(const SceneObject& obj,Shader& phongShader,Shader& pbrShader,Shader& nprShader)
{
    switch(obj.material.renderMode)
    {
        case RenderMode::PBR:
            return pbrShader;
        case RenderMode::NPR:
            return nprShader;
        case RenderMode::Phong:
        default:
            return phongShader;
    }
}

void DrawSceneObjectGeometry(Shader& shader,SceneObject& obj)
{
    shader.setMat4("transform", obj.model);

    if(obj.type == ObjType::Model && obj.modelAsset)
    {
        obj.modelAsset->Draw(shader);
    }
    else if(obj.type == ObjType::Image)
    {
        build.quad.Draw();
    }
    else
    {
        build.cube.Draw();
    }
}

void drawOpaqueObject(Shader& lampShader,Shader& phongShader,Shader& pbrShader,Shader& nprShader,const glm::mat4& view,const glm::mat4& projection)
{
    for (auto& obj : build.objects)
    {
        //跳过透明物体
        if(obj.type==ObjType::Image)continue;

        // 默认不写入模板
        glStencilMask(0x00);

        if(obj.selected)
        {
            // 如果被选中，允许写入模板，且值设为 1
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);
        }

        if(obj.type==ObjType::Light)
        {
            lampShader.use();
            lampShader.setMat4("view", view);
            lampShader.setMat4("projection", projection);
            lampShader.setMat4("model",obj.model);
            build.cube.Draw();
        }
        else if(obj.type==ObjType::Model&&obj.modelAsset)
        {
            Shader& surfaceShader = SelectSurfaceShader(obj,phongShader,pbrShader,nprShader);
            surfaceShader.use();
            ApplySurfaceMaterialUniforms(surfaceShader,obj,false);
            DrawSceneObjectGeometry(surfaceShader,obj);
        }
        
        else //cube
        {
            Shader& surfaceShader = SelectSurfaceShader(obj,phongShader,pbrShader,nprShader);
            surfaceShader.use();
            const bool enableNormalMap = obj.material.renderMode == RenderMode::Phong && !obj.material.normalTexPath.empty();
            ApplySurfaceMaterialUniforms(surfaceShader,obj,enableNormalMap);
            DrawSceneObjectGeometry(surfaceShader,obj);
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    resizeFramebufferAttachments(width, height);
}

void resizeFramebufferAttachments(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    if (width == gFbWidth && height == gFbHeight) return;
    if (gColorTex != 0)
    {
        glBindTexture(GL_TEXTURE_2D, gColorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    if (gRBO != 0)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, gRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    gFbWidth = width;
    gFbHeight = height;
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;
    if(!cameraControl) return;


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD,deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD,deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT,deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT,deltaTime);
}

//鼠标输入
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if(!cameraControl) return;
    if(ImGui::GetIO().WantCaptureMouse)
        return;

    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset,yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if(ImGui::GetIO().WantCaptureMouse)
        return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // 如果 ImGui 正在使用鼠标，就不选
        if (ImGui::GetIO().WantCaptureMouse) return;

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)width / (float)height, 0.1f, 100.0f);

        raycaster.PickObject(mx, my, width, height, view, projection, build);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}




void drop_callback(GLFWwindow* window,int count, const char** paths)
{
    for(int i=0;i<count;i++)
    {
        build.ImportModel(paths[i]);
    }
}

//加载天空盒纹理LDR
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1,&textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP,textureID);

    int width,height,nrChannels;
    for(unsigned int i=0;i<faces.size();i++)
    {
        unsigned char *data=stbi_load(faces[i].c_str(),&width,&height,&nrChannels,0);
        if(data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,
                        0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout<<"cubemap texture failed to load at path:"<<faces[i]<<std::endl;
            stbi_image_free(data);
        }
    }
        //设定每个面的环绕啊和过滤方式
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

    return textureID;

}
