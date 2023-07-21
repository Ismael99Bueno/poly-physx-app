#ifndef PPX_MENU_LAYER_HPP
#define PPX_MENU_LAYER_HPP

#include "lynx/app/layer.hpp"

namespace ppx
{
class menu_layer : public lynx::layer
{
  public:
    menu_layer();

  private:
    void on_attach() override;
    void on_render(float ts) override;

    lynx::window *m_window;
};
} // namespace ppx

#endif