#ifndef PPX_SPRING_LINE_HPP
#define PPX_SPRING_LINE_HPP

#include "lynx/drawing/drawable.hpp"
#include "lynx/drawing/line.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace ppx
{
class spring_line : public lynx::drawable2D
{
  public:
    spring_line(const glm::vec2 &p1, const glm::vec2 &p2, std::size_t supports_count = 6,
                const glm::vec4 &color = glm::vec4(1.f));

    void draw(lynx::window2D &window) const override;

    const glm::vec2 &p1() const;
    const glm::vec2 &p2() const;

    void p1(const glm::vec2 &p1);
    void p2(const glm::vec2 &p2);

    std::size_t supports_count() const;
    float supports_length() const;
    float left_padding() const;
    float right_padding() const;
    float min_height() const;

    void supports_length(float supports_length);
    void left_padding(float left_padding);
    void right_padding(float right_padding);
    void min_height(float min_height);

  private:
    std::size_t m_supports_count;
    float m_supports_length = 0.8f;
    float m_left_padding = 0.f;
    float m_right_padding = 0.f;
    float m_min_height = 1.f;

    lynx::line_strip2D m_line_strip;

    std::vector<glm::vec2> build_line_points(const glm::vec2 &p1, const glm::vec2 &p2);
    void update_line_points(const glm::vec2 &p1, const glm::vec2 &p2);
};
} // namespace ppx

#endif