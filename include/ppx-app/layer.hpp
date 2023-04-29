#ifndef LAYER_HPP
#define LAYER_HPP

#include <string>
#include <SFML/Graphics.hpp>
#include "ini/saveable.hpp"

namespace ppx
{
    class app;
    class layer : public ini::saveable
    {
    public:
        layer(const char *name);
        virtual ~layer() = default;

        virtual void write(ini::output &out) const override;
        virtual void read(ini::input &in) override;

        bool p_enabled = true, p_visible = true;

    private:
        virtual void on_attach(app *papp) {}
        virtual void on_start() {}
        virtual void on_update() {}
        virtual void on_late_update() {}
        virtual void on_render() {}
        virtual void on_event(sf::Event &event) {}
        virtual void on_detach() {}
        virtual void on_end() {}

        const char *m_name;

        friend class app;
    };
}

#endif