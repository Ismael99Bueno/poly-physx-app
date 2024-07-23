#include "ppx-app/internal/pch.hpp"
#include "ppx-app/drawables/shapes/collider_repr.hpp"
#include "ppx-app/drawables/shapes/oriented_circle.hpp"

namespace ppx
{
collider_repr2D::collider_repr2D(collider2D *collider, const lynx::color &color, const float sleep_greyout)
    : collider(collider), color(color)
{
    if (const auto *c = collider->shape_if<circle>())
        shape = kit::make_scope<oriented_circle>(c->radius(), color);
    else
    {
        const polygon &poly = collider->shape<polygon>();
        const std::vector<glm::vec2> vertices{poly.vertices.model.begin(), poly.vertices.model.end()};
        shape = kit::make_scope<lynx::polygon2D>(vertices, color);
    }
    update(sleep_greyout);
}

void collider_repr2D::update(const float sleep_greyout)
{
    const glm::vec2 scale = shape->transform.scale;
    shape->transform = collider->ltransform();
    shape->transform.scale = scale;
    shape->color(collider->body()->asleep() ? color * sleep_greyout : color);
}

void collider_repr2D::draw(lynx::window2D &window) const
{
    window.draw(*shape);
}

} // namespace ppx