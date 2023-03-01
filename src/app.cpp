#include "app.hpp"
#include "constants.hpp"
#include "spring_line.hpp"
#include "thick_line.hpp"
#include "rigid_bar2D.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace phys
{
    app::app(const rk::butcher_tableau &table,
             const std::size_t allocations,
             const std::string &name) : m_window(sf::VideoMode::getFullscreenModes()[0],
                                                 name,
                                                 sf::Style::Fullscreen),
                                        m_engine(table, allocations)
    {
        m_window.setView(sf::View(sf::Vector2f(0.f, 0.f), sf::Vector2f(WIDTH, -HEIGHT)));

        const auto add_shape = [this](entity2D_ptr e)
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

        if (!ImGui::SFML::Init(m_window, false))
        {
            perror("ImGui initialization failed\n");
            exit(EXIT_FAILURE);
        }
        add_font("fonts/FiraCode/FiraCode-Light.ttf", FONT_SIZE_PIXELS);
        framerate(DEFAULT_FPS);
    }

    void app::run(std::function<bool(engine2D &, float &)> forward)
    {
        sf::Clock dclock;
        on_start();
        layer_start();

        while (m_window.isOpen())
        {
            PERF_SCOPE("-Frame-")
            handle_events();
            sf::Clock phys_clock;
            if (!m_paused)
                for (std::size_t i = 0; i < m_integrations_per_frame; i++)
                    forward(m_engine, m_dt);

            m_raw_phys_time = phys_clock.getElapsedTime();
            m_phys_time = (1.f - m_time_smoothness) * m_raw_phys_time +
                          m_time_smoothness * m_phys_time;

            {
                PERF_SCOPE("-Drawing-")
                sf::Clock draw_clock;
                ImGui::SFML::Update(m_window, dclock.restart());

                m_window.clear();
                on_update();

                draw_entities();
                draw_springs();
                draw_rigid_bars();

                on_render();
                layer_render();

                on_late_update();
                control_camera();
                ImGui::SFML::Render(m_window);
                m_window.display();

                m_raw_draw_time = draw_clock.getElapsedTime();
                m_draw_time = (1.f - m_time_smoothness) * m_raw_draw_time +
                              m_time_smoothness * m_draw_time;
            }
            if (m_aligned_dt)
                align_dt();
        }
        on_end();
        layer_end();
    }

    void app::push_layer(layer *l)
    {
        m_layers.emplace_back(l);
        l->on_attach(this);
    }

    void app::pop_layer(const layer *l)
    {
        const auto it = m_layers.erase(std::remove(m_layers.begin(), m_layers.end(), l), m_layers.end());
        if (it != m_layers.end())
            (*it)->on_detach();
    }

    void app::draw_entity(const std::vector<alg::vec2> vertices,
                          sf::ConvexShape &shape, const sf::Color &color)
    {
        shape.setFillColor(color);
        draw_entity(vertices, shape);
    }

    void app::draw_entity(const std::vector<alg::vec2> vertices,
                          sf::ConvexShape &shape)
    {
        if (shape.getPointCount() != vertices.size())
            shape.setPointCount(vertices.size());
        for (std::size_t j = 0; j < shape.getPointCount(); j++)
            shape.setPoint(j, vertices[j] * WORLD_TO_PIXEL);
        m_window.draw(shape);
    }

    void app::draw_spring(const alg::vec2 &p1, const alg::vec2 &p2, const sf::Color &color)
    {
        prm::spring_line sp_line(p1, p2, color);
        m_window.draw(sp_line);
    }
    void app::draw_rigid_bar(const alg::vec2 &p1, const alg::vec2 &p2, const sf::Color &color)
    {
        prm::thick_line tl(p1, p2, 8.f, color);
        m_window.draw(tl);
    }

    void app::write(ini::output &out) const
    {
        out.begin_section("engine");
        m_engine.write(out);
        out.end_section();

        std::size_t index = 0;
        const std::string section = "entity";
        for (const sf::ConvexShape &shape : m_shapes)
        {
            out.begin_section(section + std::to_string(index++));
            out.write("r", (int)shape.getFillColor().r);
            out.write("g", (int)shape.getFillColor().g);
            out.write("b", (int)shape.getFillColor().b);
            out.end_section();
        }

        out.write("framerate", m_framerate);
        out.write("time_smoothness", m_time_smoothness);
        out.write("integ_per_frame", m_integrations_per_frame);
        out.write("aligned_dt", m_aligned_dt);
        out.write("timestep", m_dt);
        out.begin_section("springs_color");
        out.write("r", (int)m_springs_color.r);
        out.write("g", (int)m_springs_color.g);
        out.write("b", (int)m_springs_color.b);
        out.end_section();
        out.begin_section("rigid_bars_color");
        out.write("r", (int)m_rigid_bars_color.r);
        out.write("g", (int)m_rigid_bars_color.g);
        out.write("b", (int)m_rigid_bars_color.b);
        out.end_section();
        out.write("paused", m_paused);

        const sf::View &view = m_window.getView();
        out.write("camx", view.getCenter().x);
        out.write("camy", view.getCenter().y);

        out.write("width", view.getSize().x);
        out.write("height", view.getSize().y);
    }

    void app::read(ini::input &in)
    {
        in.begin_section("engine");
        m_engine.read(in);
        in.end_section();

        std::size_t index = 0;
        const std::string section = "entity";
        for (sf::ConvexShape &shape : m_shapes)
        {
            in.begin_section(section + std::to_string(index++));
            shape.setFillColor({(sf::Uint8)in.readi("r"), (sf::Uint8)in.readi("g"), (sf::Uint8)in.readi("b")});
            in.end_section();
        }

        framerate(in.readi("framerate"));
        m_integrations_per_frame = in.readi("integ_per_frame");
        m_aligned_dt = (bool)in.readi("aligned_dt");
        m_time_smoothness = in.readf("time_smoothness");
        m_dt = in.readf("timestep");
        in.begin_section("springs_color");
        m_springs_color = {(sf::Uint8)in.readi("r"), (sf::Uint8)in.readi("g"), (sf::Uint8)in.readi("b")};
        in.end_section();
        in.begin_section("rigid_bars_color");
        m_rigid_bars_color = {(sf::Uint8)in.readi("r"), (sf::Uint8)in.readi("g"), (sf::Uint8)in.readi("b")};
        in.end_section();
        m_paused = (bool)in.readi("paused");

        sf::View view = m_window.getView();
        const float camx = in.readf("camx"), camy = in.readf("camy"),
                    width = in.readf("width"), height = in.readf("height");

        view.setCenter(camx, camy);
        view.setSize(width, height);
        m_window.setView(view);
    }

    void app::draw_spring(const alg::vec2 &p1, const alg::vec2 &p2) { draw_spring(p1, p2, m_springs_color); }
    void app::draw_rigid_bar(const alg::vec2 &p1, const alg::vec2 &p2) { draw_rigid_bar(p1, p2, m_rigid_bars_color); }

    void app::layer_start()
    {
        for (layer *l : m_layers)
            l->on_start();
    }

    void app::layer_render()
    {
        PERF_FUNCTION()
        for (layer *l : m_layers)
            l->on_render();
    }

    void app::layer_event(sf::Event &event)
    {
        PERF_FUNCTION()
        for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it)
            (*it)->on_event(event);
    }

    void app::layer_end()
    {
        for (layer *l : m_layers)
            l->on_end();
    }

    void app::draw_entities()
    {
        PERF_FUNCTION()
        for (std::size_t i = 0; i < m_shapes.size(); i++)
        {
            sf::ConvexShape &shape = m_shapes[i];
            const phys::entity2D_ptr e = m_engine[i];

            on_entity_draw(e, shape);
            draw_entity(e->shape().vertices(), shape);
        }
    }

    void app::draw_springs()
    {
        PERF_FUNCTION()
        for (const phys::spring2D &sp : m_engine.springs())
        {
            const alg::vec2 p1 = (sp.e1()->pos() + sp.joint1()) * WORLD_TO_PIXEL,
                            p2 = (sp.e2()->pos() + sp.joint2()) * WORLD_TO_PIXEL;
            draw_spring(p1, p2);
        }
    }

    void app::draw_rigid_bars()
    {
        PERF_FUNCTION()
        for (const auto &ctr : m_engine.compeller().constraints())
        {
            const auto &rb = dynamic_cast<phys::rigid_bar2D *>(ctr.get());
            if (!rb)
                continue;
            const alg::vec2 p1 = (rb->e1()->pos() + rb->joint1()) * WORLD_TO_PIXEL,
                            p2 = (rb->e2()->pos() + rb->joint2()) * WORLD_TO_PIXEL;
            draw_rigid_bar(p1, p2);
        }
    }

    void app::handle_events()
    {
        PERF_FUNCTION()
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
            case sf::Event::MouseWheelScrolled:
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
                    zoom(event.mouseWheelScroll.delta);
            default:
                break;
            }
            on_event(event);
            layer_event(event);
        }
    }

    void app::align_dt()
    {
        const rk::integrator &integ = m_engine.integrator();
        m_dt = std::clamp(raw_delta_time().asSeconds(), integ.min_dt(), integ.max_dt());
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

    void app::transform_camera(const alg::vec2 &dir)
    {
        sf::View v = m_window.getView();
        v.move(dir);
        m_window.setView(v);
    }

    void app::transform_camera(const alg::vec2 &dir, const alg::vec2 &size)
    {
        sf::View v = m_window.getView();
        v.setSize(size);
        v.move(dir);
        m_window.setView(v);
    }

    void app::control_camera()
    {
        if (ImGui::GetIO().WantCaptureKeyboard)
            return;
        const alg::vec2 size = m_window.getView().getSize();
        const float speed = 0.75f * raw_delta_time().asSeconds() * size.norm();
        alg::vec2 vel;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            vel.x += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            vel.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            vel.y += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            vel.y -= speed;
        if (vel.sq_norm() > 0.f)
            transform_camera(vel);
    }

    void app::zoom(const float delta)
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return;
        const float factor = delta * 0.006f;

        const sf::View &v = m_window.getView();
        transform_camera((pixel_mouse() - v.getCenter()) * factor, v.getSize() * (1.f - factor));
    }

    alg::vec2 app::world_mouse() const { return pixel_mouse() * PIXEL_TO_WORLD; }
    alg::vec2 app::world_mouse_delta() const { return pixel_mouse_delta() * PIXEL_TO_WORLD; }

    const engine2D &app::engine() const { return m_engine; }
    engine2D &app::engine() { return m_engine; }

    const std::vector<sf::ConvexShape> &app::shapes() const { return m_shapes; }
    utils::vector_view<sf::ConvexShape> app::shapes() { return m_shapes; }

    const sf::Color &app::entity_color() const { return m_entity_color; }
    const sf::Color &app::springs_color() const { return m_springs_color; }
    const sf::Color &app::rigid_bars_color() const { return m_rigid_bars_color; }

    void app::entity_color(const sf::Color &color) { m_entity_color = color; }
    void app::springs_color(const sf::Color &color) { m_springs_color = color; }
    void app::rigid_bars_color(const sf::Color &color) { m_rigid_bars_color = color; }

    void app::add_font(const char *path, const float size_pixels) const
    {
        ImGuiIO &io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF(path, size_pixels);
        io.Fonts->Build();
        if (!ImGui::SFML::UpdateFontTexture())
        {
            perror("ImGui font initialization failed\n");
            exit(EXIT_FAILURE);
        }
    }

    int app::integrations_per_frame() const { return m_integrations_per_frame; }
    void app::integrations_per_frame(int integrations_per_frame) { m_integrations_per_frame = integrations_per_frame; }

    float app::timestep() const { return m_dt; }
    void app::timestep(float ts) { m_dt = ts; }

    bool app::paused() const { return m_paused; }
    void app::paused(const bool paused) { m_paused = paused; }

    int app::framerate() const { return m_framerate; }
    void app::framerate(const int framerate)
    {
        m_framerate = framerate;
        m_window.setFramerateLimit(framerate);
    }

    bool app::aligned_timestep() const { return m_aligned_dt; }
    void app::aligned_timestep(const bool aligned_dt) { m_aligned_dt = aligned_dt; }

    const sf::RenderWindow &app::window() const { return m_window; }
    sf::RenderWindow &app::window() { return m_window; }

    const sf::Time &app::phys_time() const { return m_phys_time; }
    const sf::Time &app::draw_time() const { return m_draw_time; }
    sf::Time app::delta_time() const { return m_phys_time + m_draw_time; }

    const sf::Time &app::raw_phys_time() const { return m_raw_phys_time; }
    const sf::Time &app::raw_draw_time() const { return m_raw_draw_time; }
    sf::Time app::raw_delta_time() const { return m_raw_phys_time + m_raw_draw_time; }

    float app::time_measure_smoothness() const { return m_time_smoothness; }
    void app::time_measure_smoothness(const float smoothness) { m_time_smoothness = smoothness; }
}