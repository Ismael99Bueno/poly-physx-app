#include "ppx-app/internal/pch.hpp"
#include "ppx-app/drawables/joints/distance_repr.hpp"

namespace ppx
{
distance_repr2D::distance_repr2D(const distance_joint2D *dj, const float sleep_greyout)
    : m_dj(dj), m_line(dj->ganchor1(), dj->ganchor2())
{
    update(sleep_greyout);
}

void distance_repr2D::update(const float sleep_greyout)
{
    m_line.p1(m_dj->ganchor1());
    m_line.p2(m_dj->ganchor2());

    const float stress = m_dj->constraint_position() * 6.f;
    const lynx::gradient<3> grad{compress, relax, stretch};
    const lynx::color color = grad.evaluate(std::clamp(0.5f * (stress + 1.f), 0.f, 1.f));
    m_line.color(m_dj->asleep() ? sleep_greyout * color : color);
}

void distance_repr2D::draw(lynx::window2D &window) const
{
    window.draw(m_line);
}

} // namespace ppx