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
    world.colliders.events.on_addition += [this](collider2D &collider) {
        if (const auto *c = collider.shape_if<circle>())
        {
            auto &shape = m_shapes.emplace_back(kit::make_scope<lynx::ellipse2D>(c->radius(), collider_color));
            shape->outline_color(collider_outline_color);
            return;
        }
        const polygon &poly = collider.shape<polygon>();
        const std::vector<glm::vec2> vertices{poly.vertices.model.begin(), poly.vertices.model.end()};

        auto &shape = m_shapes.emplace_back(kit::make_scope<lynx::polygon2D>(vertices, collider_color));
        shape->outline_color(collider_outline_color);
    };

    world.colliders.events.on_late_removal +=
        [this](const std::size_t index) { m_shapes.erase(m_shapes.begin() + index); };

    world.springs.events.on_addition += [this](spring2D &sp) {
        m_spring_lines.emplace_back(sp.joint.body1()->centroid() + sp.joint.rotated_anchor1(),
                                    sp.joint.body2()->centroid() + sp.joint.rotated_anchor2(), joint_color);
    };

    world.springs.events.on_late_removal += [this](const std::size_t index) {
        m_spring_lines[index] = m_spring_lines.back();
        m_spring_lines.pop_back();
    };

    world.constraints.events.on_addition += [this](constraint2D *ctr) {
        const auto *dj = dynamic_cast<distance_joint2D *>(ctr);
        if (!dj)
            return;
        m_dist_joint_lines.emplace(dj, thick_line(dj->joint.body1()->centroid() + dj->joint.rotated_anchor1(),
                                                  dj->joint.body2()->centroid() + dj->joint.rotated_anchor2(),
                                                  joint_color));
    };

    world.constraints.events.on_removal += [this](const constraint2D &ctr) {
        const auto *dj = dynamic_cast<const distance_joint2D *>(&ctr);
        if (!dj)
            return;
        m_dist_joint_lines.erase(dj);
    };
}

void app::on_update(const float ts)
{
    {
        KIT_PERF_SCOPE("PPX-APP:Physics")
        const kit::perf::clock physics_clock;

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
        zoom(event.scroll_offset.y, frame_time().as<kit::perf::time::seconds, float>());
        return true;
    default:
        return false;
    }
    return false;
}

void app::update_shapes()
{
    for (std::size_t i = 0; i < world.colliders.size(); i++)
        m_shapes[i]->transform = world.colliders[i].ltransform();
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
        const float stress = dj->constraint_value() * 5.f;
        const lynx::gradient<3> grad{lynx::color::blue, lynx::color{glm::vec3(0.8f)}, lynx::color::red};
        const lynx::color color = grad.evaluate(std::clamp(0.5f * (stress + 1.f), 0.f, 1.f));

        thline.p1(dj->joint.body1()->position() + dj->joint.rotated_anchor1());
        thline.p2(dj->joint.body2()->position() + dj->joint.rotated_anchor2());
        thline.color(color);
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

kit::perf::time app::physics_time() const
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
