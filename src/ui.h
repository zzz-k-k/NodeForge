#ifndef UI_H
#define UI_H

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuizmo.h"
#include "build.h"

#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#include<algorithm>
#include<string>

bool showBuildWindow=false;


class UI
{
    public:
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
                build.objects.push_back(obj);
            }
            ImGui::End();
        }

        //右侧常驻菜单
        ImGuiIO& io=ImGui::GetIO();
        ImVec2 size(240,300);
        ImVec2 pos(io.DisplaySize.x-size.x-10,220);
        ImGui::SetNextWindowPos(pos,ImGuiCond_Always);
        ImGui::SetNextWindowSize(size,ImGuiCond_Always);

        ImGui::Begin("scene",nullptr,ImGuiWindowFlags_NoResize);

        int deleteId=-1;
        
        for(auto& obj:build.objects)
        {
            std::string label="cube##"+std::to_string(obj.id);
            bool isSelected=(obj.id==build.selectedId);

            if(ImGui::Selectable(label.c_str(),isSelected))
            {
                build.selectedId=obj.id;
                for(auto& o:build.objects)o.selected=(o.id=obj.id);
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
            ImVec2 size(360,200);
            ImVec2 pos(io.DisplaySize.x-size.x-10,40);
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
    }

};



#endif