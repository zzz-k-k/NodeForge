//使用这些代需要先在cmakelist中定义SHADERGRAPH_ENABLE_NODE_EDITOR

#ifndef SHADERGRAPH_H
#define SHADERGRAPH_H

#include "imgui.h"

#ifdef SHADERGRAPH_ENABLE_NODE_EDITOR
#include "imgui_node_editor.h"
#include <vector>
#include <string>
#include <algorithm>


namespace ed=ax::NodeEditor;
#endif

class ShaderGraphUI
{
public:
    void Init()
    {
#ifdef SHADERGRAPH_ENABLE_NODE_EDITOR
        //如果没有定义SHADERGRAPH_ENABLE_NODE_EDITOR将不会执行之后的代码
        if (!context)
        {
            ed::Config config;
            config.SettingsFile="ShaderGraph.json";//保存节点位置配置
            context=ed::CreateEditor(&config);
            BuildDemoGraph();
        }
#endif
    }

    void Shutdown()
    {
#ifdef SHADERGRAPH_ENABLE_NODE_EDITOR
        if (context)
        {
            ed::DestroyEditor(context);
            context = nullptr;
        }
#endif
    }

    void Draw(bool* open)
    {
        if (open && !*open)
            return;

        if (!ImGui::Begin("ShaderGraph", open))
        {
            ImGui::End();
            return;
        }
        
        #ifdef SHADERGRAPH_ENABLE_NODE_EDITOR
        ax::NodeEditor::SetCurrentEditor(context);
        ed::EnableShortcuts(!ImGui::GetIO().WantTextInput);
        ax::NodeEditor::Begin("ShaderGraphEditor");
        ed::NodeId contextNodeId = 0;

        for (const auto& node : nodes)
        {
            ed::BeginNode(node.id);
            ImGui::TextUnformatted(node.name.c_str());

            if (node.inputPin != 0)
            {
                ed::BeginPin(node.inputPin, ed::PinKind::Input);
                ImGui::Text("->In");
                ed::EndPin();
            }

            if (node.outputPin != 0)
            {
                ed::BeginPin(node.outputPin, ed::PinKind::Output);
                ImGui::Text("Out->");
                ed::EndPin();
            }

            ed::EndNode();
        }

        //绘制连线
        for (const auto& link : links)
            ed::Link(link.id, link.startPin, link.endPin);

        //处理交互，创建连线
        if(ed::BeginCreate())
        {
            ed::PinId inputPinId,outputPinId;
            if(ed::QueryNewLink(&inputPinId,&outputPinId))
            {
                if(inputPinId&&outputPinId)
                {
                    if(ed::AcceptNewItem())
                    {
                        links.push_back({GetNextId(),(int)inputPinId.Get(),(int)outputPinId.Get()});
                    }
                }
            }
            ed::EndCreate();
        }

        //处理交互，删除
        if(ed::BeginDelete())
        {
            ed::LinkId deletedLinkId;
            //循环查询被选中的连线
            while(ed::QueryDeletedLink(&deletedLinkId))
            {
                if(ed::AcceptDeletedItem())
                {
                    links.erase(std::remove_if(links.begin(),links.end(),
                    [deletedLinkId](const Link& link){return link.id==deletedLinkId.Get();}),
                    links.end());
                }
            }

            ed::NodeId deletedNodeId;

            //循环查询被选中的节点
            while(ed::QueryDeletedNode(&deletedNodeId))
            {
                if(ed::AcceptDeletedItem())
                {
                    int nodeIdInt=(int)deletedNodeId.Get();
                    auto it=std::find_if(nodes.begin(),nodes.end(),[nodeIdInt](const Node& n){return n.id==nodeIdInt;});
                    if(it!=nodes.end())
                    {
                        int inPin=it->inputPin;
                        int outPin=it->outputPin;
                        //删除这些pin的所有连线
                        links.erase(std::remove_if(links.begin(),links.end(),
                                    [inPin,outPin](const Link& l)
                                    {
                                        return l.startPin==outPin||l.endPin==inPin||l.startPin==inPin||l.endPin==outPin;
                                    }),links.end());
                    }
                    //从vector中移出节点
                    nodes.erase(std::remove_if(nodes.begin(),nodes.end(),
                                [nodeIdInt](const Node& n){return n.id==nodeIdInt;}),
                                nodes.end());
                }
            }
            ed::EndDelete();
        }

        //处理右键菜单
        ed::Suspend();
        if(ed::ShowNodeContextMenu(&contextNodeId))
        {
            ImGui::OpenPopup("NodeContextMenu");
        }
        else if(ed::ShowBackgroundContextMenu())
        {
            ImGui::OpenPopup("CreateNodeMenu");
        }
        if(ImGui::BeginPopup("CreateNodeMenu"))
        {
            ImGui::Text("Create Node:");
            ImGui::Separator();

            if(ImGui::MenuItem("Add teexture2D"))
            {
                CreateNode("Texture2D",false,true);
            }
            if(ImGui::MenuItem("Add Multiply"))
            {
                CreateNode("Multply",true,true);
            }
            if(ImGui::MenuItem("Add Output"))
            {
                CreateNode("Output",true,false);
            }
            ImGui::EndPopup();
        }
        if(ImGui::BeginPopup("NodeContextMenu"))
        {
            ImGui::Text("Node");
            ImGui::Separator();
            if(ImGui::MenuItem("Delete"))
            {
                ed::DeleteNode(contextNodeId);
                //todo 这里有bug，删除逻辑没有正常工作
            }
            ImGui::EndPopup();
        }

        ed::Resume();
        ed::End();
        ed::SetCurrentEditor(nullptr);

#else
        ImGui::TextUnformatted("imgui-node-editor is not enabled.");
        ImGui::Spacing();
        ImGui::TextUnformatted("Define SHADERGRAPH_ENABLE_NODE_EDITOR and add the library includes.");
#endif

        ImGui::End();
    }

private:
#ifdef SHADERGRAPH_ENABLE_NODE_EDITOR
    struct Node
    {
        int id = 0;
        int inputPin = 0;
        int outputPin = 0;
        std::string name;
    };

    struct Link
    {
        int id = 0;
        int startPin = 0;
        int endPin = 0;
    };

    ax::NodeEditor::EditorContext* context = nullptr;
    std::vector<Node> nodes;
    std::vector<Link> links;
    int uniqueIdCounter=1000;

    //生成唯一id
    int GetNextId()
    {
        return uniqueIdCounter++;
    }

    void CreateNode(const std::string& name,bool hasInput,bool hasOutput)
    {
        int nodeId=GetNextId();
        int inPin=hasInput?GetNextId():0;
        int outPin=hasOutput?GetNextId():0;

        nodes.push_back({nodeId,inPin,outPin,name});
        ed::SetNodePosition(nodeId,ed::ScreenToCanvas(ImGui::GetMousePos()));
    }

    void BuildDemoGraph()
    {
        nodes.clear();
        links.clear();

        nodes.push_back({1, 0, 11, "Texture2D"});
        nodes.push_back({2, 12, 21, "Multiply"});
        nodes.push_back({3, 22, 0, "Output"});

        links.push_back({100, 11, 12});
        links.push_back({101, 21, 22});
    }
#endif
};

#endif
