#include "ppx-app/pch.hpp"
#include "ppx-app/app.hpp"
#include "prm/spring_line.hpp"
#include "prm/thick_line.hpp"
#include "ppx/rigid_bar2D.hpp"

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
        recreate_window(sf::Style::Default, {0.f, 0.f}, {PPX_WIDTH, -PPX_HEIGHT}, name);
        push_layer<menu_layer>();

        m_window.setVerticalSyncEnabled(false);
        m_engine.integrator().min_dt(0.0002f);
        m_engine.integrator().max_dt(0.006f);

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
            DBG_CRITICAL("ImGui initialization failed");
            exit(EXIT_FAILURE);
        }
        add_fonts();
        framerate(PPX_DEFAULT_FPS);
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
                layer_update();

                draw_entities();
                draw_springs();
                draw_rigid_bars();

                on_render();
                layer_render();

                on_late_update();
                layer_late_update();
                control_camera();
                ImGui::SFML::Render(m_window);
                m_window.display();

                m_raw_draw_time = draw_clock.getElapsedTime();
                m_draw_time = (1.f - m_time_smoothness) * m_raw_draw_time +
                              m_time_smoothness * m_draw_time;
            }
            if (m_sync_dt)
                sync_dt();
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

    void app::draw_spring(const glm::vec2 &p1, const glm::vec2 &p2) { draw_spring(p1, p2, m_springs_color); }
    void app::draw_rigid_bar(const glm::vec2 &p1, const glm::vec2 &p2) { draw_rigid_bar(p1, p2, m_rigid_bars_color); }

    void app::update_shapes()
    {
        for (std::size_t i = 0; i < m_engine.size(); i++)
        {
            const ppx::entity2D &e = m_engine.entities()[i];
            if (const auto *poly = e.shape_if<geo::polygon>())
            {
                sf::ConvexShape shape = convex_shape_from(*poly);
                shape.setFillColor(m_shapes[i]->getFillColor());
                m_shapes[i] = std::make_unique<sf::ConvexShape>(shape);
            }
            else
            {
                const auto &c = e.shape<geo::circle>();
                sf::CircleShape shape = circle_shape_from(c);
                shape.setFillColor(m_shapes[i]->getFillColor());
                m_shapes[i] = std::make_unique<sf::CircleShape>(shape);
            }
        }
    }

    void app::layer_start()
    {
        for (const auto &l : m_layers)
            if (l->p_enabled)
                l->on_start();
    }

    void app::layer_update()
    {
        for (const auto &l : m_layers)
            if (l->p_enabled)
                l->on_update();
    }

    void app::layer_late_update()
    {
        for (const auto &l : m_layers)
            if (l->p_enabled)
                l->on_late_update();
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
            const glm::vec2 center = shape.centroid() * PPX_WORLD_TO_PIXEL;

            m_shapes[i]->setPosition(center.x, center.y);
            m_shapes[i]->setRotation(shape.rotation() * TO_DEGREES);
            m_window.draw(*m_shapes[i]);
        }
    }

    void app::draw_springs()
    {
        PERF_FUNCTION()
        for (const spring2D &sp : m_engine.springs())
        {
            const glm::vec2 p1 = (sp.e1()->pos() + sp.anchor1()) * PPX_WORLD_TO_PIXEL,
                            p2 = (sp.e2()->pos() + sp.anchor2()) * PPX_WORLD_TO_PIXEL;
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
            const glm::vec2 p1 = (rb->e1()->pos() + rb->anchor1()) * PPX_WORLD_TO_PIXEL,
                            p2 = (rb->e2()->pos() + rb->anchor2()) * PPX_WORLD_TO_PIXEL;
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

    void app::sync_dt()
    {
        const rk::integrator &integ = m_engine.integrator();
        m_dt = std::clamp(raw_delta_time().asSeconds(), integ.min_dt(), integ.max_dt());
    }

    sf::ConvexShape app::convex_shape_from(const geo::polygon &poly) const
    {
        sf::ConvexShape shape(poly.size());
        const glm::vec2 centroid = poly.centroid() * PPX_WORLD_TO_PIXEL,
                        origin = -poly.locals()[0] * PPX_WORLD_TO_PIXEL;
        shape.setOrigin(origin.x, origin.y);

        for (std::size_t i = 0; i < poly.size(); i++)
        {
            const glm::vec2 point = (poly.locals()[i] - poly.locals()[0]) * PPX_WORLD_TO_PIXEL;
            shape.setPoint(i, {point.x, point.y});
        }
        shape.setPosition(centroid.x, centroid.y);
        shape.setRotation(poly.rotation() * TO_DEGREES);
        return shape;
    }
    sf::CircleShape app::circle_shape_from(const geo::circle &c) const
    {
        const float scaled_radius = PPX_WORLD_TO_PIXEL * c.radius();
        sf::CircleShape shape(scaled_radius);
        shape.setOrigin(scaled_radius, scaled_radius);
        const glm::vec2 centroid = c.centroid() * PPX_WORLD_TO_PIXEL;
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
        const geo::aabb2D qt_size = {-PPX_PIXEL_TO_WORLD * (size - pos), // Not halving the size to give some margin
                                     PPX_PIXEL_TO_WORLD * (size + pos)};
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

    glm::vec2 app::world_mouse() const { return pixel_mouse() * PPX_PIXEL_TO_WORLD; }
    glm::vec2 app::world_mouse_delta() const { return pixel_mouse_delta() * PPX_PIXEL_TO_WORLD; }

    sf::Uint32 app::style() const { return m_style; }
    void app::style(const sf::Uint32 style) { m_style = style; }

    const engine2D &app::engine() const { return m_engine; }
    engine2D &app::engine() { return m_engine; }

    const std::vector<std::unique_ptr<sf::Shape>> &app::shapes() const { return m_shapes; }
    cvw::vector<std::unique_ptr<sf::Shape>> app::shapes() { return m_shapes; }

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
            DBG_CRITICAL("ImGui font initialization failed");
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

    bool app::sync_timestep() const { return m_sync_dt; }
    void app::sync_timestep(const bool aligned_dt) { m_sync_dt = aligned_dt; }

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

#ifdef HAS_YAML_CPP
    YAML::Emitter &operator<<(YAML::Emitter &out, const app &papp)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Engine" << YAML::Value << papp.engine();
        out << YAML::Key << "Timestep" << YAML::Value << papp.timestep();
        out << YAML::Key << "Window style" << YAML::Value << papp.style();
        out << YAML::Key << "Layers" << YAML::Value << YAML::BeginMap;
        for (const auto &l : papp.m_layers)
            out << YAML::Key << l->name() << YAML::Value << *l;
        out << YAML::EndMap;

        out << YAML::Key << "Shape colors" << YAML::Value << YAML::BeginSeq;
        for (const auto &shape : papp.shapes())
            out << shape->getFillColor();
        out << YAML::EndSeq;

        out << YAML::Key << "Paused" << YAML::Value << papp.paused();
        out << YAML::Key << "Sync timestep" << YAML::Value << papp.sync_timestep();
        out << YAML::Key << "Time smoothness" << YAML::Value << papp.time_measure_smoothness();
        out << YAML::Key << "Entity color" << YAML::Value << papp.entity_color();
        out << YAML::Key << "Springs color" << YAML::Value << papp.springs_color();
        out << YAML::Key << "Rigid bars color" << YAML::Value << papp.rigid_bars_color();
        out << YAML::Key << "Integrations per frame" << YAML::Value << papp.integrations_per_frame();
        out << YAML::Key << "Framerate" << YAML::Value << papp.framerate();
        const sf::View &view = papp.window().getView();
        const glm::vec2 center = {view.getCenter().x, view.getCenter().y},
                        size = {view.getSize().x, view.getSize().y};
        out << YAML::Key << "Camera center" << YAML::Value << center;
        out << YAML::Key << "Camera size" << YAML::Value << size;
        out << YAML::EndMap;
        return out;
    }
#endif
}

#ifdef HAS_YAML_CPP
namespace sf
{
    YAML::Emitter &operator<<(YAML::Emitter &out, const Color &color)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << (int)color.r << (int)color.g << (int)color.b << YAML::EndSeq;
        return out;
    }
}
#endif

#ifdef HAS_YAML_CPP
namespace YAML
{
    Node convert<ppx::app>::encode(const ppx::app &papp)
    {
        Node node;
        node["Engine"] = papp.engine();
        node["Timestep"] = papp.timestep();
        node["Window style"] = papp.style();
        for (const auto &l : papp.m_layers)
            node["Layers"][l->name()] = *l;
        for (const auto &shape : papp.shapes())
            node["Shape colors"].push_back(shape->getFillColor());
        node["Paused"] = papp.paused();
        node["Sync timestep"] = papp.sync_timestep();
        node["Time smoothness"] = papp.time_measure_smoothness();
        node["Entity color"] = papp.entity_color();
        node["Springs color"] = papp.springs_color();
        node["Rigid bars color"] = papp.rigid_bars_color();
        node["Integrations per frame"] = papp.integrations_per_frame();
        node["Framerate"] = papp.framerate();
        const sf::View &view = papp.window().getView();
        const glm::vec2 center = {view.getCenter().x, view.getCenter().y},
                        size = {view.getSize().x, view.getSize().y};
        node["Camera center"] = center;
        node["Camera size"] = size;
        return node;
    }
    bool convert<ppx::app>::decode(const Node &node, ppx::app &papp)
    {
        if (!node.IsMap() || node.size() != 15)
            return false;

        node["Engine"].as<ppx::engine2D>(papp.engine());
        papp.timestep(node["Timestep"].as<float>());
        papp.recreate_window(node["Window style"].as<std::uint32_t>());
        for (const auto &l : papp.m_layers)
            if (node["Layers"][l->name()])
                node["Layers"][l->name()].as<ppx::layer>(*l);

        for (std::size_t i = 0; i < papp.m_shapes.size(); i++)
            papp.m_shapes[i]->setFillColor(node["Shape colors"][i].as<sf::Color>());
        papp.update_shapes();

        papp.paused(node["Paused"].as<bool>());
        papp.sync_timestep(node["Sync timestep"].as<bool>());
        papp.time_measure_smoothness(node["Time smoothness"].as<float>());
        papp.entity_color(node["Entity color"].as<sf::Color>());
        papp.springs_color(node["Springs color"].as<sf::Color>());
        papp.rigid_bars_color(node["Rigid bars color"].as<sf::Color>());
        papp.integrations_per_frame(node["Integrations per frame"].as<std::uint32_t>());
        papp.framerate(node["Framerate"].as<std::uint32_t>());

        const glm::vec2 center = node["Camera center"].as<glm::vec2>(),
                        size = node["Camera size"].as<glm::vec2>();
        sf::View view = papp.window().getView();
        view.setCenter(center.x, center.y);
        view.setSize(size.x, size.y);
        papp.window().setView(view);
        return true;
    };

    Node convert<sf::Color>::encode(const sf::Color &color)
    {
        Node node;
        node.push_back((int)color.r);
        node.push_back((int)color.g);
        node.push_back((int)color.b);
        node.SetStyle(YAML::EmitterStyle::Flow);
        return node;
    }
    bool convert<sf::Color>::decode(const Node &node, sf::Color &color)
    {
        if (!node.IsSequence() || node.size() != 3)
            return false;
        color.r = (sf::Uint8)node[0].as<int>();
        color.g = (sf::Uint8)node[1].as<int>();
        color.b = (sf::Uint8)node[2].as<int>();

        return true;
    };
}
#endif