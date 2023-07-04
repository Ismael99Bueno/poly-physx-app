#include "ppx-app/pch.hpp"
#include "ppx-app/app.hpp"
#include "ppx-app/menu_layer.hpp"
#include "lynx/geometry/camera.hpp"

namespace ppx
{
app::app(const rk::butcher_tableau &table, const std::size_t allocations, const char *name)
    : lynx::app2D(800, 600, name), m_engine(table, allocations)
{
    push_layer<menu_layer>();

    m_window = window();
    m_window->set_camera<lynx::orthographic2D>(glm::vec2(m_window->swap_chain_aspect() * 50.f, -50.f));

    m_engine.integrator().min_dt(0.0002f);
    m_engine.integrator().max_dt(0.006f);
    m_engine.integrator().limited_timestep(false);

    const auto add_shape = [this](const entity2D_ptr &e) {
        if (const auto *c = e->shape_if<geo::circle>())
        {
            m_shapes.emplace_back(make_scope<lynx::ellipse2D>(c->radius()));
            return;
        }
        const geo::polygon &poly = e->shape<geo::polygon>();
        m_shapes.emplace_back(
            make_scope<lynx::polygon2D>(std::vector<glm::vec2>(poly.locals().begin(), poly.locals().end())));
    };

    const auto remove_shape = [this](const std::size_t index) {
        m_shapes[index] = std::move(m_shapes.back());
        m_shapes.pop_back();
    };

    m_engine.events().on_entity_addition += add_shape;
    m_engine.events().on_late_entity_removal += remove_shape;
}

void app::on_update(float ts)
{
    m_engine.raw_forward(ts);
    update_and_draw_entities();
}

void app::update_and_draw_entities()
{
    for (std::size_t i = 0; i < m_shapes.size(); i++)
    {
        const entity2D_ptr e = m_engine[i];

        m_shapes[i]->transform.position = e->pos();
        m_shapes[i]->transform.rotation = e->angpos();
        on_entity_draw(e, *m_shapes[i]);
        m_window->draw(*m_shapes[i]);
    }
}

engine2D &app::engine()
{
    return m_engine;
}
const engine2D &app::engine() const
{
    return m_engine;
}
} // namespace ppx
