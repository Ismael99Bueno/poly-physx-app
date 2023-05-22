#ifndef APP_HPP
#define APP_HPP

#include "ppx/engine2D.hpp"
#include "ppx-app/layer.hpp"
#include "ppx-app/menu_layer.hpp"
#include "rk/tableaus.hpp"
#include <string>
#include <vector>
#include <functional>
#include <SFML/Graphics.hpp>

#define PPX_WIDTH 1920.f
#define PPX_HEIGHT 1280.f
#define PPX_WORLD_TO_PIXEL 10.f
#define PPX_PIXEL_TO_WORLD 0.1f
#define PPX_NO_FPS_LIMIT 0
#define PPX_DEFAULT_FPS 120
#define PPX_DEFAULT_ENTITY_COLOR sf::Color(123, 143, 161)
#define PPX_DEFAULT_JOINT_COLOR sf::Color(207, 185, 151)

namespace ppx
{
    class app
    {
    public:
        app(const rk::butcher_tableau &table = rk::rk4,
            std::size_t allocations = 100,
            const char *name = "poly-physx");
        virtual ~app() = default;

        void run(std::function<bool(engine2D &, float &)> forward = &engine2D::raw_forward);

        template <typename T, class... Args>
        ref<T> push_layer(Args &&...args)
        {
            static_assert(std::is_base_of<layer, T>::value, "Layer must inherit from layer!");
            const auto layer = make_ref<T>(std::forward<Args>(args)...);
            m_layers.emplace_back(layer)->on_attach(this);
            return layer;
        }
        void pop_layer(const ref<layer> &l);

        template <class... Args>
        void draw(Args &&...args) { m_window.draw(std::forward<Args>(args)...); }

        void draw_spring(const glm::vec2 &p1, const glm::vec2 &p2, const sf::Color &color);
        void draw_rigid_bar(const glm::vec2 &p1, const glm::vec2 &p2, const sf::Color &color);

        void draw_spring(const glm::vec2 &p1, const glm::vec2 &p2);
        void draw_rigid_bar(const glm::vec2 &p1, const glm::vec2 &p2);

        void update_shapes(); // consider implementing update a single shape

        sf::ConvexShape convex_shape_from(const geo::polygon &poly) const;
        sf::CircleShape circle_shape_from(const geo::circle &c) const;

        void transform_camera(const glm::vec2 &dir);
        void transform_camera(const glm::vec2 &dir, const glm::vec2 &size);

        void recreate_window(sf::Uint32 style,
                             const glm::vec2 &center,
                             const glm::vec2 &size,
                             const char *name = "poly-physx");
        void recreate_window(sf::Uint32 style, const char *name = "poly-physx");

        const engine2D &engine() const;
        engine2D &engine();

        const std::vector<scope<sf::Shape>> &shapes() const;
        cvw::vector<scope<sf::Shape>> shapes();
        sf::Shape &operator[](std::size_t index) const;

        const sf::Color &entity_color() const;
        const sf::Color &springs_color() const;
        const sf::Color &rigid_bars_color() const;

        void entity_color(const sf::Color &color);
        void springs_color(const sf::Color &color);
        void rigid_bars_color(const sf::Color &color);

        std::uint32_t integrations_per_frame() const;
        void integrations_per_frame(std::uint32_t integrations_per_frame);

        float timestep() const;
        void timestep(float ts);

        bool paused() const;
        void paused(bool paused);

        std::uint32_t framerate() const;
        void framerate(std::uint32_t framerate);

        bool sync_timestep() const;
        void sync_timestep(bool aligned_dt);

        const sf::RenderWindow &window() const;
        sf::RenderWindow &window();

        const sf::Time &phys_time() const;
        const sf::Time &draw_time() const;
        sf::Time delta_time() const;

        const sf::Time &raw_phys_time() const;
        const sf::Time &raw_draw_time() const;
        sf::Time raw_delta_time() const;

        float time_measure_smoothness() const;
        void time_measure_smoothness(float smoothness);

        glm::vec2 pixel_mouse() const;
        glm::vec2 pixel_mouse_delta() const;

        glm::vec2 world_mouse() const;
        glm::vec2 world_mouse_delta() const;

        sf::Uint32 style() const;
        void style(sf::Uint32 style);

    private:
        sf::RenderWindow m_window;
        engine2D m_engine;

        std::vector<ref<layer>> m_layers;
        std::vector<scope<sf::Shape>> m_shapes;

        bool m_paused = false, m_sync_dt = true;
        sf::Uint32 m_style = sf::Style::Default;

        sf::Time m_phys_time, m_draw_time,
            m_raw_phys_time, m_raw_draw_time;

        float m_time_smoothness = 0.f;
        sf::Color m_entity_color = PPX_DEFAULT_ENTITY_COLOR,
                  m_springs_color = PPX_DEFAULT_JOINT_COLOR,
                  m_rigid_bars_color = PPX_DEFAULT_JOINT_COLOR;

        std::uint32_t m_integrations_per_frame = 1, m_framerate;
        float m_dt = 1e-3f;

        virtual void on_start() {}
        virtual void on_update() {}
        virtual void on_late_update() {}
        virtual void on_render() {}
        virtual void on_entity_draw(const entity2D_ptr &e, sf::Shape &shape) {}
        virtual void on_event(sf::Event &event) {}
        virtual void on_end() {}

        void layer_start();
        void layer_update();
        void layer_late_update();
        void layer_render();
        void layer_event(sf::Event &event);
        void layer_end();

        void draw_entities();
        void draw_springs();
        void draw_rigid_bars();
        void handle_events();
        void sync_dt();

        void add_fonts() const;
        void control_camera();
        void zoom(float delta);
#ifdef HAS_YAML_CPP
        friend YAML::Emitter &operator<<(YAML::Emitter &, const app &);
        friend struct YAML::convert<app>;
#endif
    };

#ifdef HAS_YAML_CPP
    YAML::Emitter &operator<<(YAML::Emitter &out, const app &papp);
#endif
}

#ifdef HAS_YAML_CPP
namespace sf
{
    YAML::Emitter &operator<<(YAML::Emitter &out, const Color &color);
}
#endif

#ifdef HAS_YAML_CPP
namespace YAML
{
    template <>
    struct convert<ppx::app>
    {
        static Node encode(const ppx::app &papp);
        static bool decode(const Node &node, ppx::app &papp);
    };

    template <>
    struct convert<sf::Color>
    {
        static Node encode(const sf::Color &color);
        static bool decode(const Node &node, sf::Color &color);
    };
}
#endif

#endif