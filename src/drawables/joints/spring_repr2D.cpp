#include "ppx-app/internal/pch.hpp"
#include "ppx-app/drawables/joints/spring_repr2D.hpp"

namespace ppx
{
spring_repr2D::spring_repr2D(const spring_joint2D *sj, const lynx::color &color, const float sleep_greyout)
    : m_sj(sj), m_line(sj->ganchor1(), sj->ganchor2(), color), m_color(color)
{
    update(sleep_greyout);
}

void spring_repr2D::update(const float sleep_greyout)
{
    m_line.p1(m_sj->ganchor1());
    m_line.p2(m_sj->ganchor2());

    m_line.color(m_sj->asleep() ? sleep_greyout * m_color : m_color);
}

void spring_repr2D::draw(lynx::window2D &window) const
{
    window.draw(m_line);
}

} // namespace ppx