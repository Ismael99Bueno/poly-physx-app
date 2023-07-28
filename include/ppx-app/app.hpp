#ifndef PPX_APP_HPP
#define PPX_APP_HPP

#include "rk/tableaus.hpp"
#include "ppx/world2D.hpp"

#include "ppx-app/thick_line.hpp"
#include "ppx-app/spring_line.hpp"

#include "lynx/app/app.hpp"
#include "lynx/drawing/shape.hpp"

#include "kit/memory/scope.hpp"
#include "kit/interface/serialization.hpp"

#ifdef KIT_USE_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif

#define PPX_DEFAULT_ENTITY_COLOR glm::vec4(123.f, 143.f, 161.f, 255.f) / 255.f
#define PPX_DEFAULT_JOINT_COLOR glm::vec4(207.f, 185.f, 151.f, 255.f) / 255.f

namespace ppx
{
class revolute_joint2D;
class app : public lynx::app2D, public kit::serializable
{
  public:
    app(const rk::butcher_tableau &table = rk::rk4, std::size_t allocations = 100, const char *name = "poly-physx");
    virtual ~app() = default;

    world2D world;
    float timestep = 1e-3f;
    bool sync_timestep = true;
    bool paused = false;

    kit::time update_time() const;
    kit::time physics_time() const;
    kit::time draw_time() const;

    glm::vec2 world_mouse_position() const;

#ifdef KIT_USE_YAML_CPP
    virtual YAML::Node encode() const override;
    virtual bool decode(const YAML::Node &node) override;
#endif

    virtual void on_update(float ts) override;
    virtual void on_render(float ts) override;
    virtual bool on_event(const lynx::event &event) override;

    glm::vec4 body_color = PPX_DEFAULT_ENTITY_COLOR;
    glm::vec4 joint_color = PPX_DEFAULT_JOINT_COLOR;

    std::uint32_t integrations_per_frame = 1;

  protected:
    lynx::orthographic2D *m_camera;

  private:
    virtual void on_body_update(const body2D &body, lynx::shape2D &shape)
    {
    }

    std::vector<kit::scope<lynx::shape2D>> m_shapes;
    std::vector<spring_line> m_spring_lines;
    std::unordered_map<const revolute_joint2D *, thick_line> m_thick_lines;

    lynx::window2D *m_window;

    kit::time m_update_time;
    kit::time m_physics_time;
    kit::time m_draw_time;

    void update_entities();
    void update_joints();

    void draw_entities() const;
    void draw_joints() const;

    void zoom(float offset, float ts);
    void move_camera(float ts);

    void add_world_callbacks();
};

} // namespace ppx

#endif