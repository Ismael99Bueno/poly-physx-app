#ifndef APP_HPP
#define APP_HPP

#include "engine2D.hpp"
#include "layer.hpp"
#include "tableaus.hpp"
#include <string>
#include <vector>
#include <functional>
#include <SFML/Graphics.hpp>

namespace phys
{
    class app
    {
    public:
        app(const rk::butcher_tableau &table = rk::rk4,
            std::size_t allocations = 100,
            const std::string &name = "Physics engine");
        virtual ~app() = default;

        void run(std::function<bool(engine2D &, float &)> forward = &engine2D::raw_forward);
        void push_layer(layer *l);

        void draw_entity(const std::vector<alg::vec2> vertices,
                         sf::ConvexShape &shape,
                         const sf::Color &color);
        void draw_entity(const std::vector<alg::vec2> vertices,
                         sf::ConvexShape &shape);

        void draw_spring(const alg::vec2 &p1, const alg::vec2 &p2, const sf::Color &color);
        void draw_rigid_bar(const alg::vec2 &p1, const alg::vec2 &p2, const sf::Color &color);

        void draw_spring(const alg::vec2 &p1, const alg::vec2 &p2);
        void draw_rigid_bar(const alg::vec2 &p1, const alg::vec2 &p2);

        const engine2D &engine() const;
        engine2D &engine();

        const std::vector<sf::ConvexShape> &shapes() const;
        std::vector<sf::ConvexShape> &shapes();

        const sf::Color &entity_color() const;
        const sf::Color &springs_color() const;
        const sf::Color &rigid_bars_color() const;

        void entity_color(const sf::Color &color);
        void springs_color(const sf::Color &color);
        void rigid_bars_color(const sf::Color &color);

        int integrations_per_frame() const;
        void integrations_per_frame(int integrations_per_frame);

        float timestep() const;
        void timestep(float ts);

        bool paused() const;
        void paused(bool paused);

        bool aligned_timestep() const;
        void aligned_timestep(bool aligned_dt);

        const sf::RenderWindow &window() const;
        sf::RenderWindow &window();

        const sf::Time &phys_time() const;
        const sf::Time &draw_time() const;

        void add_font(const char *path, float size_pixels) const;

        alg::vec2 pixel_mouse() const;
        alg::vec2 pixel_mouse_delta() const;

        alg::vec2 world_mouse() const;
        alg::vec2 world_mouse_delta() const;

    private:
        const std::string m_name;
        sf::RenderWindow m_window;
        engine2D m_engine;
        std::vector<layer *> m_layers;
        std::vector<sf::ConvexShape> m_shapes;
        bool m_paused = false, m_aligned_dt = true;

        sf::Time m_phys_time, m_draw_time;
        sf::Color m_entity_color = sf::Color::Green,
                  m_springs_color = sf::Color::Magenta,
                  m_rigid_bars_color = sf::Color::Magenta;

        virtual void on_update() {}
        virtual void on_late_update() {}
        virtual void on_render() {}
        virtual void on_entity_draw(const entity_ptr &e, sf::ConvexShape &shape) {}
        virtual void on_event(sf::Event &event) {}

        int m_integrations_per_frame = 1;
        float m_dt;
        bool m_visualize_qt;

        void layer_render();
        void layer_event(sf::Event &event);

        void draw_entities();
        void draw_springs();
        void draw_rigid_bars();
        void handle_events();
        void align_dt();
    };

}

#endif