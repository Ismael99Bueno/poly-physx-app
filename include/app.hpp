#ifndef APP_HPP
#define APP_HPP

#include "engine2D.hpp"
#include "layer.hpp"
#include "tableaus.hpp"
#include <string>
#include <stack>
#include <vector>
#include <functional>
#include <SFML/Graphics.hpp>

namespace phys
{
    class app
    {
    public:
        app(const rk::butcher_tableau &table = rk::rk4,
            float dt = 0.001f,
            std::size_t allocations = 100,
            const std::string &name = "Physics engine");
        virtual ~app() = default;

        void run(std::function<bool(engine2D &, float &)> forward = &engine2D::raw_forward);
        void push_layer(layer *l);

        virtual void on_update() {}

        const engine2D &engine() const;
        engine2D &engine();

        const sf::Time &phys_time() const;
        const sf::Time &draw_time() const;

    private:
        const std::string m_name;
        sf::RenderWindow m_window;
        engine2D m_engine;
        std::stack<layer *> m_layers;
        std::vector<sf::ConvexShape> m_shapes;

        alg::vec2 m_mouse_add;
        sf::Time m_phys_time, m_draw_time;

        int m_integrations_per_frame = 10;
        float m_dt;
        bool m_visualize_qt;

        void draw_entities();
        void handle_events();
        alg::vec2 world_mouse() const;
        alg::vec2 world_mouse_delta() const;
    };

}

#endif