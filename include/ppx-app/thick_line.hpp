#ifndef PPX_THICK_LINE_HPP
#define PPX_THICK_LINE_HPP

#include "lynx/drawing/drawable.hpp"
#include "lynx/drawing/shape.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace ppx
{
class thick_line : public lynx::drawable2D
{
  public:
    thick_line(const glm::vec2 &p1, const glm::vec2 &p2, float width = 1.f, const glm::vec4 &color = glm::vec4(1.f));

    void draw(lynx::window2D &window) const override;

    const glm::vec2 &p1() const;
    const glm::vec2 &p2() const;

    void p1(const glm::vec2 &p1);
    void p2(const glm::vec2 &p2);

    const glm::vec4 &color() const;
    void color(const glm::vec4 &color);

    float width() const;
    void width(float width);

  private:
    lynx::rect2D m_rect;
    lynx::ellipse2D m_circle1;
    lynx::ellipse2D m_circle2;

    void update_width(float width);
    void update_orientation();
};
} // namespace ppx

#endif