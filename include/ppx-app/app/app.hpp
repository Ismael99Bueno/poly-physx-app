#pragma once

#include "ppx/world2D.hpp"

#include "ppx-app/drawables/joints/joint_repr2D.hpp"
#include "ppx-app/app/menu_layer.hpp"

#include "lynx/app/app.hpp"
#include "lynx/drawing/shape.hpp"

#include "kit/memory/ptr/scope.hpp"
#include "kit/serialization/yaml/serializer.hpp"

namespace ppx
{
class app : public lynx::app2D
{
  public:
    inline static const lynx::color DEFAULT_COLLIDER_COLOR{123u, 143u, 161u};
    inline static const lynx::color DEFAULT_JOINT_COLOR{207u, 185u, 151u};

    template <class... WorldArgs>
    app(const char *name, WorldArgs &&...args) : lynx::app2D(800, 600, name), world(std::forward<WorldArgs>(args)...)
    {
        world.add_builtin_joint_managers();
        m_window = window();
        push_layer<menu_layer>();

        m_window->maintain_camera_aspect_ratio(true);
        m_camera = m_window->set_camera<lynx::orthographic2D>(m_window->pixel_aspect(), 50.f);
        m_camera->flip_y_axis();

        add_world_callbacks();
    }

    virtual ~app() = default;

    world2D world;
    bool sync_timestep = true;
    bool paused = false;
    float sync_speed = 0.01f;

    lynx::color collider_color = DEFAULT_COLLIDER_COLOR;
    lynx::color joint_color = DEFAULT_JOINT_COLOR;

    std::uint32_t integrations_per_frame = 1;

    kit::perf::time physics_time() const;

    glm::vec2 world_mouse_position() const;
    const std::unordered_map<collider2D *, kit::scope<lynx::shape2D>> &shapes() const;

    virtual void on_update(float ts) override;
    virtual void on_render(float ts) override;
    virtual bool on_event(const lynx::event2D &event) override;

    std::pair<lynx::color, lynx::color> &color(collider2D *collider);

#ifdef KIT_USE_YAML_CPP
    virtual YAML::Node encode() const override;
    virtual bool decode(const YAML::Node &node) override;
#endif

  private:
    lynx::window2D *m_window;
    lynx::orthographic2D *m_camera;

    std::unordered_map<collider2D *, kit::scope<lynx::shape2D>> m_shapes;
    std::unordered_map<collider2D *, std::pair<lynx::color, lynx::color>> m_shape_colors;
    std::unordered_map<joint2D *, kit::scope<joint_repr2D>> m_joints;

    kit::perf::time m_physics_time;

    void update_shapes();
    void update_joints();

    void draw_shapes() const;
    void draw_joints() const;

    void zoom(float offset);
    void move_camera(float ts);

    void add_world_callbacks();
};

} // namespace ppx
