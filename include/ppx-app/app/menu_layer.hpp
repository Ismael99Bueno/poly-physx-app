#pragma once

#include "lynx/app/layer.hpp"
#include "lynx/app/window.hpp"

namespace ppx
{
class menu_layer : public lynx::layer2D
{
  public:
    menu_layer();

  private:
    void on_attach() override;
    void on_render(float ts) override;

    lynx::window2D *m_window;
};
} // namespace ppx
