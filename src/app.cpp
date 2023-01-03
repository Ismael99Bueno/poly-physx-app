#include "app.hpp"
#include "constants.hpp"

namespace phys
{
    app::app(const rk::butcher_tableau &table,
             const std::size_t allocations,
             const std::string &name) : m_engine(table, allocations),
                                        m_window(sf::VideoMode::getFullscreenModes()[0],
                                                 name,
                                                 sf::Style::Fullscreen)
    {
        m_window.setView(sf::View(sf::Vector2f(0.f, 0.f), sf::Vector2f(WIDTH, -HEIGHT)));

        const auto add_shape = [this](entity_ptr e)
        {
            sf::ConvexShape &shape = m_shapes.emplace_back(sf::ConvexShape());
            const geo::polygon2D &poly = e->shape();

            shape.setPointCount(poly.size());
            for (std::size_t i = 0; i < poly.size(); i++)
                shape.setPoint(i, poly[i] * WORLD_TO_PIXEL);
            shape.setFillColor(m_entity_color);
            shape.setOutlineColor(sf::Color::Red);
        };
        const auto remove_shape = [this](const std::size_t index)
        {
            m_shapes[index] = m_shapes.back();
            m_shapes.pop_back();
        };

        m_engine.on_entity_addition(add_shape);
        m_engine.on_entity_removal(remove_shape);
        align_dt();
    }

    void app::run(std::function<bool(engine2D &, float &)> forward)
    {
        if (!ImGui::SFML::Init(m_window, false))
        {
            perror("ImGui initialization failed\n");
            exit(EXIT_FAILURE);
        }
        add_font("fonts/FiraCode/FiraCode-Light.ttf", FONT_SIZE_PIXELS);
        if (!ImGui::SFML::UpdateFontTexture())
        {
            perror("ImGui font initialization failed\n");
            exit(EXIT_FAILURE);
        }

        m_window.setFramerateLimit(DEFAULT_FPS);
        sf::Clock dclock;
        while (m_window.isOpen())
        {
            PERF_SCOPE("-Frame-")
            handle_events();
            sf::Clock phys_clock;
            if (!m_paused)
                for (std::size_t i = 0; i < m_integrations_per_frame; i++)
                    forward(m_engine, m_dt);
            m_phys_time = phys_clock.getElapsedTime();

            {
                PERF_SCOPE("-Drawing-")
                sf::Clock draw_clock;
                ImGui::SFML::Update(m_window, dclock.restart());

                m_window.clear();
                draw_entities();
                update_layers();
                on_update();
                ImGui::SFML::Render(m_window);
                m_window.display();
                m_draw_time = draw_clock.getElapsedTime();
            }
            if (m_aligned_dt)
                align_dt();
        }
    }

    void app::push_layer(layer *l)
    {
        m_layers.emplace_back(l);
        l->on_attach(this);
    }

    void app::update_layers()
    {
        for (layer *l : m_layers)
            l->on_update();
    }

    void app::event_layers(sf::Event &event)
    {
        for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it)
            (*it)->on_event(event);
    }

    void app::draw_entities()
    {
        PERF_FUNCTION()
        m_engine.retrieve();
        const std::vector<phys::entity2D> &entities = m_engine.entities();
        for (std::size_t i = 0; i < m_shapes.size(); i++)
        {
            sf::ConvexShape &shape = m_shapes[i];
            const entity2D &e = entities[i];
            if (shape.getPointCount() != e.shape().size())
                shape.setPointCount(e.shape().size());
            for (std::size_t j = 0; j < shape.getPointCount(); j++)
                shape.setPoint(j, e.shape()[j] * WORLD_TO_PIXEL);
            on_entity_draw({&entities, i}, shape);
            m_window.draw(shape);
        }
    }

    void app::handle_events()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            switch (event.type)
            {
            case sf::Event::Closed:
                m_window.close();
                break;

            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                case sf::Keyboard::Escape:
                    m_window.close();
                    break;
                case sf::Keyboard::Space:
                    m_paused = !m_paused;
                    break;
                default:
                    break;
                }
            default:
                break;
            }
            event_layers(event);
            on_event(event);
        }
    }

    void app::align_dt()
    {
        const rk::integrator &integ = m_engine.integrator();
        m_dt = std::clamp((m_phys_time + m_draw_time).asSeconds(), integ.min_dt(), integ.max_dt());
    }

    alg::vec2 app::pixel_mouse() const
    {
        const sf::Vector2i mpos = sf::Mouse::getPosition(m_window);
        const sf::Vector2f wpos = m_window.mapPixelToCoords(mpos);
        return alg::vec2(wpos.x, wpos.y);
    }

    alg::vec2 app::pixel_mouse_delta() const
    {
        return alg::vec2(ImGui::GetIO().MouseDelta.x, -ImGui::GetIO().MouseDelta.y);
    }

    alg::vec2 app::world_mouse() const { return pixel_mouse() * PIXEL_TO_WORLD; }
    alg::vec2 app::world_mouse_delta() const { return pixel_mouse_delta() * PIXEL_TO_WORLD; }

    const engine2D &app::engine() const { return m_engine; }
    engine2D &app::engine() { return m_engine; }

    const std::vector<sf::ConvexShape> &app::shapes() const { return m_shapes; }
    std::vector<sf::ConvexShape> &app::shapes() { return m_shapes; }

    const sf::Color &app::entity_color() const { return m_entity_color; }
    sf::Color &app::entity_color() { return m_entity_color; }

    void app::add_font(const char *path, const float size_pixels) const
    {
        ImGuiIO &io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF(path, size_pixels);
        io.Fonts->Build();
    }

    int app::integrations_per_frame() const { return m_integrations_per_frame; }
    void app::integrations_per_frame(int integrations_per_frame) { m_integrations_per_frame = integrations_per_frame; }

    float app::timestep() const { return m_dt; }
    void app::timestep(float ts) { m_dt = ts; }

    bool app::paused() const { return m_paused; }
    void app::paused(const bool paused) { m_paused = paused; }

    bool app::aligned_timestep() const { return m_aligned_dt; }
    void app::aligned_timestep(const bool aligned_dt) { m_aligned_dt = aligned_dt; }

    const sf::RenderWindow &app::window() const { return m_window; }
    sf::RenderWindow &app::window() { return m_window; }

    const sf::Time &app::phys_time() const { return m_phys_time; }
    const sf::Time &app::draw_time() const { return m_draw_time; }

    void app::entity_color(const sf::Color &color) { m_entity_color = color; }
}