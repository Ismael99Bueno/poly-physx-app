#ifndef MENU_LAYER_HPP
#define MENU_LAYER_HPP

#include "ppx/layer.hpp"

namespace ppx
{
    class menu_layer final : public layer
    {
    public:
        menu_layer();

    private:
        app *m_app;

        void on_attach(app *papp) override;
        void on_render() override;
    };
}

#endif