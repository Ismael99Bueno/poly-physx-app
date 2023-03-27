#include "layer.hpp"

namespace ppx
{
    layer::layer(const std::string &name) : m_name(name) {}

    const std::string &layer::name() const { return m_name; }
}