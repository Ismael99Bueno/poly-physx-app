#include "ppx-app/internal/pch.hpp"
#include "ppx-app/app/app.hpp"
#include "ppx-app/serialization/serialization.hpp"

#include "lynx/geometry/camera.hpp"
#include "ppx/joints/distance_joint2D.hpp"
#include "kit/utility/utils.hpp"

namespace ppx
{
void app::add_world_callbacks()
{
    const kit::callback<body2D &> add_shape{[this](body2D &body) {
        if (const auto *c = body.shape_if<geo::circle>())
        {
            auto &shape = m_shapes.emplace_back(kit::make_scope<lynx::ellipse2D>(c->radius, body_color));
            shape->outline_color(body_outline_color);
            shape->transform = body.transform();
            return;
        }
        const geo::polygon &poly = body.shape<geo::polygon>();
        auto &shape = m_shapes.emplace_back(kit::make_scope<lynx::polygon2D>(poly.locals().as_vector(), body_color));
        shape->outline_color(body_outline_color);
        shape->transform = body.transform();
    }};

    const kit::callback<std::size_t> remove_shape{[this](const std::size_t index) {
        m_shapes[index] = std::move(m_shapes.back());
        m_shapes.pop_back();
    }};

    const kit::callback<spring2D &> add_spring{[this](spring2D &sp) {
        m_spring_lines.emplace_back(sp.joint.body1()->position() + sp.joint.rotated_anchor1(),
                                    sp.joint.body2()->position() + sp.joint.rotated_anchor2(), joint_color);
    }};
    const kit::callback<const spring2D &> remove_spring{[this](const spring2D &sp) {
        m_spring_lines[sp.index] = m_spring_lines.back();
        m_spring_lines.pop_back();
    }};

    const kit::callback<constraint2D *> add_dist_joint{[this](constraint2D *ctr) {
        const auto *dj = dynamic_cast<distance_joint2D *>(ctr);
        if (!dj)
            return;
        m_dist_joint_lines.emplace(dj, thick_line(dj->joint.body1()->position() + dj->joint.rotated_anchor1(),
                                                  dj->joint.body2()->position() + dj->joint.rotated_anchor2(),
                                                  joint_color));
    }};
    const kit::callback<const constraint2D &> remove_dist_joint{[this](const constraint2D &ctr) {
        const auto *dj = dynamic_cast<const distance_joint2D *>(&ctr);
        if (!dj)
            return;
        m_dist_joint_lines.erase(dj);
    }};

    world.events.on_body_addition += add_shape;
    world.events.on_late_body_removal += remove_shape;

    world.events.on_spring_addition += add_spring;
    world.events.on_early_spring_removal += remove_spring;

    world.events.on_constraint_addition += add_dist_joint;
    world.events.on_constraint_removal += remove_dist_joint;
}

void app::on_update(const float ts)
{
    {
        KIT_PERF_SCOPE("PPX-APP:Physics")
        const kit::clock physics_clock;

        if (sync_timestep)
            world.integrator.ts.value = ts;
        if (world.integrator.ts.limited)
            world.integrator.ts.clamp();

        if (!paused)
            for (std::uint32_t i = 0; i < integrations_per_frame; i++)
                world.step();
        m_physics_time = physics_clock.elapsed();
    }
    update_shapes();
    update_joints();
    move_camera(ts);
}

void app::on_render(const float ts)
{
    draw_shapes();
    draw_joints();
}

bool app::on_event(const lynx::event2D &event)
{
    switch (event.type)
    {
    case lynx::event2D::KEY_PRESSED:
        if (ImGui::GetIO().WantCaptureKeyboard)
            break;
        switch (event.key)
        {
        case lynx::input2D::key::ESCAPE:
            shutdown();
            return true;
        case lynx::input2D::key::P:
            paused = !paused;
            return true;
        default:
            return false;
        }
    case lynx::event2D::SCROLLED:
        if (ImGui::GetIO().WantCaptureMouse)
            break;
        zoom(event.scroll_offset.y, frame_time().as<kit::time::seconds, float>());
        return true;
    default:
        return false;
    }
    return false;
}

void app::update_shapes()
{
    for (std::size_t i = 0; i < world.bodies.size(); i++)
    {
        const body2D &body = world.bodies[i];
        const kit::transform2D<float> &transform = body.transform();

        m_shapes[i]->transform.position = transform.position;
        m_shapes[i]->transform.rotation = transform.rotation;
        on_body_update(body, *m_shapes[i]);
    }
}
void app::update_joints()
{
    for (std::size_t i = 0; i < world.springs.size(); i++)
    {
        const spring2D &sp = world.springs[i];
        spring_line &spline = m_spring_lines[i];

        spline.p1(sp.joint.body1()->position() + sp.joint.rotated_anchor1());
        spline.p2(sp.joint.body2()->position() + sp.joint.rotated_anchor2());
    }

    for (auto &[dj, thline] : m_dist_joint_lines)
    {
        thline.p1(dj->joint.body1()->position() + dj->joint.rotated_anchor1());
        thline.p2(dj->joint.body2()->position() + dj->joint.rotated_anchor2());
    }
}

void app::draw_shapes() const
{
    for (const auto &shape : m_shapes)
        m_window->draw(*shape);
}

void app::draw_joints() const
{
    for (const spring_line &spline : m_spring_lines)
        m_window->draw(spline);
    for (const auto &[dj, thline] : m_dist_joint_lines)
        m_window->draw(thline);
}

void app::move_camera(const float ts)
{
    if (ImGui::GetIO().WantCaptureKeyboard || lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL) ||
        lynx::input2D::key_pressed(lynx::input2D::key::LEFT_SHIFT))
        return;
    glm::vec2 dpos{0.f};
    if (lynx::input2D::key_pressed(lynx::input2D::key::A))
        dpos.x = -1.f;
    if (lynx::input2D::key_pressed(lynx::input2D::key::D))
        dpos.x = 1.f;
    if (lynx::input2D::key_pressed(lynx::input2D::key::W))
        dpos.y = 1.f;
    if (lynx::input2D::key_pressed(lynx::input2D::key::S))
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

kit::time app::physics_time() const
{
    return m_physics_time;
}

glm::vec2 app::world_mouse_position() const
{
    const glm::vec2 mpos = lynx::input2D::mouse_position();
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
const std::unordered_map<const distance_joint2D *, thick_line> &app::dist_joint_lines() const
{
    return m_dist_joint_lines;
}

#ifdef KIT_USE_YAML_CPP
YAML::Node app::encode() const
{
    return kit::yaml::codec<app>::encode(*this);
}
bool app::decode(const YAML::Node &node)
{
    return kit::yaml::codec<app>::decode(node, *this);
}
#endif

} // namespace ppx
