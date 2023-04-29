#include "ppx-app/pch.hpp"
#include "ppx-app/layer.hpp"

namespace ppx
{
    layer::layer(const char *name) : m_name(name) {}

    void layer::write(ini::output &out) const
    {
        out.write("enabled", p_enabled);
        out.write("visible", p_visible);
    }
    void layer::read(ini::input &in)
    {
        p_enabled = (bool)in.readi16("enabled");
        p_visible = (bool)in.readi16("visible");
    }
}