#include "ppx-app/internal/pch.hpp"
#include "ppx-app/drawables/lines/thick_line.hpp"
#include "lynx/app/window.hpp"

namespace ppx
{
thick_line2D::thick_line2D(const glm::vec2 &p1, const glm::vec2 &p2, const lynx::color &color, const float width)
    : m_rect(color), m_circle1(color), m_circle2(color)
{
    m_circle1.transform.position = p1;
    m_circle2.transform.position = p2;

    update_width(width);
    update_orientation();
}
thick_line2D::thick_line2D(const lynx::color &color, const float width)
    : thick_line2D({0.f, 0.f}, {1.f, 0.f}, color, width)
{
}

void thick_line2D::update_width(const float width)
{
    m_circle1.radius(0.5f * width);
    m_circle2.radius(0.5f * width);

    m_rect.transform.scale.y = width;
}

void thick_line2D::update_length()
{
    m_rect.transform.scale.x = glm::distance(m_circle1.transform.position, m_circle2.transform.position);
}

void thick_line2D::update_orientation()
{
    const glm::vec2 &p1 = m_circle1.transform.position;
    const glm::vec2 &p2 = m_circle2.transform.position;

    m_rect.transform.position = 0.5f * (p1 + p2);

    const glm::vec2 dp = p2 - p1;
    m_rect.transform.rotation = atan2f(dp.y, dp.x);
}

void thick_line2D::draw(lynx::window2D &window) const
{
    window.draw(m_rect);
    window.draw(m_circle1);
    window.draw(m_circle2);
}

const glm::vec2 &thick_line2D::p1() const
{
    return m_circle1.transform.position;
}
const glm::vec2 &thick_line2D::p2() const
{
    return m_circle2.transform.position;
}

void thick_line2D::p1(const glm::vec2 &p1)
{
    m_circle1.transform.position = p1;
    update_length();
    update_orientation();
}
void thick_line2D::p2(const glm::vec2 &p2)
{
    m_circle2.transform.position = p2;
    update_length();
    update_orientation();
}

const lynx::color &thick_line2D::color() const
{
    return m_rect.color();
}
void thick_line2D::color(const lynx::color &color)
{
    m_rect.color(color);
    m_circle1.color(color);
    m_circle2.color(color);
}

const kit::transform2D<float> *thick_line2D::parent() const
{
    return m_rect.transform.parent;
}
void thick_line2D::parent(const kit::transform2D<float> *parent)
{
    m_rect.transform.parent = parent;
    m_circle1.transform.parent = parent;
    m_circle2.transform.parent = parent;
}

float thick_line2D::width() const
{
    return m_rect.transform.scale.y;
}
void thick_line2D::width(const float width)
{
    update_width(width);
}
} // namespace ppx