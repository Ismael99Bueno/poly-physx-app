#include "ppx-app/pch.hpp"
#include "ppx-app/layer.hpp"

namespace ppx
{
layer::layer(const char *name) : m_name(name)
{
}

const char *layer::name() const
{
    return m_name;
}

#ifdef HAS_YAML_CPP
void layer::write(YAML::Emitter &out) const
{
    out << YAML::Key << "Enabled" << YAML::Value << p_enabled;
    out << YAML::Key << "Visible" << YAML::Value << p_visible;
}
YAML::Node layer::encode() const
{
    YAML::Node node;
    node["Enabled"] = p_enabled;
    node["Visible"] = p_visible;
    return node;
}
bool layer::decode(const YAML::Node &node)
{
    if (!node.IsMap() || node.size() < 2)
        return false;
    p_enabled = node["Enabled"].as<bool>();
    p_visible = node["Visible"].as<bool>();
    return true;
}
#endif
#ifdef HAS_YAML_CPP
YAML::Emitter &operator<<(YAML::Emitter &out, const layer &ly)
{
    out << YAML::BeginMap;
    ly.write(out);
    out << YAML::EndMap;
    return out;
}
#endif
} // namespace ppx

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
} // namespace YAML
#endif