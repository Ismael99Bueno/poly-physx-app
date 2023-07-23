#include "ppx-app/pch.hpp"
#include "ppx-app/thick_line.hpp"
#include "lynx/app/window.hpp"
#include "lynx/rendering/buffer.hpp"

namespace ppx
{
thick_line::thick_line(const glm::vec2 &p1, const glm::vec2 &p2, const float width, const glm::vec4 &color)
    : m_rect(color), m_circle1(color), m_circle2(color)
{
    m_circle1.transform.position = p1;
    m_circle2.transform.position = p2;

    update_width(width);
    update_orientation();
}

void thick_line::update_width(const float width)
{
    m_circle1.radius(0.5f * width);
    m_circle2.radius(0.5f * width);

    m_rect.transform.scale.y = width;
}

void thick_line::update_length()
{
    m_rect.transform.scale.x = glm::distance(m_circle1.transform.position, m_circle2.transform.position);
}

void thick_line::update_orientation()
{
    const glm::vec2 &p1 = m_circle1.transform.position;
    const glm::vec2 &p2 = m_circle2.transform.position;

    m_rect.transform.position = 0.5f * (p1 + p2);

    const glm::vec2 dp = p2 - p1;
    m_rect.transform.rotation = atan2f(dp.y, dp.x);
}

void thick_line::draw(lynx::window2D &window) const
{
    window.draw(m_rect);
    window.draw(m_circle1);
    window.draw(m_circle2);
}

const glm::vec2 &thick_line::p1() const
{
    return m_circle1.transform.position;
}
const glm::vec2 &thick_line::p2() const
{
    return m_circle2.transform.position;
}

void thick_line::p1(const glm::vec2 &p1)
{
    m_circle1.transform.position = p1;
    update_length();
    update_orientation();
}
void thick_line::p2(const glm::vec2 &p2)
{
    m_circle2.transform.position = p2;
    update_length();
    update_orientation();
}

const glm::vec4 &thick_line::color() const
{
    return m_rect.color();
}
void thick_line::color(const glm::vec4 &color)
{
    m_rect.color(color);
    m_circle1.color(color);
    m_circle2.color(color);
}

float thick_line::width() const
{
    return m_rect.transform.scale.y;
}
void thick_line::width(const float width)
{
    update_width(width);
}
} // namespace ppx