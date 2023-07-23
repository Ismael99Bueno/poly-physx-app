#include "ppx-app/pch.hpp"
#include "ppx-app/menu_layer.hpp"
#include "lynx/app/app.hpp"
#include "lynx/app/window.hpp"
#include "lynx/geometry/camera.hpp"

namespace ppx
{
menu_layer::menu_layer() : lynx::layer("Menu layer")
{
}

void menu_layer::on_attach()
{
    m_window = parent()->window();
}

void menu_layer::on_render(const float ts)
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