#include "ppx-app/pch.hpp"
#include "ppx-app/app.hpp"
#include "ppx-app/menu_layer.hpp"

#include "lynx/geometry/camera.hpp"
#include "lynx/rendering/buffer.hpp"

#include "ppx/joints/revolute_joint2D.hpp"

namespace ppx
{
app::app(const rk::butcher_tableau &table, const std::size_t allocations, const char *name)
    : lynx::app2D(800, 600, name), m_engine(table, allocations)
{
    push_layer<menu_layer>();

    m_window = window();
    m_camera = m_window->set_camera<lynx::orthographic2D>(glm::vec2(m_window->swap_chain_aspect() * 50.f, -50.f));

    m_engine.integrator().min_dt(0.0002f);
    m_engine.integrator().max_dt(0.006f);
    m_engine.integrator().limited_timestep(false);

    add_engine_callbacks();
}

void app::add_engine_callbacks()
{
    const kit::callback<const entity2D::ptr &> add_shape{[this](const entity2D::ptr &e) {
        if (const auto *c = e->shape_if<geo::circle>())
        {
            m_shapes.emplace_back(kit::make_scope<lynx::ellipse2D>(c->radius(), entity_color));
            return;
        }
        const geo::polygon &poly = e->shape<geo::polygon>();
        m_shapes.emplace_back(kit::make_scope<lynx::polygon2D>(
            std::vector<glm::vec2>(poly.locals().begin(), poly.locals().end()), entity_color));
    }};

    const kit::callback<std::size_t> remove_shape{[this](const std::size_t index) {
        m_shapes[index] = std::move(m_shapes.back());
        m_shapes.pop_back();
    }};

    const kit::callback<const spring2D::ptr &> add_spring{[this](const spring2D::ptr &sp) {
        if (sp->has_anchors())
            m_spring_lines.emplace_back(sp->e1()->pos() + sp->anchor1(), sp->e2()->pos() + sp->anchor2(), 6,
                                        joint_color);
        else
            m_spring_lines.emplace_back(sp->e1()->pos(), sp->e2()->pos(), 6, joint_color);
    }};
    const kit::callback<const spring2D &> remove_spring{
        [this](const spring2D &sp) { m_spring_lines.erase(m_spring_lines.begin() + (long)sp.index()); }};

    const kit::callback<constraint_interface2D *> add_revolute{[this](constraint_interface2D *ctr) {
        const auto *rj = dynamic_cast<revolute_joint2D *>(ctr);
        if (!rj)
            return;
        if (rj->has_anchors())
            m_thick_lines.emplace(
                rj, thick_line(rj->e1()->pos() + rj->anchor1(), rj->e2()->pos() + rj->anchor2(), 1.f, joint_color));
        else
            m_thick_lines.emplace(rj, thick_line(rj->e1()->pos(), rj->e2()->pos(), 1.f, joint_color));
    }};
    const kit::callback<const constraint_interface2D &> remove_revolute{[this](const constraint_interface2D &ctr) {
        const auto *rj = dynamic_cast<const revolute_joint2D *>(&ctr);
        if (!rj)
            return;
        m_thick_lines.erase(rj);
    }};

    m_engine.events().on_entity_addition += add_shape;
    m_engine.events().on_late_entity_removal += remove_shape;

    m_engine.events().on_spring_addition += add_spring;
    m_engine.events().on_spring_removal += remove_spring;

    m_engine.events().on_constraint_addition += add_revolute;
    m_engine.events().on_constraint_removal += remove_revolute;
}

void app::on_update(const float ts)
{
    const kit::clock update_clock;
    if (m_sync_timestep)
        m_timestep = ts;

    const kit::clock physics_clock;
    if (!m_paused)
        for (std::uint32_t i = 0; i < integrations_per_frame; i++)
            m_engine.raw_forward(m_timestep);
    m_physics_time = physics_clock.elapsed();

    update_entities();
    update_joints();
    move_camera();
    m_update_time = update_clock.elapsed();
}

void app::on_render(const float ts)
{
    const kit::clock draw_clock;
    draw_entities();
    draw_joints();
    m_draw_time = draw_clock.elapsed();
}

bool app::on_event(const lynx::event &event)
{
    switch (event.type)
    {
    case lynx::event::KEY_PRESSED:
        if (ImGui::GetIO().WantCaptureKeyboard)
            break;
        switch (event.key)
        {
        case lynx::input::key::F:
            if (m_engine.size() > 0)
                m_engine.remove_entity(m_engine.size() - 1);
            break;
        case lynx::input::key::ESCAPE:
            shutdown();
            return false;
        case lynx::input::key::SPACE:
            m_paused = !m_paused;
            return false;
        default:
            return false;
        }
    case lynx::event::SCROLLED:
        if (ImGui::GetIO().WantCaptureMouse)
            break;
        zoom(event.scroll_offset.y);
    default:
        return false;
    }
    return false;
}

void app::update_entities()
{
    const auto entities = m_engine.entities();
    for (std::size_t i = 0; i < entities.unwrap().size(); i++)
    {
        const entity2D &e = entities[i];
        m_shapes[i]->transform.position = e.pos();
        m_shapes[i]->transform.rotation = e.angpos();
        on_entity_update(e, *m_shapes[i]);
    }
}
void app::update_joints()
{
    const auto springs = m_engine.springs();
    for (std::size_t i = 0; i < springs.unwrap().size(); i++)
    {
        const spring2D &sp = springs[i];
        spring_line &spline = m_spring_lines[i];
        if (sp.has_anchors())
        {
            spline.p1(sp.e1()->pos() + sp.anchor1());
            spline.p2(sp.e2()->pos() + sp.anchor2());
        }
        else
        {
            spline.p1(sp.e1()->pos());
            spline.p2(sp.e2()->pos());
        }
    }

    for (auto &[rj, thline] : m_thick_lines)
    {
        if (rj->has_anchors())
        {
            thline.p1(rj->e1()->pos() + rj->anchor1());
            thline.p2(rj->e2()->pos() + rj->anchor2());
        }
        else
        {
            thline.p1(rj->e1()->pos());
            thline.p2(rj->e2()->pos());
        }
    }
}

void app::draw_entities() const
{
    for (const auto &shape : m_shapes)
        m_window->draw(*shape);
}

void app::draw_joints() const
{
    for (const spring_line &spline : m_spring_lines)
        m_window->draw(spline);
    for (const auto &[rj, thline] : m_thick_lines)
        m_window->draw(thline);
}

void app::move_camera()
{
    glm::vec2 dpos{0.f};
    if (lynx::input::key_pressed(lynx::input::key::A))
        dpos.x = -1.f;
    if (lynx::input::key_pressed(lynx::input::key::D))
        dpos.x = 1.f;
    if (lynx::input::key_pressed(lynx::input::key::W))
        dpos.y = 1.f;
    if (lynx::input::key_pressed(lynx::input::key::S))
        dpos.y = -1.f;
    if (glm::length2(dpos) > std::numeric_limits<float>::epsilon())
        m_camera->transform.position += glm::normalize(dpos) * abs(m_camera->size()) / 50.f;
}

void app::zoom(const float offset)
{
    const float factor = offset * 0.05f; // glm::clamp(offset, -0.05f, 0.05f);
    const glm::vec2 mpos = mouse_position();
    const glm::vec2 dpos = (mpos - m_camera->transform.position) * factor;
    const float size = m_camera->size() * (1.f - factor);

    m_camera->transform.scale.x = -m_window->swap_chain_aspect() * size;
    m_camera->transform.scale.y = size;
    m_camera->transform.position += dpos;
}

glm::vec2 app::mouse_position() const
{
    return m_camera->screen_to_world(lynx::input::mouse_position());
}

engine2D &app::engine()
{
    return m_engine;
}
const engine2D &app::engine() const
{
    return m_engine;
}

float app::timestep() const
{
    return m_timestep;
}
void app::timestep(float ts)
{
    m_timestep = ts;
}

bool app::sync_timestep() const
{
    return m_sync_timestep;
}
void app::sync_timestep(const bool sync)
{
    m_sync_timestep = sync;
}

kit::time app::update_time() const
{
    return m_update_time;
}
kit::time app::physics_time() const
{
    return m_physics_time;
}
kit::time app::draw_time() const
{
    return m_draw_time;
}

// // HACER INTERFAZ NAMEABLE, TOGGLEABLE
// #ifdef KIT_USE_YAML_CPP
// YAML::Emitter &operator<<(YAML::Emitter &out, const app &papp)
// {
//     out << YAML::BeginMap;
//     out << YAML::Key << "Engine" << YAML::Value << papp.engine();
//     out << YAML::Key << "Timestep" << YAML::Value << papp.timestep();
//     out << YAML::Key << "Layers" << YAML::Value << YAML::BeginMap;
//     for (const auto &l : papp.layers())
//         out << YAML::Key << l->name() << YAML::Value << *l;
//     out << YAML::EndMap;

//     out << YAML::Key << "Shape colors" << YAML::Value << YAML::BeginSeq;
//     for (const auto &shape : papp.shapes())
//         out << shape->getFillColor();
//     out << YAML::EndSeq;

//     out << YAML::Key << "Paused" << YAML::Value << papp.paused();
//     out << YAML::Key << "Sync timestep" << YAML::Value << papp.sync_timestep();
//     out << YAML::Key << "Time smoothness" << YAML::Value << papp.time_measure_smoothness();
//     out << YAML::Key << "Entity color" << YAML::Value << papp.entity_color();
//     out << YAML::Key << "Springs color" << YAML::Value << papp.springs_color();
//     out << YAML::Key << "Rigid bars color" << YAML::Value << papp.rigid_bars_color();
//     out << YAML::Key << "Integrations per frame" << YAML::Value << papp.integrations_per_frame();
//     out << YAML::Key << "Framerate" << YAML::Value << papp.framerate();
//     const sf::View &view = papp.window().getView();
//     const glm::vec2 center = {view.getCenter().x, view.getCenter().y}, size = {view.getSize().x, view.getSize().y};
//     out << YAML::Key << "Camera center" << YAML::Value << center;
//     out << YAML::Key << "Camera size" << YAML::Value << size;
//     out << YAML::EndMap;
//     return out;
// }
// #endif
} // namespace ppx

// #ifdef KIT_USE_YAML_CPP
// namespace YAML
// {
// Node convert<ppx::app>::encode(const ppx::app &papp)
// {
//     Node node;
//     node["Engine"] = papp.engine();
//     node["Timestep"] = papp.timestep();
//     for (const auto &l : papp.m_layers)
//         node["Layers"][l->name()] = *l;
//     for (const auto &shape : papp.shapes())
//         node["Shape colors"].push_back(shape->getFillColor());
//     node["Paused"] = papp.paused();
//     node["Sync timestep"] = papp.sync_timestep();
//     node["Time smoothness"] = papp.time_measure_smoothness();
//     node["Entity color"] = papp.entity_color();
//     node["Springs color"] = papp.springs_color();
//     node["Rigid bars color"] = papp.rigid_bars_color();
//     node["Integrations per frame"] = papp.integrations_per_frame();
//     node["Framerate"] = papp.framerate();
//     const sf::View &view = papp.window().getView();
//     const glm::vec2 center = {view.getCenter().x, view.getCenter().y}, size = {view.getSize().x, view.getSize().y};
//     node["Camera center"] = center;
//     node["Camera size"] = size;
//     return node;
// }
// bool convert<ppx::app>::decode(const Node &node, ppx::app &papp)
// {
//     if (!node.IsMap() || node.size() != 15)
//         return false;

//     node["Engine"].as<ppx::engine2D>(papp.engine());
//     papp.timestep(node["Timestep"].as<float>());
//     papp.recreate_window(node["Window style"].as<std::uint32_t>());
//     for (const auto &l : papp.m_layers)
//         if (node["Layers"][l->name()])
//             node["Layers"][l->name()].as<ppx::layer>(*l);

//     for (std::size_t i = 0; i < papp.m_shapes.size(); i++)
//         papp.m_shapes[i]->setFillColor(node["Shape colors"][i].as<sf::Color>());
//     papp.update_shapes();

//     papp.paused(node["Paused"].as<bool>());
//     papp.sync_timestep(node["Sync timestep"].as<bool>());
//     papp.time_measure_smoothness(node["Time smoothness"].as<float>());
//     papp.entity_color(node["Entity color"].as<sf::Color>());
//     papp.springs_color(node["Springs color"].as<sf::Color>());
//     papp.rigid_bars_color(node["Rigid bars color"].as<sf::Color>());
//     papp.integrations_per_frame(node["Integrations per frame"].as<std::uint32_t>());
//     papp.framerate(node["Framerate"].as<std::uint32_t>());

//     const glm::vec2 center = node["Camera center"].as<glm::vec2>(), size = node["Camera size"].as<glm::vec2>();
//     sf::View view = papp.window().getView();
//     view.setCenter(center.x, center.y);
//     view.setSize(size.x, size.y);
//     papp.window().setView(view);
//     return true;
// };
// } // namespace YAML
// #endif
