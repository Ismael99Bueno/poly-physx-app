#pragma once

#include "ppx/joints/prismatic_joint2D.hpp"
#include "lynx/drawing/line.hpp"
#include "ppx-app/drawables/joints/joint_repr2D.hpp"
#include "lynx/app/window.hpp"

namespace ppx
{
class prismatic_repr2D : public joint_repr2D
{
  public:
    prismatic_repr2D(const prismatic_joint2D *pj, const lynx::color &color);

  private:
    const prismatic_joint2D *m_pj;
    lynx::thin_line2D m_line;
    lynx::color m_color;

    void update(float sleep_greyout) override;
    void draw(lynx::window2D &window) const override;
};
} // namespace ppx