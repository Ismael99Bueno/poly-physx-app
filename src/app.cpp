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
    m_camera = m_window->set_camera<lynx::orthographic2D>(glm::vec2(m_window->swap_chain_aspect() * 50.f, -50.f));

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
    if (!m_paused)
        m_engine.raw_forward(ts);
    update_entities();
    move_camera();
}

void app::on_render(const float ts)
{
    draw_entities();
}

bool app::on_event(const lynx::event &event)
{
    switch (event.type)
    {
    case lynx::event::KEY_PRESSED:
        if (ImGui::GetIO().WantCaptureKeyboard)
            break;
        switch (event.key)
        {
        case lynx::input::key::ESCAPE:
            shutdown();
            return false;
        case lynx::input::key::SPACE:
            m_paused = !m_paused;
            return false;
        default:
            return false;
        }
    case lynx::event::SCROLLED:
        if (ImGui::GetIO().WantCaptureMouse)
            break;
        zoom(event.scroll_offset.y);
    default:
        return false;
    }
    return false;
}

void app::update_entities()
{
    for (std::size_t i = 0; i < m_shapes.size(); i++)
    {
        const entity2D_ptr e = m_engine[i];
        m_shapes[i]->transform.position = e->pos();
        m_shapes[i]->transform.rotation = e->angpos();
        on_entity_draw(e, *m_shapes[i]);
    }
}

void app::draw_entities() const
{
    for (const auto &shape : m_shapes)
        m_window->draw(*shape);
}

void app::move_camera()
{
    glm::vec2 dpos{0.f};
    if (lynx::input::key_pressed(lynx::input::key::A))
        dpos.x = -0.8f;
    if (lynx::input::key_pressed(lynx::input::key::D))
        dpos.x = 0.8f;
    if (lynx::input::key_pressed(lynx::input::key::W))
        dpos.y = 0.8f;
    if (lynx::input::key_pressed(lynx::input::key::S))
        dpos.y = -0.8f;
    if (glm::length2(dpos) > std::numeric_limits<float>::epsilon())
        m_camera->transform.position += glm::normalize(dpos);
}

void app::zoom(const float offset)
{
    const float factor = offset * 0.05f; // glm::clamp(offset, -0.05f, 0.05f);
    const glm::vec2 mpos = mouse_position();
    const glm::vec2 dpos = (mpos - m_camera->transform.position) * factor;
    const float size = m_camera->size() * (1.f - factor);

    m_camera->transform.scale.x = -m_window->swap_chain_aspect() * size;
    m_camera->transform.scale.y = size;
    m_camera->transform.position += dpos;
}

glm::vec2 app::mouse_position() const
{
    return m_camera->screen_to_world(lynx::input::mouse_position());
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
