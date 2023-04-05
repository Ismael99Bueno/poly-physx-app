#ifndef LAYER_HPP
#define LAYER_HPP

#include <string>
#include <SFML/Graphics.hpp>

namespace ppx
{
    class app;
    class layer
    {
    public:
        layer() = default;
        virtual ~layer() = default;

    private:
        virtual void on_attach(app *papp) {}
        virtual void on_start() {}
        virtual void on_render() {}
        virtual void on_event(sf::Event &event) {}
        virtual void on_detach() {}
        virtual void on_end() {}

        friend class app;
    };
}

#endif