#include "ppx-app/internal/pch.hpp"
#include "ppx-app/drawables/joints/distance_repr2D.hpp"

namespace ppx
{
distance_repr2D::distance_repr2D(const distance_joint2D *dj, const lynx::color &color1, const lynx::color &color2)
    : m_dj(dj), m_line(dj->ganchor1(), dj->ganchor2()), m_color1(color1), m_color2(color2)
{
}

void distance_repr2D::update()
{
    m_line.p1(m_dj->ganchor1());
    m_line.p2(m_dj->ganchor2());

    const float stress = m_dj->constraint_position() * 2.f;
    const lynx::gradient<3> grad{m_color1, lynx::color{glm::vec3(0.8f)}, m_color2};
    const lynx::color color = grad.evaluate(std::clamp(0.5f * (stress + 1.f), 0.f, 1.f));
    m_line.color(color);
}

void distance_repr2D::draw(lynx::window2D &window) const
{
    window.draw(m_line);
}

} // namespace ppx