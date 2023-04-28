#include "ppx/app.hpp"
#include "prm/spring_line.hpp"
#include "prm/thick_line.hpp"
#include "ppx/rigid_bar2D.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include <filesystem>
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>

#define FONTS_DIR "fonts/"
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define TO_DEGREES (180.0f / (float)M_PI)

namespace ppx
{
    app::app(const rk::butcher_tableau &table,
             const std::size_t allocations,
             const char *name) : m_engine(table, allocations)

    {
        recreate_window(sf::Style::Default, {0.f, 0.f}, {WIDTH, -HEIGHT}, name);
        push_layer<menu_layer>();

        m_window.setVerticalSyncEnabled(false);
        m_engine.integrator().min_dt(1.e-5f);
        m_engine.integrator().max_dt(0.008f);

        const auto add_shape = [this](const entity2D_ptr &e)
        {
            if (const auto *c = e->shape_if<geo::circle>())
            {
                const sf::CircleShape shape = circle_shape_from(*c);
                m_shapes.emplace_back(std::make_unique<sf::CircleShape>(shape))->setFillColor(m_entity_color);
                return;
            }
            const sf::ConvexShape shape = convex_shape_from(e->shape<geo::polygon>());
            m_shapes.emplace_back(std::make_unique<sf::ConvexShape>(shape))->setFillColor(m_entity_color);
        };

        const auto remove_shape = [this](const std::size_t index)
        {
            m_shapes[index] = std::move(m_shapes.back());
            m_shapes.pop_back();
        };

        m_engine.events().on_entity_addition += add_shape;
        m_engine.events().on_late_entity_removal += remove_shape;

        if (!ImGui::SFML::Init(m_window))
        {
            perror("ImGui initialization failed\n");
            exit(EXIT_FAILURE);
        }
        add_fonts();
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
                for (std::uint32_t i = 0; i < m_integrations_per_frame; i++)
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

    void app::pop_layer(const std::shared_ptr<layer> &l)
    {
        const auto it = m_layers.erase(std::remove(m_layers.begin(), m_layers.end(), l), m_layers.end());
        if (it != m_layers.end())
            (*it)->on_detach();
    }

    void app::draw_spring(const glm::vec2 &p1, const glm::vec2 &p2, const sf::Color &color)
    {
        prm::spring_line sp_line(p1, p2, color);
        m_window.draw(sp_line);
    }
    void app::draw_rigid_bar(const glm::vec2 &p1, const glm::vec2 &p2, const sf::Color &color)
    {
        prm::thick_line tl(p1, p2, 8.f, color);
        m_window.draw(tl);
    }

    void app::write(ini::output &out) const
    {
        out.begin_section("engine");
        m_engine.write(out);
        out.end_section();

        out.write("window_style", m_style);
        std::size_t index = 0;
        const std::string section = "entity";
        for (std::size_t i = 0; i < m_engine.size(); i++)
        {
            const sf::Shape &shape = *(m_shapes[i]);
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

        for (const auto &l : m_layers)
        {
            out.begin_section(l->m_name);
            l->write(out);
            out.end_section();
        }
    }

    void app::read(ini::input &in)
    {
        in.begin_section("engine");
        m_engine.read(in);
        in.end_section();

        recreate_window(in.readui32("window_style"));

        std::size_t index = 0;
        const std::string section = "entity";
        for (auto &shape : m_shapes)
        {
            in.begin_section(section + std::to_string(index)); // CLEANUP THIS
            const entity2D &e = m_engine.entities()[index++];
            if (const auto *poly = e.shape_if<geo::polygon>())
            {
                const sf::ConvexShape temp_shape = convex_shape_from(*poly);
                shape = std::make_unique<sf::ConvexShape>(temp_shape);
            }
            else
            {
                const sf::CircleShape temp_shape = circle_shape_from(e.shape<geo::circle>());
                shape = std::make_unique<sf::CircleShape>(temp_shape);
            }
            shape->setFillColor({(sf::Uint8)in.readui32("r"), (sf::Uint8)in.readui32("g"), (sf::Uint8)in.readui32("b")});
            in.end_section();

            for (const auto &l : m_layers)
            {
                in.begin_section(l->m_name);
                l->read(in);
                in.end_section();
            }
        }

        framerate(in.readui32("framerate"));
        m_integrations_per_frame = in.readui32("integ_per_frame");
        m_aligned_dt = (bool)in.readi16("aligned_dt");
        m_time_smoothness = in.readf32("time_smoothness");
        m_dt = in.readf32("timestep");
        in.begin_section("springs_color");
        m_springs_color = {(sf::Uint8)in.readui32("r"), (sf::Uint8)in.readui32("g"), (sf::Uint8)in.readui32("b")};
        in.end_section();
        in.begin_section("rigid_bars_color");
        m_rigid_bars_color = {(sf::Uint8)in.readui32("r"), (sf::Uint8)in.readui32("g"), (sf::Uint8)in.readui32("b")};
        in.end_section();
        m_paused = (bool)in.readi16("paused");

        sf::View view = m_window.getView();
        const float camx = in.readf32("camx"), camy = in.readf32("camy"),
                    width = in.readf32("width"), height = in.readf32("height");

        view.setCenter(camx, camy);
        view.setSize(width, height);
        m_window.setView(view);

        if (m_engine.collider().coldet() == collider2D::QUAD_TREE)
            resize_quad_tree_to_window();
    }

    void app::draw_spring(const glm::vec2 &p1, const glm::vec2 &p2) { draw_spring(p1, p2, m_springs_color); }
    void app::draw_rigid_bar(const glm::vec2 &p1, const glm::vec2 &p2) { draw_rigid_bar(p1, p2, m_rigid_bars_color); }

    void app::update_convex_shapes_from_polygons()
    {
        for (std::size_t i = 0; i < m_engine.size(); i++)
        {
            sf::ConvexShape shape = convex_shape_from(m_engine.entities()[i].shape<geo::polygon>());
            shape.setFillColor(m_shapes[i]->getFillColor());
            m_shapes[i] = std::make_unique<sf::ConvexShape>(shape);
        }
    }

    void app::layer_start()
    {
        for (const auto &l : m_layers)
            l->on_start();
    }

    void app::layer_render()
    {
        PERF_FUNCTION()
        for (const auto &l : m_layers)
            if (l->p_enabled)
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
        for (const auto &l : m_layers)
            l->on_end();
    }

    void app::draw_entities()
    {
        PERF_FUNCTION()
        for (std::size_t i = 0; i < m_shapes.size(); i++)
        {
            const entity2D_ptr e = m_engine[i];

            on_entity_draw(e, *m_shapes[i]);
            const geo::shape2D &shape = e->shape();
            const glm::vec2 centre = shape.centroid() * WORLD_TO_PIXEL;

            m_shapes[i]->setPosition(centre.x, centre.y);
            m_shapes[i]->setRotation(shape.rotation() * TO_DEGREES);
            m_window.draw(*m_shapes[i]);
        }
    }

    void app::draw_springs()
    {
        PERF_FUNCTION()
        for (const spring2D &sp : m_engine.springs())
        {
            const glm::vec2 p1 = (sp.e1()->pos() + sp.joint1()) * WORLD_TO_PIXEL,
                            p2 = (sp.e2()->pos() + sp.joint2()) * WORLD_TO_PIXEL;
            draw_spring(p1, p2);
        }
    }

    void app::draw_rigid_bars()
    {
        PERF_FUNCTION()
        for (const auto &ctr : m_engine.compeller().constraints())
        {
            const auto rb = std::dynamic_pointer_cast<rigid_bar2D>(ctr);
            if (!rb)
                continue;
            const glm::vec2 p1 = (rb->e1()->pos() + rb->joint1()) * WORLD_TO_PIXEL,
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
                if (ImGui::GetIO().WantCaptureKeyboard)
                    break;
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

    sf::ConvexShape app::convex_shape_from(geo::polygon poly) const
    {
        poly.rotation(0.f);

        sf::ConvexShape shape(poly.size());
        const glm::vec2 centroid = poly.centroid() * WORLD_TO_PIXEL,
                        origin = centroid - poly[0] * WORLD_TO_PIXEL;
        shape.setOrigin(origin.x, origin.y);

        for (std::size_t i = 0; i < poly.size(); i++)
        {
            const glm::vec2 point = (poly[i] - poly[0]) * WORLD_TO_PIXEL;
            shape.setPoint(i, {point.x, point.y});
        }
        shape.setPosition(centroid.x, centroid.y);
        return shape;
    }
    sf::CircleShape app::circle_shape_from(const geo::circle &c) const
    {
        const float scaled_radius = WORLD_TO_PIXEL * c.radius();
        sf::CircleShape shape(scaled_radius);
        shape.setOrigin(scaled_radius, scaled_radius);
        const glm::vec2 centroid = c.centroid() * WORLD_TO_PIXEL;
        shape.setPosition(centroid.x, centroid.y);
        return shape;
    }

    glm::vec2 app::pixel_mouse() const
    {
        const sf::Vector2i mpos = sf::Mouse::getPosition(m_window);
        const sf::Vector2f wpos = m_window.mapPixelToCoords(mpos);
        return glm::vec2(wpos.x, wpos.y);
    }

    glm::vec2 app::pixel_mouse_delta() const
    {
        return glm::vec2(ImGui::GetIO().MouseDelta.x, -ImGui::GetIO().MouseDelta.y);
    }

    void app::transform_camera(const glm::vec2 &dir) // TODO: rebuild quad tree si se esta usando
    {
        sf::View v = m_window.getView();
        v.move({dir.x, dir.y});
        m_window.setView(v);

        if (m_engine.collider().coldet() == collider2D::QUAD_TREE)
            resize_quad_tree_to_window();
    }

    void app::transform_camera(const glm::vec2 &dir, const glm::vec2 &size)
    {
        sf::View v = m_window.getView();
        v.setSize({size.x, size.y});
        v.move({dir.x, dir.y});
        m_window.setView(v);

        if (m_engine.collider().coldet() == collider2D::QUAD_TREE)
            resize_quad_tree_to_window();
    }

    void app::recreate_window(const sf::Uint32 style,
                              const glm::vec2 &center,
                              const glm::vec2 &size,
                              const char *name)
    {
        if (style & sf::Style::Fullscreen)
            m_window.create(sf::VideoMode::getFullscreenModes()[0], name, sf::Style::Fullscreen);
        else
            m_window.create(sf::VideoMode(800, 600), name, style);
        m_window.setView(sf::View({center.x, center.y}, {size.x, size.y}));
        m_window.setFramerateLimit(m_framerate);
        m_style = style;
    }

    void app::recreate_window(const sf::Uint32 style, const char *name)
    {
        const auto center = m_window.getView().getCenter(),
                   size = m_window.getView().getSize();
        recreate_window(style, glm::vec2(center.x, center.y), glm::vec2(size.x, size.y));
    }

    void app::resize_quad_tree_to_window()
    {
        const sf::View &v = m_window.getView();
        const glm::vec2 pos = glm::vec2(v.getCenter().x, v.getCenter().y),
                        size = {v.getSize().x, -v.getSize().y};
        const geo::aabb2D qt_size = {-PIXEL_TO_WORLD * (size - pos), // Not halving the size to give some margin
                                     PIXEL_TO_WORLD * (size + pos)};
        m_engine.collider().quad_tree().aabb(qt_size);
        m_engine.collider().rebuild_quad_tree();
    }

    void app::control_camera()
    {
        if (ImGui::GetIO().WantCaptureKeyboard ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            return;

        const sf::View &v = m_window.getView();
        const glm::vec2 size = glm::vec2(v.getSize().x, v.getSize().y);
        const float speed = 0.75f * raw_delta_time().asSeconds() * glm::length(size);
        glm::vec2 vel(0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            vel.x += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            vel.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            vel.y += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            vel.y -= speed;
        if (glm::length2(vel) > 0.f)
            transform_camera(vel);
    }

    void app::zoom(const float delta)
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return;
        const float factor = std::clamp(delta, -0.05f, 0.05f); // delta * 0.006f;

        const sf::View &v = m_window.getView();
        const glm::vec2 dir = (pixel_mouse() - glm::vec2(v.getCenter().x, v.getCenter().y)) * factor,
                        size = glm::vec2(v.getSize().x, v.getSize().y) * (1.f - factor);
        transform_camera(dir, size);
    }

    glm::vec2 app::world_mouse() const { return pixel_mouse() * PIXEL_TO_WORLD; }
    glm::vec2 app::world_mouse_delta() const { return pixel_mouse_delta() * PIXEL_TO_WORLD; }

    sf::Uint32 app::style() const { return m_style; }
    void app::style(const sf::Uint32 style) { m_style = style; }

    const engine2D &app::engine() const { return m_engine; }
    engine2D &app::engine() { return m_engine; }

    const std::vector<std::unique_ptr<sf::Shape>> &app::shapes() const { return m_shapes; }
    utils::vector_view<std::unique_ptr<sf::Shape>> app::shapes() { return m_shapes; }

    sf::Shape &app::operator[](std::size_t index) const { return *m_shapes[index]; }

    const sf::Color &app::entity_color() const
    {
        return m_entity_color;
    }
    const sf::Color &app::springs_color() const { return m_springs_color; }
    const sf::Color &app::rigid_bars_color() const { return m_rigid_bars_color; }

    void app::entity_color(const sf::Color &color) { m_entity_color = color; }
    void app::springs_color(const sf::Color &color) { m_springs_color = color; }
    void app::rigid_bars_color(const sf::Color &color) { m_rigid_bars_color = color; }

    void app::add_fonts() const
    {
        if (!std::filesystem::exists(FONTS_DIR))
            return;

        ImGuiIO &io = ImGui::GetIO();
        for (const auto &entry : std::filesystem::directory_iterator(FONTS_DIR)) // ADD MACRO FONTS DIR
        {
            const std::string &path = entry.path().string(),
                              extension = path.substr(path.find(".") + 1, path.size() - 1);
            if (extension != "ttf" && extension != "otf")
                continue;

            const float size_pixels = 13.f;
            io.Fonts->AddFontFromFileTTF(entry.path().string().c_str(), size_pixels);
        }
        io.Fonts->Build();
        if (!ImGui::SFML::UpdateFontTexture())
        {
            perror("ImGui font initialization failed\n");
            exit(EXIT_FAILURE);
        }
    }

    std::uint32_t app::integrations_per_frame() const { return m_integrations_per_frame; }
    void app::integrations_per_frame(std::uint32_t integrations_per_frame) { m_integrations_per_frame = integrations_per_frame; }

    float app::timestep() const { return m_dt; }
    void app::timestep(float ts) { m_dt = ts; }

    bool app::paused() const { return m_paused; }
    void app::paused(const bool paused) { m_paused = paused; }

    std::uint32_t app::framerate() const { return m_framerate; }
    void app::framerate(const std::uint32_t framerate)
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