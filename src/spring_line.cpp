#include "ppx-app/pch.hpp"
#include "ppx-app/spring_line.hpp"
#include "lynx/app/window.hpp"
#include "lynx/rendering/buffer.hpp"

namespace ppx
{
spring_line::spring_line(const glm::vec2 &p1, const glm::vec2 &p2, const std::size_t supports_count,
                         const glm::vec4 &color)
    : m_supports_count(supports_count), m_line_strip(build_line_points(p1, p2), color)
{
}

std::vector<glm::vec2> spring_line::build_line_points(const glm::vec2 &p1, const glm::vec2 &p2)
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

void spring_line::update_line_points(const glm::vec2 &p1, const glm::vec2 &p2)
{
    const std::vector<glm::vec2> points = build_line_points(p1, p2);
    std::size_t i = 0;
    const auto feach = [&points, &i](lynx::vertex2D &v) { v.position = points[i++]; };
    m_line_strip.update_points(feach);
}

void spring_line::draw(lynx::window2D &window) const
{
    window.draw(m_line_strip);
}

const glm::vec2 &spring_line::p1() const
{
    return m_line_strip[0].position;
}
const glm::vec2 &spring_line::p2() const
{
    return m_line_strip[1].position;
}

void spring_line::p1(const glm::vec2 &p1)
{
    update_line_points(p1, p2());
}
void spring_line::p2(const glm::vec2 &p2)
{
    update_line_points(p1(), p2);
}

std::size_t spring_line::supports_count() const
{
    return m_supports_count;
}
float spring_line::supports_length() const
{
    return m_supports_length;
}
float spring_line::left_padding() const
{
    return m_left_padding;
}
float spring_line::right_padding() const
{
    return m_right_padding;
}
float spring_line::min_height() const
{
    return m_min_height;
}

void spring_line::supports_length(const float supports_length)
{
    m_supports_length = supports_length;
    update_line_points(p1(), p2());
}
void spring_line::left_padding(const float left_padding)
{
    m_left_padding = left_padding;
    update_line_points(p1(), p2());
}
void spring_line::right_padding(const float right_padding)
{
    m_right_padding = right_padding;
    update_line_points(p1(), p2());
}
void spring_line::min_height(const float min_height)
{
    m_min_height = min_height;
    update_line_points(p1(), p2());
}
} // namespace ppx