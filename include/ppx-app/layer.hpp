#ifndef LAYER_HPP
#define LAYER_HPP

#include <string>
#include <SFML/Graphics.hpp>
#include "ini/serializable.hpp"

namespace ppx
{
    class app;
    class layer : public ini::serializable
    {
    public:
        layer(const char *name);
        virtual ~layer() = default;

        virtual void serialize(ini::serializer &out) const override;
        virtual void deserialize(ini::deserializer &in) override;

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