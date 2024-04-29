#include "ppx-app/internal/pch.hpp"
#include "ppx-app/drawables/joints/prismatic_repr2D.hpp"

namespace ppx
{
prismatic_repr2D::prismatic_repr2D(const prismatic_joint2D *pj, const lynx::color &color)
    : m_pj(pj), m_line(pj->ganchor1(), pj->ganchor2(), color)
{
}

void prismatic_repr2D::update()
{
    m_line.p1(m_pj->ganchor1());
    m_line.p2(m_pj->ganchor2());
}

void prismatic_repr2D::draw(lynx::window2D &window) const
{
    window.draw(m_line);
}

} // namespace ppx