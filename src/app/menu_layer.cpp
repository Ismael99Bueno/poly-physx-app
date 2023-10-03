#include "ppx-app/internal/pch.hpp"
#include "ppx-app/app/menu_layer.hpp"
#include "lynx/app/app.hpp"
#include "lynx/app/window.hpp"
#include "lynx/geometry/camera.hpp"

namespace ppx
{
menu_layer::menu_layer() : lynx::layer2D("Menu layer")
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