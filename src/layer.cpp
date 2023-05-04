#include "ppx-app/pch.hpp"
#include "ppx-app/layer.hpp"

namespace ppx
{
    layer::layer(const char *name) : m_name(name) {}

#ifdef HAS_YAML_CPP
    void layer::write(YAML::Emitter &out) const
    {
        out << YAML::Key << "enabled" << YAML::Value << p_enabled;
        out << YAML::Key << "visible" << YAML::Value << p_visible;
    }
    YAML::Node layer::encode() const
    {
        YAML::Node node;
        node["enabled"] = p_enabled;
        node["visible"] = p_visible;
        return node;
    }
    bool layer::decode(const YAML::Node &node)
    {
        if (!node.IsMap() || node.size() < 2)
            return false;
        p_enabled = node["enabled"].as<bool>();
        p_visible = node["visible"].as<bool>();
        return true;
    }
#endif
#ifdef HAS_YAML_CPP
    YAML::Emitter &operator<<(YAML::Emitter &out, const layer &ly)
    {
        ly.write(out);
        return out;
    }
#endif
}

#ifdef HAS_YAML_CPP
namespace YAML
{
    Node convert<ppx::layer>::encode(const ppx::layer &ly)
    {
        return ly.encode();
    }
    bool convert<ppx::layer>::decode(const Node &node, ppx::layer &ly)
    {
        return ly.decode(node);
    };
}
#endif