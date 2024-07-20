#pragma once

#include "lynx/drawing/drawable.hpp"
#include "lynx/drawing/shape.hpp"
#include "lynx/drawing/line.hpp"
#include "lynx/app/window.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace ppx
{
class thick_line2D final : public lynx::line2D
{
  public:
    thick_line2D(const glm::vec2 &p1 = {0.f, 0.f}, const glm::vec2 &p2 = {1.f, 0.f},
                 const lynx::color &color = lynx::color::white, float width = 1.f);
    thick_line2D(const lynx::color &color, float width = 1.f);

    void draw(lynx::window2D &window) const override;

    const glm::vec2 &p1() const override;
    const glm::vec2 &p2() const override;

    void p1(const glm::vec2 &p1) override;
    void p2(const glm::vec2 &p2) override;

    const lynx::color &color() const override;
    void color(const lynx::color &color) override;

    const kit::transform2D<float> *parent() const;
    void parent(const kit::transform2D<float> *parent);

    float width() const;
    void width(float width);

  private:
    lynx::rect2D m_rect;
    lynx::ellipse2D m_circle1;
    lynx::ellipse2D m_circle2;

    void update_width(float width);
    void update_length();
    void update_orientation();
};
} // namespace ppx
