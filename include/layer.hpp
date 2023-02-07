#ifndef LAYER_HPP
#define LAYER_HPP

#include <string>
#include <SFML/Graphics.hpp>

namespace phys
{
    class app;
    class layer
    {
    public:
        layer(const std::string &name = "layer");
        virtual ~layer() = default;

        const std::string &name() const;

    protected:
        const std::string m_name;

    private:
        virtual void on_attach(app *papp) {}
        virtual void on_start() {}
        virtual void on_render() {}
        virtual void on_event(sf::Event &event) {}

        friend class app;
    };
}

#endif