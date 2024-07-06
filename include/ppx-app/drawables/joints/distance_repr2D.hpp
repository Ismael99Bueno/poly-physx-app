#pragma once

#include "ppx-app/drawables/lines/thick_line2D.hpp"
#include "ppx-app/drawables/joints/joint_repr2D.hpp"
#include "ppx/joints/distance_joint2D.hpp"

namespace ppx
{
class distance_repr2D final : public joint_repr2D
{
  public:
    distance_repr2D(const distance_joint2D *dj);

    static inline lynx::color stretch = lynx::color::blue;
    static inline lynx::color relax = lynx::color::white * 0.8f;
    static inline lynx::color compress = lynx::color::red;

  private:
    const distance_joint2D *m_dj;
    thick_line2D m_line;
    lynx::color m_color1;
    lynx::color m_color2;

    void update(float sleep_greyout) override;
    void draw(lynx::window2D &window) const override;
};
} // namespace ppx