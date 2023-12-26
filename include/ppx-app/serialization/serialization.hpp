#pragma once

#include "ppx-app/app/app.hpp"
#include "kit/serialization/yaml/codec.hpp"

template <> struct kit::yaml::codec<ppx::app>
{
    static YAML::Node encode(const ppx::app &app)
    {
        YAML::Node node;
        node["Engine"] = app.world;
        for (const auto &shape : app.shapes())
            node["Shape colors"].push_back(shape->color().normalized);
        node["Paused"] = app.paused;
        node["Sync timestep"] = app.sync_timestep;
        node["Body color"] = app.body_color.normalized;
        node["Joints color"] = app.joint_color.normalized;
        node["Integrations per frame"] = app.integrations_per_frame;
        node["Framerate"] = app.framerate_cap();
        node["Camera position"] = app.window()->camera()->transform.position;
        node["Camera scale"] = app.window()->camera()->transform.scale;
        node["Camera rotation"] = app.window()->camera()->transform.rotation;
        return node;
    }
    static bool decode(const YAML::Node &node, ppx::app &app)
    {
        if (!node.IsMap() || node.size() < 11)
            return false;

        node["Engine"].as<ppx::world2D>(app.world);

        if (node["Shape colors"])
            for (std::size_t i = 0; i < app.shapes().size(); i++)
                app.shapes()[i]->color(lynx::color(node["Shape colors"][i].as<glm::vec4>()));

        app.paused = node["Paused"].as<bool>();
        app.sync_timestep = node["Sync timestep"].as<bool>();
        app.body_color = lynx::color(node["Body color"].as<glm::vec4>());
        app.joint_color = lynx::color(node["Joints color"].as<glm::vec4>());
        app.integrations_per_frame = node["Integrations per frame"].as<std::uint32_t>();
        app.limit_framerate(node["Framerate"].as<std::uint32_t>());

        app.window()->camera()->transform.position = node["Camera position"].as<glm::vec2>();
        app.window()->camera()->transform.scale = node["Camera scale"].as<glm::vec2>();
        app.window()->camera()->transform.rotation = node["Camera rotation"].as<float>();
        return true;
    }
};