#pragma once

#include "ppx-app/drawables/lines/spring_line2D.hpp"
#include "ppx-app/drawables/joints/joint_repr2D.hpp"
#include "ppx/joints/spring_joint2D.hpp"

namespace ppx
{
class spring_repr2D : public joint_repr2D
{
  public:
    spring_repr2D(const spring_joint2D *sj, const lynx::color &color);

  private:
    const spring_joint2D *m_sj;
    spring_line2D m_line;

    void update() override;
    void draw(lynx::window2D &window) const override;
};
} // namespace ppx