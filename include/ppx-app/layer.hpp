#ifndef LAYER_HPP
#define LAYER_HPP

#include <string>
#include <SFML/Graphics.hpp>
#include <yaml-cpp/yaml.h>

namespace ppx
{
    class app;
    class layer
    {
    public:
        layer(const char *name);
        virtual ~layer() = default;

        bool p_enabled = true, p_visible = true;
#ifdef HAS_YAML_CPP
        virtual void write(YAML::Emitter &out) const;
        virtual YAML::Node encode() const;
        virtual bool decode(const YAML::Node &node);
#endif

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
        const char *name() const;

        friend class app;
#ifdef HAS_YAML_CPP
        friend YAML::Emitter &operator<<(YAML::Emitter &, const app &);
        friend struct YAML::convert<app>;
#endif
    };
#ifdef HAS_YAML_CPP
    YAML::Emitter &operator<<(YAML::Emitter &out, const layer &ly);
#endif
}

#ifdef HAS_YAML_CPP
namespace YAML
{
    template <>
    struct convert<ppx::layer>
    {
        static Node encode(const ppx::layer &ly);
        static bool decode(const Node &node, ppx::layer &ly);
    };
}
#endif

#endif