#include "ppx-app/internal/pch.hpp"
#include "ppx-app/app/app.hpp"
#include "ppx-app/serialization/serialization.hpp"
#include "ppx-app/drawables/shapes/oriented_circle.hpp"
#include "ppx-app/drawables/joints/distance_repr2D.hpp"
#include "ppx-app/drawables/joints/spring_repr2D.hpp"
#include "ppx-app/drawables/joints/prismatic_repr2D.hpp"

#include "lynx/geometry/camera.hpp"
#include "ppx/joints/distance_joint2D.hpp"
#include "kit/utility/utils.hpp"

namespace ppx
{
app::app(const specs &spc) : lynx::app2D(spc.window), world(spc.world)
{
    world.add_builtin_joint_managers();
    m_window = window();
    push_layer<menu_layer>();

    m_window->maintain_camera_aspect_ratio(true);
    m_camera = m_window->set_camera<lynx::orthographic2D>(m_window->pixel_aspect(), 50.f);
    m_camera->flip_y_axis();

    add_world_callbacks();
}

void app::add_world_callbacks()
{
    world.colliders.events.on_addition += [this](collider2D *collider) {
        KIT_ASSERT_ERROR(!m_shapes.contains(collider), "Collider already exists in the app");
        m_shapes.emplace(collider, collider_repr2D(collider, collider_color, sleep_greyout));
    };

    world.colliders.events.on_removal += [this](collider2D &collider) {
        KIT_ASSERT_ERROR(m_shapes.contains(&collider), "Collider does not exist in the app");
        m_shapes.erase(&collider);
    };

    world.joints.manager<spring_joint2D>()->events.on_addition += [this](spring_joint2D *sp) {
        m_joints.emplace(sp, kit::make_scope<spring_repr2D>(sp, joint_color, sleep_greyout));
    };

    world.joints.manager<distance_joint2D>()->events.on_addition +=
        [this](distance_joint2D *dj) { m_joints.emplace(dj, kit::make_scope<distance_repr2D>(dj, sleep_greyout)); };
    world.joints.manager<prismatic_joint2D>()->events.on_addition += [this](prismatic_joint2D *pj) {
        m_joints.emplace(pj, kit::make_scope<prismatic_repr2D>(pj, joint_color, sleep_greyout));
    };

    world.joints.events.on_removal += [this](joint2D &joint) { m_joints.erase(&joint); };
}

void app::on_update(const float ts)
{
    {
        KIT_PERF_SCOPE("PPX-APP::Physics")
        const kit::perf::clock physics_clock;

        if (sync_timestep)
            world.integrator.ts.value = sync_speed * ts + (1.f - sync_speed) * world.integrator.ts.value;

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
        case lynx::input2D::key::RIGHT:
            if (paused)
                world.step();
            return true;
        default:
            return false;
        }
    case lynx::event2D::KEY_REPEAT:
        if (ImGui::GetIO().WantCaptureKeyboard)
            break;
        switch (event.key)
        {
        case lynx::input2D::key::RIGHT:
            if (paused)
                world.step();
            return true;
        default:
            return false;
        }
    case lynx::event2D::SCROLLED:
        if (ImGui::GetIO().WantCaptureMouse)
            break;
        zoom(event.scroll_offset.y);
        return true;
    default:
        return false;
    }
    return false;
}

void app::update_shapes()
{
    for (auto &[collider, crepr] : m_shapes)
        crepr.update(sleep_greyout);
}
void app::update_joints()
{
    for (auto &[joint, jrepr] : m_joints)
        jrepr->update(sleep_greyout);
}

void app::draw_shapes() const
{
    for (const auto &[collider, crepr] : m_shapes)
        m_window->draw(crepr);
}

void app::draw_joints() const
{
    for (const auto &[joint, jrepr] : m_joints)
        m_window->draw(*jrepr);
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

void app::zoom(const float offset)
{
    float factor = 4.f * offset * 0.006f; // glm::clamp(offset, -0.05f, 0.05f);
    if (lynx::input2D::key_pressed(lynx::input2D::key::LEFT_CONTROL))
        factor *= 5.f;

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
const std::unordered_map<collider2D *, collider_repr2D> &app::shapes() const
{
    return m_shapes;
}
void app::color(collider2D *collider, const lynx::color &color)
{
    KIT_ASSERT_ERROR(m_shapes.contains(collider), "Collider does not exist in the app");
    m_shapes.at(collider).color = color;
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
