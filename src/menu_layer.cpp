#include "ppx-app/pch.hpp"
#include "ppx-app/menu_layer.hpp"
#include "lynx/app/app.hpp"
#include "lynx/app/window.hpp"

namespace ppx
{
menu_layer::menu_layer() : lynx::imgui_layer("Menu layer")
{
}

void menu_layer::on_attach()
{
    lynx::imgui_layer::on_attach();
    m_window = parent()->window();
}

void menu_layer::on_imgui_render()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit", "ESC"))
                m_window->close();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
} // namespace ppx