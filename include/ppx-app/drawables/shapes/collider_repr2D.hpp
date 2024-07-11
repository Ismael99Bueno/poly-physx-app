#pragma once

#include "ppx/collider/collider2D.hpp"
#include "lynx/drawing/color.hpp"
#include "lynx/drawing/shape.hpp"
#include "lynx/app/window.hpp"

namespace ppx
{
class collider_repr2D final : public lynx::drawable2D
{
  public:
    collider_repr2D(collider2D *collider, const lynx::color &color, float sleep_greyout);
    collider2D *collider;
    kit::scope<lynx::shape2D> shape;
    lynx::color color;

    void update(float sleep_greyout);
    void draw(lynx::window2D &window) const override;
};
} // namespace ppx