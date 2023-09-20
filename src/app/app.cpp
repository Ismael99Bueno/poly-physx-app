#include "ppx-app/internal/pch.hpp"
#include "ppx-app/app/app.hpp"
#include "ppx-app/app/menu_layer.hpp"
#include "lynx/geometry/camera.hpp"
#include "ppx/joints/revolute_joint2D.hpp"
#include "kit/utility/utils.hpp"

namespace ppx
{
app::app(const rk::butcher_tableau &table, const std::size_t allocations, const char *name) : world(table, allocations)
{
    m_window = set_window<lynx::window2D>(800, 600, name);
    push_layer<menu_layer>();

    m_window->maintain_camera_aspect_ratio(true);
    m_camera = m_window->set_camera<lynx::orthographic2D>(m_window->pixel_aspect(), 50.f);
    m_camera->flip_y_axis();

    world.integrator.min_timestep = 0.0002f;
    world.integrator.max_timestep = 0.006f;

    add_world_callbacks();
}

void app::add_world_callbacks()
{
    const kit::callback<const body2D::ptr &> add_shape{[this](const body2D::ptr &body) {
        if (const auto *c = body->shape_if<geo::circle>())
        {
            m_shapes.emplace_back(kit::make_scope<lynx::ellipse2D>(c->radius, body_color))
                ->outline_color(body_outline_color);
            return;
        }
        const geo::polygon &poly = body->shape<geo::polygon>();
        m_shapes.emplace_back(kit::make_scope<lynx::polygon2D>(poly.locals(), body_color))
            ->outline_color(body_outline_color);
    }};

    const kit::callback<std::size_t> remove_shape{[this](const std::size_t index) {
        m_shapes[index] = std::move(m_shapes.back());
        m_shapes.pop_back();
    }};

    const kit::callback<const spring2D::ptr &> add_spring{[this](const spring2D::ptr &sp) {
        if (sp->has_anchors())
            m_spring_lines.emplace_back(sp->body1()->transform().position + sp->rotated_anchor1(),
                                        sp->body2()->transform().position + sp->rotated_anchor2(), joint_color);
        else
            m_spring_lines.emplace_back(sp->body1()->transform().position, sp->body2()->transform().position,
                                        joint_color);
    }};
    const kit::callback<const spring2D &> remove_spring{[this](const spring2D &sp) {
        m_spring_lines[sp.index] = m_spring_lines.back();
        m_spring_lines.pop_back();
    }};

    const kit::callback<constraint2D *> add_revolute{[this](constraint2D *ctr) {
        const auto *rj = dynamic_cast<revolute_joint2D *>(ctr);
        if (!rj)
            return;
        if (rj->has_anchors())
            m_revolute_lines.emplace(rj, thick_line(rj->body1()->transform().position + rj->rotated_anchor1(),
                                                    rj->body2()->transform().position + rj->rotated_anchor2(),
                                                    joint_color));
        else
            m_revolute_lines.emplace(
                rj, thick_line(rj->body1()->transform().position, rj->body2()->transform().position, joint_color));
    }};
    const kit::callback<const constraint2D &> remove_revolute{[this](const constraint2D &ctr) {
        const auto *rj = dynamic_cast<const revolute_joint2D *>(&ctr);
        if (!rj)
            return;
        m_revolute_lines.erase(rj);
    }};

    world.events.on_body_addition += add_shape;
    world.events.on_late_body_removal += remove_shape;

    world.events.on_spring_addition += add_spring;
    world.events.on_spring_removal += remove_spring;

    world.events.on_constraint_addition += add_revolute;
    world.events.on_constraint_removal += remove_revolute;
}

void app::on_update(const float ts)
{
    const kit::clock update_clock;
    timestep = glm::clamp(sync_timestep ? ts : timestep, world.integrator.min_timestep, world.integrator.max_timestep);

    {
        KIT_PERF_SCOPE("-Physics-")
        const kit::clock physics_clock;
        if (!paused)
            for (std::uint32_t i = 0; i < integrations_per_frame; i++)
                world.raw_forward(timestep);
        m_physics_time = physics_clock.elapsed();
    }

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
            return true;
        case lynx::input::key::SPACE:
            paused = !paused;
            return true;
        default:
            return false;
        }
    case lynx::event::SCROLLED:
        if (ImGui::GetIO().WantCaptureMouse)
            break;
        zoom(event.scroll_offset.y, frame_time().as<kit::time::seconds, float>());
        return true;
    default:
        return false;
    }
    return false;
}

void app::update_entities()
{
    const auto bodies = world.bodies();
    for (std::size_t i = 0; i < bodies.unwrap().size(); i++)
    {
        const body2D &body = bodies[i];
        const kit::transform2D &transform = body.transform();

        m_shapes[i]->transform.position = transform.position;
        m_shapes[i]->transform.rotation = transform.rotation;
        on_body_update(body, *m_shapes[i]);
    }
}
void app::update_joints()
{
    const auto springs = world.springs();
    for (std::size_t i = 0; i < springs.unwrap().size(); i++)
    {
        const spring2D &sp = springs[i];
        spring_line &spline = m_spring_lines[i];
        if (sp.has_anchors())
        {
            spline.p1(sp.body1()->transform().position + sp.rotated_anchor1());
            spline.p2(sp.body2()->transform().position + sp.rotated_anchor2());
        }
        else
        {
            spline.p1(sp.body1()->transform().position);
            spline.p2(sp.body2()->transform().position);
        }
    }

    for (auto &[rj, thline] : m_revolute_lines)
    {
        if (rj->has_anchors())
        {
            thline.p1(rj->body1()->transform().position + rj->rotated_anchor1());
            thline.p2(rj->body2()->transform().position + rj->rotated_anchor2());
        }
        else
        {
            thline.p1(rj->body1()->transform().position);
            thline.p2(rj->body2()->transform().position);
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
    for (const auto &[rj, thline] : m_revolute_lines)
        m_window->draw(thline);
}

void app::move_camera(const float ts)
{
    if (ImGui::GetIO().WantCaptureKeyboard)
        return;
    glm::vec2 dpos{0.f};
    if (lynx::input::key_pressed(lynx::input::key::A))
        dpos.x = -1.f;
    if (lynx::input::key_pressed(lynx::input::key::D))
        dpos.x = 1.f;
    if (lynx::input::key_pressed(lynx::input::key::W))
        dpos.y = 1.f;
    if (lynx::input::key_pressed(lynx::input::key::S))
        dpos.y = -1.f;
    if (!kit::approaches_zero(glm::length2(dpos)))
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
const std::vector<kit::scope<lynx::shape2D>> &app::shapes() const
{
    return m_shapes;
}
const std::vector<spring_line> &app::spring_lines() const
{
    return m_spring_lines;
}
const std::unordered_map<const revolute_joint2D *, thick_line> &app::revolute_lines() const
{
    return m_revolute_lines;
}

#ifdef KIT_USE_YAML_CPP
YAML::Node app::encode() const
{
    YAML::Node node;
    node["Engine"] = world;
    node["Timestep"] = timestep;
    for (const auto &l : layers())
        node["Layers"][l->id] = *l;
    for (const auto &shape : m_shapes)
        node["Shape colors"].push_back(shape->color().normalized);
    node["Paused"] = paused;
    node["Sync timestep"] = sync_timestep;
    node["Body color"] = body_color.normalized;
    node["Joints color"] = joint_color.normalized;
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

    node["Engine"].as<ppx::world2D>(world);
    timestep = node["Timestep"].as<float>();

    if (node["Layers"])
        for (const auto &l : layers())
            if (node["Layers"][l->id])
                node["Layers"][l->id].as<lynx::layer>(*l);

    if (node["Shape colors"])
        for (std::size_t i = 0; i < m_shapes.size(); i++)
            m_shapes[i]->color(lynx::color(node["Shape colors"][i].as<glm::vec4>()));

    paused = node["Paused"].as<bool>();
    sync_timestep = node["Sync timestep"].as<bool>();
    body_color = lynx::color(node["Body color"].as<glm::vec4>());
    joint_color = lynx::color(node["Joints color"].as<glm::vec4>());
    integrations_per_frame = node["Integrations per frame"].as<std::uint32_t>();
    limit_framerate(node["Framerate"].as<std::uint32_t>());

    m_camera->transform.position = node["Camera position"].as<glm::vec2>();
    m_camera->transform.scale = node["Camera scale"].as<glm::vec2>();
    m_camera->transform.rotation = node["Camera rotation"].as<float>();
    return true;
}
#endif

} // namespace ppx
