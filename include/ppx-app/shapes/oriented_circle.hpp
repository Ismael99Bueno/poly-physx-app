#pragma once

#include "lynx/app/window.hpp"
#include "lynx/drawing/line.hpp"
#include "lynx/drawing/shape.hpp"

namespace ppx
{
class oriented_circle final : public lynx::ellipse2D
{
  public:
    using lynx::ellipse2D::ellipse2D;

    void draw(lynx::window2D &win) const override;

  private:
    mutable lynx::thin_line2D m_line{glm::vec2(0.f), glm::vec2(1.f, 0.f)};
};
} // namespace ppx