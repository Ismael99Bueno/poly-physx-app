#include "ppx-app/internal/pch.hpp"
#include "ppx-app/lines/spring_line2D.hpp"
#include "lynx/app/window.hpp"

namespace ppx
{
spring_line2D::spring_line2D(const glm::vec2 &p1, const glm::vec2 &p2, const lynx::color &color,
                             const std::size_t supports_count)
    : m_supports_count(supports_count), m_line_strip(build_line_points(p1, p2), color)
{
}
spring_line2D::spring_line2D(const lynx::color &color, const std::size_t supports_count)
    : spring_line2D({0.f, 0.f}, {1.f, 0.f}, color, supports_count)
{
}

std::vector<glm::vec2> spring_line2D::build_line_points(const glm::vec2 &p1, const glm::vec2 &p2)
{
    std::vector<glm::vec2> points = {p1, p2};
    points.resize(3 + 4 * m_supports_count);

    const glm::vec2 segment = p2 - p1;
    const float base_length = (glm::length(segment) - m_left_padding - m_right_padding) / m_supports_count,
                angle = atan2f(segment.y, segment.x);

    glm::vec2 ref1 = p1 + glm::normalize(segment) * m_left_padding,
              ref2 = p2 - glm::normalize(segment) * m_right_padding;
    points[2] = ref1;
    for (std::size_t i = 0; i < m_supports_count; i++)
    {
        const float y =
            sqrtf(std::max(m_min_height, m_supports_length * m_supports_length - base_length * base_length));
        const glm::vec2 side1 = glm::rotate(glm::vec2(0.5f * base_length, y), angle),
                        side2 = glm::rotate(glm::vec2(0.5f * base_length, -y), angle);

        const std::size_t idx1 = 3 + 2 * i, idx2 = 3 + 2 * m_supports_count + 2 * i;

        points[idx1] = ref1 + side1;
        points[idx1 + 1] = ref1 + side1 + side2;

        points[idx2] = ref2 - side1;
        points[idx2 + 1] = ref2 - side1 - side2;

        ref1 += side1 + side2;
        ref2 -= side1 + side2;
    }

    return points;
}

void spring_line2D::update_line_points(const glm::vec2 &p1, const glm::vec2 &p2)
{
    const std::vector<glm::vec2> points = build_line_points(p1, p2);
    for (std::size_t i = 0; i < points.size(); i++)
        m_line_strip[i].position = points[i];
}

void spring_line2D::draw(lynx::window2D &window) const
{
    window.draw(m_line_strip);
}

const glm::vec2 &spring_line2D::p1() const
{
    return m_line_strip[0].position;
}
const glm::vec2 &spring_line2D::p2() const
{
    return m_line_strip[1].position;
}

void spring_line2D::p1(const glm::vec2 &p1)
{
    update_line_points(p1, p2());
}
void spring_line2D::p2(const glm::vec2 &p2)
{
    update_line_points(p1(), p2);
}

const lynx::color &spring_line2D::color() const
{
    return m_line_strip.color();
}
void spring_line2D::color(const lynx::color &color)
{
    m_line_strip.color(color);
}

const kit::transform2D<float> *spring_line2D::parent() const
{
    return m_line_strip.parent();
}
void spring_line2D::parent(const kit::transform2D<float> *parent)
{
    m_line_strip.parent(parent);
}

std::size_t spring_line2D::supports_count() const
{
    return m_supports_count;
}
float spring_line2D::supports_length() const
{
    return m_supports_length;
}
float spring_line2D::left_padding() const
{
    return m_left_padding;
}
float spring_line2D::right_padding() const
{
    return m_right_padding;
}
float spring_line2D::min_height() const
{
    return m_min_height;
}

void spring_line2D::supports_length(const float supports_length)
{
    m_supports_length = supports_length;
    update_line_points(p1(), p2());
}
void spring_line2D::left_padding(const float left_padding)
{
    m_left_padding = left_padding;
    update_line_points(p1(), p2());
}
void spring_line2D::right_padding(const float right_padding)
{
    m_right_padding = right_padding;
    update_line_points(p1(), p2());
}
void spring_line2D::min_height(const float min_height)
{
    m_min_height = min_height;
    update_line_points(p1(), p2());
}
} // namespace ppx