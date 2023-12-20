#pragma once

#include "ppx/world2D.hpp"

#include "ppx-app/lines/thick_line.hpp"
#include "ppx-app/lines/spring_line.hpp"

#include "lynx/app/app.hpp"
#include "lynx/drawing/shape.hpp"

#include "kit/memory/scope.hpp"
#include "kit/interface/serialization.hpp"

#ifdef KIT_USE_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif

namespace ppx
{
class distance_joint2D;
class app : public lynx::app2D, public kit::serializable
{
  public:
    inline static const lynx::color DEFAULT_BODY_COLOR{123u, 143u, 161u};
    inline static const lynx::color DEFAULT_JOINT_COLOR{207u, 185u, 151u};
    inline static const lynx::color DEFAULT_BODY_OUTLINE_COLOR{225u, 152u, 152u};

    app(const rk::butcher_tableau &table = rk::butcher_tableau::rk4, const char *name = "poly-physx");
    virtual ~app() = default;

    world2D world;
    float timestep = 1e-3f;
    bool sync_timestep = true;
    bool paused = false;

    kit::time physics_time() const;

    glm::vec2 world_mouse_position() const;
    const std::vector<kit::scope<lynx::shape2D>> &shapes() const;
    const std::vector<spring_line> &spring_lines() const;
    const std::unordered_map<const distance_joint2D *, thick_line> &dist_joint_lines() const;

#ifdef KIT_USE_YAML_CPP
    virtual YAML::Node encode() const override;
    virtual bool decode(const YAML::Node &node) override;
#endif

    virtual void on_update(float ts) override;
    virtual void on_render(float ts) override;
    virtual bool on_event(const lynx::event2D &event) override;

    lynx::color body_color = DEFAULT_BODY_COLOR;
    lynx::color joint_color = DEFAULT_JOINT_COLOR;
    lynx::color body_outline_color = DEFAULT_BODY_OUTLINE_COLOR;

    std::uint32_t integrations_per_frame = 1;

  private:
    virtual void on_body_update(const body2D &body, lynx::shape2D &shape)
    {
    }

    lynx::window2D *m_window;
    lynx::orthographic2D *m_camera;

    std::vector<kit::scope<lynx::shape2D>> m_shapes;
    std::vector<spring_line> m_spring_lines;
    std::unordered_map<const distance_joint2D *, thick_line> m_dist_joint_lines;

    kit::time m_physics_time;

    void update_entities();
    void update_joints();

    void draw_entities() const;
    void draw_joints() const;

    void zoom(float offset, float ts);
    void move_camera(float ts);

    void add_world_callbacks();
};

} // namespace ppx
