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
bool useSkybox=false;
bool enableDirlight=true;
bool useBlinnPhongShader=false;

bool p_open=false;
float fps;
float frame_time;
float delta_time;
std::vector<float> frame_times;    // 历史帧时间记录
std::vector<float> fps_history;    // 历史FPS记录
size_t max_history_size = 300;

size_t current_memory_usage = 0;
size_t peak_memory_usage = 0;

float gpu_time = 0.0f;
size_t draw_calls = 0;
size_t vertices_count = 0;
size_t indices_count = 0;

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

        fps = io.Framerate;          // 当前帧率（FPS）
        frame_time = 1000.0f / fps;  // 每帧耗时（毫秒）
        delta_time = io.DeltaTime;
        
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
            if(ImGui::Begin("fps"))
            {
                ImGuiIO& io=ImGui::GetIO();
                ImGui::Text("fps:%.1f",io.Framerate);
                ImGui::Text("frame time:%.3f ms",1000.0f/io.Framerate);
                ImGui::Text("delta time:%.6f s",io.DeltaTime);

                ImGui::Separator();

                //内存计算
                frame_times.push_back(delta_time*1000.0f);
                fps_history.push_back(1.0f/delta_time);
                if(frame_times.size()>max_history_size){
                    frame_times.erase(frame_times.begin());
                    fps_history.erase(fps_history.begin());
                }
                peak_memory_usage=std::max(peak_memory_usage,current_memory_usage);
                

                ImGui::Text("memory usage:%.2f MB",current_memory_usage/(1024.0f*1024.0f));
                ImGui::Text("peak memory:%.2f MB",peak_memory_usage/(1024.0f*1024.0f));
                
                ImGui::Separator();

                ImGui::Text("draw calls:%zu",draw_calls);
                ImGui::Text("Vertices: %zu",vertices_count);
                ImGui::Text("Indices: %zu",indices_count);
                ImGui::Text("GPU Time: %.3f ms",gpu_time);

                ImGui::End();
            }
            ImGui::EndMainMenuBar();
        }
        if(showBuildWindow)
        {
            ImGui::Begin("build",&showBuildWindow);
            if(ImGui::Button("cube"))
            {
                build.CreateCube();
            }
            if(ImGui::Button("light"))
            {
                build.CreateLight();
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
            ImGui::Checkbox("skybox",&useSkybox);
            ImGui::Checkbox("dirlight",&enableDirlight);
            ImGui::Checkbox("useBlinPhongShader",&useBlinnPhongShader);
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
            std::string name = GetObjTypeLabel(obj.type);
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
            ImVec2 size(300,360);
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
                ImGui::Separator();
                ImGui::Text("type: %s", GetObjTypeLabel(selected->type));

                if(selected->type != ObjType::Light)
                {
                    const char* currentModeLabel = GetRenderModeLabel(selected->material.renderMode);
                    if(ImGui::BeginCombo("render mode", currentModeLabel))
                    {
                        const RenderMode renderModes[] = {RenderMode::Phong, RenderMode::PBR, RenderMode::NPR};
                        for(RenderMode mode : renderModes)
                        {
                            const bool isSelectedMode = selected->material.renderMode == mode;
                            if(ImGui::Selectable(GetRenderModeLabel(mode), isSelectedMode))
                                selected->material.renderMode = mode;
                            if(isSelectedMode)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    ImGui::ColorEdit3("albedo", glm::value_ptr(selected->material.albedo));
                    ImGui::ColorEdit3("shadow color", glm::value_ptr(selected->material.shadowColor));
                    ImGui::SliderFloat("metallic", &selected->material.metallic, 0.0f, 1.0f);
                    ImGui::SliderFloat("roughness", &selected->material.roughness, 0.05f, 1.0f);
                    ImGui::SliderFloat("ao", &selected->material.ao, 0.0f, 1.0f);
                    ImGui::SliderFloat("toon levels", &selected->material.toonLevels, 2.0f, 6.0f, "%.0f");
                    ImGui::SliderFloat("shadow threshold", &selected->material.shadowThreshold, 0.0f, 0.95f, "%.2f");
                    ImGui::SliderFloat("spec threshold", &selected->material.specularThreshold, 0.0f, 0.99f, "%.2f");
                    ImGui::ColorEdit3("rim light color", glm::value_ptr(selected->material.rimLightColor));
                    ImGui::SliderFloat("rim intensity", &selected->material.rimLightIntensity, 0.0f, 1.5f, "%.2f");
                    ImGui::SliderFloat("rim width", &selected->material.rimLightWidth, 0.02f, 1.0f, "%.2f");
                    ImGui::ColorEdit3("outline color", glm::value_ptr(selected->material.outlineColor));
                    ImGui::SliderFloat("outline width", &selected->material.outlineWidth, 0.0f, 0.2f, "%.3f");
                    ImGui::SliderFloat("shininess", &selected->material.shininess, 1.0f, 128.0f);
                }

                ImGui::Separator();
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
