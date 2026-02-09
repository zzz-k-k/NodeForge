#ifndef UI_H
#define UI_H

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuizmo.h"
#include "build.h"
#include "nfd.h"

#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#include<algorithm>
#include<string>

#include<shadergraph.h>

bool showBuildWindow=false;
bool showShaderGraphWindow=false;


class UI
{
    public:
    ShaderGraphUI shaderGraph;

    // ===== ImGui init =====
    void UIinit(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); 
        (void)io;
        io.FontGlobalScale = 1.5f; 
        
        ImGui::StyleColorsDark();

        // 绑定到 GLFW + OpenGL3
        ImGui_ImplGlfw_InitForOpenGL(window, false);
        ImGui_ImplOpenGL3_Init("#version 330");

        //初始化shaderui
        shaderGraph.Init();
    }
    void BeginUI()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

    }

    void DrawUI(BuildSystem& build,ImGuizmo::OPERATION& gizmoOp,ImGuizmo::MODE& gizmoMode)
    {
        
        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("Window"))
            {
                ImGui::MenuItem("build",nullptr,&showBuildWindow);
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("file"))
            {
                if(ImGui::MenuItem("import"))
                {
                    nfdchar_t* outPath=nullptr;
                    nfdresult_t result=NFD_OpenDialog("obj;fbx;gltf",nullptr,&outPath);
                    if(result==NFD_OKAY)
                    {
                        build.ImportModel(outPath);
                        free(outPath);
                    }
                }
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("shader"))
            {
                ImGui::MenuItem("Shader Graph",nullptr,&showShaderGraphWindow);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        if(showBuildWindow)
        {
            ImGui::Begin("build",&showBuildWindow);
            if(ImGui::Button("cube"))
            {
                SceneObject obj;
                obj.id=build.nextId++;
                obj.model=glm::mat4(1.0f);
                obj.selected=false;
                obj.type=ObjType::Cube;
                build.objects.push_back(obj);
            }
            if(ImGui::Button("light"))
            {
                SceneObject obj;
                obj.id=build.nextId++;
                obj.model=glm::mat4(1.0f);
                obj.selected=false;
                obj.type=ObjType::Light;
                build.objects.push_back(obj);
            }
            if(ImGui::Button("image"))
            {
                nfdchar_t* outPath=nullptr;
                    nfdresult_t result=NFD_OpenDialog("png,jpg",nullptr,&outPath);
                    if(result==NFD_OKAY)
                    {
                        build.CreateImage(outPath);
                        free(outPath);
                    }
            }
            ImGui::End();
            
        }
        if(showShaderGraphWindow)
            shaderGraph.Draw(&showShaderGraphWindow);

        //右侧常驻菜单
        ImGuiIO& io=ImGui::GetIO();
        ImVec2 size(300,260);
        ImVec2 pos(io.DisplaySize.x-size.x-10,40);
        ImGui::SetNextWindowPos(pos,ImGuiCond_Always);
        ImGui::SetNextWindowSize(size,ImGuiCond_Always);

        ImGui::Begin("scene",nullptr,ImGuiWindowFlags_NoResize);

        int deleteId=-1;
        
        for(auto& obj:build.objects)
        {
            std::string name = (obj.type == ObjType::Light) ? "Light" : "Cube";
            std::string label = name + "##" + std::to_string(obj.id);
            bool isSelected=(obj.id==build.selectedId);

            if(ImGui::Selectable(label.c_str(),isSelected))
            {
                build.selectedId=obj.id;
                for(auto& o:build.objects)o.selected=(o.id==obj.id);
            }
            if(ImGui::BeginPopupContextItem())
            {
                if(ImGui::MenuItem("delete"))
                {
                    deleteId=obj.id;
                }
                ImGui::EndPopup();
            }
        }
        if(deleteId!=-1)
        {
            build.objects.erase(
                std::remove_if(build.objects.begin(),build.objects.end(),
                                [deleteId](const SceneObject& o){return o.id==deleteId;}),
                build.objects.end()
            );
            if(build.selectedId==deleteId)
                build.selectedId=-1;
        }
        
        if(build.selectedId!=-1)
        {
            ImVec2 size(300,200);
            ImVec2 pos(io.DisplaySize.x-size.x-10,320);
            ImGui::SetNextWindowPos(pos,ImGuiCond_Always);
            ImGui::SetNextWindowSize(size,ImGuiCond_Always);

            ImGui::Begin("inspector",nullptr,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove);

            ImGui::Text("mode");
            ImGui::SameLine();//不换行
            if(ImGui::RadioButton("TR",gizmoOp==ImGuizmo::TRANSLATE))gizmoOp=ImGuizmo::TRANSLATE;
            ImGui::SameLine();
            if(ImGui::RadioButton("RT",gizmoOp==ImGuizmo::ROTATE))gizmoOp=ImGuizmo::ROTATE;
            ImGui::SameLine();
            if(ImGui::RadioButton("SC",gizmoOp==ImGuizmo::SCALE))gizmoOp=ImGuizmo::SCALE;

            if(ImGui::RadioButton("local",gizmoMode==ImGuizmo::LOCAL))gizmoMode=ImGuizmo::LOCAL;
            ImGui::SameLine();
            if(ImGui::RadioButton("world",gizmoMode==ImGuizmo::WORLD))gizmoMode=ImGuizmo::WORLD;

            SceneObject* selected=nullptr;
            for(auto& o:build.objects)
            {
                if(o.id==build.selectedId){selected=&o;break;}
            }
            if(selected)
            {
                float t[3],r[3],s[3];
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(selected->model), t, r, s);
                ImGui::InputFloat3("tr",t);
                ImGui::InputFloat3("rt",r);
                ImGui::InputFloat3("sc",s);
                ImGuizmo::RecomposeMatrixFromComponents(t,r,s,glm::value_ptr(selected->model));
            }

            ImGui::End();
        }

        ImGui::End();
    }
    void EndUI()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    //释放imgui
    void ReleaseUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        shaderGraph.Shutdown();
    }

};



#endif