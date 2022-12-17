#include "layer.hpp"

namespace phys
{
    layer::layer(const std::string &name) : m_name(name) {}

    const std::string &layer::name() const { return m_name; }
}