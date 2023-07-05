#ifndef PPX_APP_HPP
#define PPX_APP_HPP

#include "rk/tableaus.hpp"
#include "ppx/engine2D.hpp"
#include "lynx/app/app.hpp"
#include "lynx/drawing/shape.hpp"

namespace ppx
{
class app : public lynx::app2D
{
  public:
    app(const rk::butcher_tableau &table = rk::rk4, std::size_t allocations = 100, const char *name = "poly-physx");
    virtual ~app() = default;

    engine2D &engine();
    const engine2D &engine() const;

  private:
    virtual void on_update(float ts) override;
    virtual void on_render(float ts) override;
    virtual bool on_event(const lynx::event &event) override;

    virtual void on_entity_draw(const entity2D_ptr &e, lynx::shape2D &shape)
    {
    }

    engine2D m_engine;
    std::vector<scope<lynx::shape2D>> m_shapes;
    lynx::window2D *m_window;
    lynx::orthographic2D *m_camera;
    bool m_paused = false;

    void update_entities();
    void draw_entities() const;

    void zoom(float offset);
    void move_camera();

    glm::vec2 mouse_position() const;
};
} // namespace ppx

#endif