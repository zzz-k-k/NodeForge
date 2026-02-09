#ifndef SHADERGRAPH_H
#define SHADERGRAPH_H

#include "imgui.h"

#ifdef SHADERGRAPH_ENABLE_NODE_EDITOR
#include "imgui_node_editor.h"
#include <vector>
#include <string>
#endif

class ShaderGraphUI
{
public:
    void Init()
    {
#ifdef SHADERGRAPH_ENABLE_NODE_EDITOR
        if (!context)
        {
            context = ax::NodeEditor::CreateEditor();
            BuildDemoGraph();
        }
#endif
    }

    void Shutdown()
    {
#ifdef SHADERGRAPH_ENABLE_NODE_EDITOR
        if (context)
        {
            ax::NodeEditor::DestroyEditor(context);
            context = nullptr;
        }
#endif
    }

    void Draw(bool* open)
    {
        if (open && !*open)
            return;

        if (open)
            ImGui::Begin("ShaderGraph", open);
        else
            ImGui::Begin("ShaderGraph");

#ifdef SHADERGRAPH_ENABLE_NODE_EDITOR
        ax::NodeEditor::SetCurrentEditor(context);
        ax::NodeEditor::Begin("ShaderGraphEditor");

        for (const auto& node : nodes)
        {
            ax::NodeEditor::BeginNode(node.id);
            ImGui::TextUnformatted(node.name.c_str());

            if (node.inputPin != 0)
            {
                ax::NodeEditor::BeginPin(node.inputPin, ax::NodeEditor::PinKind::Input);
                ImGui::TextUnformatted("In");
                ax::NodeEditor::EndPin();
            }

            if (node.outputPin != 0)
            {
                ax::NodeEditor::BeginPin(node.outputPin, ax::NodeEditor::PinKind::Output);
                ImGui::TextUnformatted("Out");
                ax::NodeEditor::EndPin();
            }

            ax::NodeEditor::EndNode();
        }

        for (const auto& link : links)
            ax::NodeEditor::Link(link.id, link.startPin, link.endPin);

        ax::NodeEditor::End();
        ax::NodeEditor::SetCurrentEditor(nullptr);
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
