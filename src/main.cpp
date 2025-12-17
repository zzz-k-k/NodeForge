#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<shader.h>
#include<iostream>
#include<cmath>
#include<stb_image.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <camera.h>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int LoadTexture2D(const char* path);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

float deltaTime = 0.0f;
float lastFrame = 0.0f; 
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float vertices[] = 
{
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

unsigned int indices[]=
{
    0,1,3,
    1,2,3
};

//世界坐标
glm::vec3 cubePositions[] = 
{
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f)  
};

//------------TRANSFORM--------------
static bool wireframe = false;
float x=1.0f;
float zrotation=0.0f;
float xrotation=0.0f;
float yrotation=0.0f;
glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 pos =glm::vec3(0.0f,0.0f,0.0f);

//鼠标初始位置
float lastX = 400, lastY = 300;
bool firstMouse=true;


float cameraControl=false;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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

    //-----------创建顶点数组对象------------
    //使用顶点数组对象
    unsigned int VAO,VBO,EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1,&EBO);
    // 1. 绑定VAO_sqr
    glBindVertexArray(VAO);
    // 2. 把顶点数组复制到缓冲中供OpenGL使用
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 3. 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //设置颜色
    // glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
    // glEnableVertexAttribArray(1);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader ourShader("../../src/shader/shader.vs", "../../src/shader/shader.fs");
    
    
    // ===== ImGui init =====
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io;
    io.FontGlobalScale = 1.5f; 
    
    ImGui::StyleColorsDark();
    
    // 绑定到 GLFW + OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 330");
    

    //启用深度缓冲
    glEnable(GL_DEPTH_TEST);
    
    

    glm::mat4 view;
    view=glm::lookAt(glm::vec3(0.0f,0.0f,3.0f),
                     glm::vec3(0.0f,0.0f,0.0f),
                     glm::vec3(0.0f,1.0f,0.0f));

    float radius = 10.0f;
    
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    
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

        processInput(window);
        
        
        ourShader.use();
        ourShader.setFloat("u_Tiling", x);
        
        ourShader.setInt("texture1",0);
        ourShader.setInt("texture2", 1); // 或者使用着色器类设置
        
        // ===== ImGui per-frame begin =====
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // ===== 这里写你的调试面板 =====
        ImGui::Begin("Debug");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Checkbox("Wireframe", &wireframe);
        ImGui::SliderFloat("x",&x,0.0f,5.0f);
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("zrotation", &zrotation, 0.0f,360.0f);
            ImGui::SliderFloat("Xrotation",&xrotation,-180.0f,180.0f);
            ImGui::SliderFloat("yrotation",&yrotation,-180.0f,180.0f);
            ImGui::DragFloat3("scale", glm::value_ptr(scale), 0.01f);
            ImGui::DragFloat3("position", glm::value_ptr(pos), 0.01f);
        }
        ImGui::End();
        
        
        //变换
        glm::mat4 trans;
        trans = glm::rotate(trans, glm::radians(zrotation), glm::vec3(0.0, 0.0, 1.0));
        trans =glm::rotate(trans,glm::radians(xrotation),glm::vec3(1.0,0.0,0.0));
        trans =glm::rotate(trans,glm::radians(yrotation),glm::vec3(0.0,1.0,0.0));
        trans = glm::scale(trans, scale);
        //位移
        trans = glm::translate(trans, pos);
        //旋转

        float camX = sin(glfwGetTime()) * radius;
        float camZ = cos(glfwGetTime()) * radius;



        ourShader.setMat4("transform",trans);
        
        glm::mat4 view = camera.GetViewMatrix(); 
        ourShader.setMat4("view",view);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)800.0f / (float)600.0f, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        
        // 用 UI 控制渲染状态/参数（示例）
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //清除深度缓冲和清屏
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //------------创建生成纹理对象---------
        unsigned int texture1 = LoadTexture2D("container.jpg");
        unsigned int texture2 = LoadTexture2D("awesomeface.png");
        
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        
        glBindVertexArray(VAO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        for(unsigned int i = 1; i < 10; i++)
        {
            glm::mat4 trans;
            trans = glm::translate(trans, cubePositions[i]);
            float angle = 20.0f * i; 
            trans = glm::rotate(trans, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMat4("transform", trans);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ===== ImGui draw =====
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        
        glfwSwapBuffers(window);//显示窗口
        glfwPollEvents(); 

        
    }

    //释放imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();


    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

//加载纹理对象
unsigned int LoadTexture2D(const char* path)
{
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w, h, c;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &c, 0);
    if (data)
    {
        GLenum format = (c == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: " << path
                  << " reason: " << stbi_failure_reason() << std::endl;
    }
    stbi_image_free(data);

    return tex;
}
