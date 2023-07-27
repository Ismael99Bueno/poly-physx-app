#include "ppx-app/pch.hpp"
#include "ppx-app/app.hpp"
#include "ppx-app/menu_layer.hpp"

#include "lynx/geometry/camera.hpp"
#include "lynx/rendering/buffer.hpp"

#include "ppx/joints/revolute_joint2D.hpp"

namespace ppx
{
app::app(const rk::butcher_tableau &table, const std::size_t allocations, const char *name)
    : lynx::app2D(800, 600, name), m_world(table, allocations)
{
    push_layer<menu_layer>();

    m_window = window();
    m_window->maintain_camera_aspect_ratio(true);
    m_camera = m_window->set_camera<lynx::orthographic2D>(m_window->pixel_aspect(), 50.f);
    m_camera->flip_y_axis();

    m_world.integrator.min_dt(0.0002f);
    m_world.integrator.max_dt(0.006f);
    m_world.integrator.limited_timestep(false);

    add_world_callbacks();
}

void app::add_world_callbacks()
{
    const kit::callback<const body2D::ptr &> add_shape{[this](const body2D::ptr &body) {
        if (const auto *c = body->shape_if<geo::circle>())
        {
            m_shapes.emplace_back(kit::make_scope<lynx::ellipse2D>(c->radius(), body_color));
            return;
        }
        const geo::polygon &poly = body->shape<geo::polygon>();
        m_shapes.emplace_back(kit::make_scope<lynx::polygon2D>(
            std::vector<glm::vec2>(poly.locals().begin(), poly.locals().end()), body_color));
    }};

    const kit::callback<std::size_t> remove_shape{[this](const std::size_t index) {
        m_shapes[index] = std::move(m_shapes.back());
        m_shapes.pop_back();
    }};

    const kit::callback<const spring2D::ptr &> add_spring{[this](const spring2D::ptr &sp) {
        if (sp->has_anchors())
            m_spring_lines.emplace_back(sp->body1()->position() + sp->anchor1(),
                                        sp->body2()->position() + sp->anchor2(), 6, joint_color);
        else
            m_spring_lines.emplace_back(sp->body1()->position(), sp->body2()->position(), 6, joint_color);
    }};
    const kit::callback<const spring2D &> remove_spring{[this](const spring2D &sp) {
        m_spring_lines[sp.index()] = m_spring_lines.back();
        m_spring_lines.pop_back();
    }};

    const kit::callback<constraint2D *> add_revolute{[this](constraint2D *ctr) {
        const auto *rj = dynamic_cast<revolute_joint2D *>(ctr);
        if (!rj)
            return;
        if (rj->has_anchors())
            m_thick_lines.emplace(rj, thick_line(rj->body1()->position() + rj->anchor1(),
                                                 rj->body2()->position() + rj->anchor2(), 1.f, joint_color));
        else
            m_thick_lines.emplace(rj, thick_line(rj->body1()->position(), rj->body2()->position(), 1.f, joint_color));
    }};
    const kit::callback<const constraint2D &> remove_revolute{[this](const constraint2D &ctr) {
        const auto *rj = dynamic_cast<const revolute_joint2D *>(&ctr);
        if (!rj)
            return;
        m_thick_lines.erase(rj);
    }};

    m_world.events.on_body_addition += add_shape;
    m_world.events.on_late_body_removal += remove_shape;

    m_world.events.on_spring_addition += add_spring;
    m_world.events.on_spring_removal += remove_spring;

    m_world.events.on_constraint_addition += add_revolute;
    m_world.events.on_constraint_removal += remove_revolute;
}

void app::on_update(const float ts)
{
    const kit::clock update_clock;
    if (m_sync_timestep)
    {
        static const float min_dt = 1.f / 120.f;
        m_timestep = glm::min(ts, min_dt);
    }

    const kit::clock physics_clock;
    if (!m_paused)
        for (std::uint32_t i = 0; i < integrations_per_frame; i++)
            m_world.raw_forward(m_timestep);
    m_physics_time = physics_clock.elapsed();

    update_entities();
    update_joints();
    move_camera(ts);
    m_update_time = update_clock.elapsed();
}

void app::on_render(const float ts)
{
    const kit::clock draw_clock;
    draw_entities();
    draw_joints();
    m_draw_time = draw_clock.elapsed();
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
        zoom(event.scroll_offset.y, frame_time().as<kit::time::seconds, float>());
    default:
        return false;
    }
    return false;
}

void app::update_entities()
{
    const auto bodies = m_world.bodies();
    for (std::size_t i = 0; i < bodies.unwrap().size(); i++)
    {
        const body2D &body = bodies[i];
        m_shapes[i]->transform.position = body.position();
        m_shapes[i]->transform.rotation = body.rotation();
        on_body_update(body, *m_shapes[i]);
    }
}
void app::update_joints()
{
    const auto springs = m_world.springs();
    for (std::size_t i = 0; i < springs.unwrap().size(); i++)
    {
        const spring2D &sp = springs[i];
        spring_line &spline = m_spring_lines[i];
        if (sp.has_anchors())
        {
            spline.p1(sp.body1()->position() + sp.anchor1());
            spline.p2(sp.body2()->position() + sp.anchor2());
        }
        else
        {
            spline.p1(sp.body1()->position());
            spline.p2(sp.body2()->position());
        }
    }

    for (auto &[rj, thline] : m_thick_lines)
    {
        if (rj->has_anchors())
        {
            thline.p1(rj->body1()->position() + rj->anchor1());
            thline.p2(rj->body2()->position() + rj->anchor2());
        }
        else
        {
            thline.p1(rj->body1()->position());
            thline.p2(rj->body2()->position());
        }
    }
}

void app::draw_entities() const
{
    for (const auto &shape : m_shapes)
        m_window->draw(*shape);
}

void app::draw_joints() const
{
    for (const spring_line &spline : m_spring_lines)
        m_window->draw(spline);
    for (const auto &[rj, thline] : m_thick_lines)
        m_window->draw(thline);
}

void app::move_camera(const float ts)
{
    glm::vec2 dpos{0.f};
    if (lynx::input::key_pressed(lynx::input::key::A))
        dpos.x = -1.f;
    if (lynx::input::key_pressed(lynx::input::key::D))
        dpos.x = 1.f;
    if (lynx::input::key_pressed(lynx::input::key::W))
        dpos.y = 1.f;
    if (lynx::input::key_pressed(lynx::input::key::S))
        dpos.y = -1.f;
    if (glm::length2(dpos) > std::numeric_limits<float>::epsilon())
        m_camera->transform.position += 2.f * glm::normalize(dpos) * ts * m_camera->size();
}

void app::zoom(const float offset, const float ts)
{
    const float factor = 4.f * offset * ts; // glm::clamp(offset, -0.05f, 0.05f);
    const glm::vec2 mpos = world_mouse_position();
    const glm::vec2 dpos = (mpos - m_camera->transform.position) * factor;
    const float size = m_camera->size() * (1.f - factor);

    m_camera->size(size);
    m_camera->transform.position += dpos;
}

world2D &app::world()
{
    return m_world;
}
const world2D &app::world() const
{
    return m_world;
}

float app::timestep() const
{
    return m_timestep;
}
void app::timestep(float ts)
{
    m_timestep = ts;
}

bool app::sync_timestep() const
{
    return m_sync_timestep;
}
void app::sync_timestep(const bool sync)
{
    m_sync_timestep = sync;
}

kit::time app::update_time() const
{
    return m_update_time;
}
kit::time app::physics_time() const
{
    return m_physics_time;
}
kit::time app::draw_time() const
{
    return m_draw_time;
}

glm::vec2 app::world_mouse_position() const
{
    const glm::vec2 mpos = lynx::input::mouse_position();
    return m_camera->screen_to_world(mpos);
}

#ifdef KIT_USE_YAML_CPP
YAML::Node app::encode() const
{
    YAML::Node node;
    node["Engine"] = m_world;
    node["Timestep"] = m_timestep;
    for (const auto &l : layers())
        node["Layers"][l->id()] = *l;
    for (const auto &shape : m_shapes)
        node["Shape colors"].push_back(shape->color());
    node["Paused"] = m_paused;
    node["Sync timestep"] = m_sync_timestep;
    node["Body color"] = body_color;
    node["Joints color"] = joint_color;
    node["Integrations per frame"] = integrations_per_frame;
    node["Framerate"] = framerate_cap();
    node["Camera position"] = m_camera->transform.position;
    node["Camera scale"] = m_camera->transform.scale;
    node["Camera rotation"] = m_camera->transform.rotation;
    return node;
}
bool app::decode(const YAML::Node &node)
{
    if (!node.IsMap() || node.size() < 11)
        return false;

    m_shapes.clear();
    node["Engine"].as<ppx::world2D>(m_world);
    m_timestep = node["Timestep"].as<float>();

    if (node["Layers"])
        for (const auto &l : layers())
            if (node["Layers"][l->id()])
                node["Layers"][l->id()].as<lynx::layer>(*l);

    if (node["Shape colors"])
        for (std::size_t i = 0; i < m_shapes.size(); i++)
            m_shapes[i]->color(node["Shape colors"][i].as<glm::vec4>());

    m_paused = node["Paused"].as<bool>();
    m_sync_timestep = node["Sync timestep"].as<bool>();
    body_color = node["Body color"].as<glm::vec4>();
    joint_color = node["Joints color"].as<glm::vec4>();
    integrations_per_frame = node["Integrations per frame"].as<std::uint32_t>();
    limit_framerate(node["Framerate"].as<std::uint32_t>());

    m_camera->transform.position = node["Camera position"].as<glm::vec2>();
    m_camera->transform.scale = node["Camera scale"].as<glm::vec2>();
    m_camera->transform.rotation = node["Camera rotation"].as<float>();
    return true;
}
#endif

} // namespace ppx
