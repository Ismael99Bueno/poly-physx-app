#pragma once

#include "ppx-app/app/app.hpp"
#include "ppx/serialization/serialization.hpp"
#include "kit/serialization/yaml/codec.hpp"
#include "kit/serialization/yaml/glm.hpp"
#include "lynx/serialization/serialization.hpp"

template <> struct kit::yaml::codec<ppx::app>
{
    static YAML::Node encode(const ppx::app &app)
    {
        YAML::Node node;
        node["Lynx app"] = kit::yaml::codec<lynx::app2D>::encode(app);

        node["Engine"] = app.world;
        for (const auto &[collider, crepr] : app.shapes())
            node["Shape colors"][collider->index] = crepr.color.rgba;
        node["Sleep greyout"] = app.sleep_greyout;
        node["Paused"] = app.paused;
        node["Sync timestep"] = app.sync_timestep;
        node["Sync speed"] = app.sync_speed;
        node["Collider color"] = app.collider_color.rgba;
        node["Joints color"] = app.joint_color.rgba;
        node["Integrations per frame"] = app.integrations_per_frame;
        node["Framerate"] = app.framerate_cap();
        node["Camera position"] = app.window()->camera()->transform.position;
        node["Camera scale"] = app.window()->camera()->transform.scale;
        node["Camera rotation"] = app.window()->camera()->transform.rotation;
        return node;
    }
    static bool decode(const YAML::Node &node, ppx::app &app)
    {
        if (!node.IsMap() || node.size() < 12)
            return false;

        app.collider_color = lynx::color(node["Collider color"].as<glm::vec4>());
        app.joint_color = lynx::color(node["Joints color"].as<glm::vec4>());

        kit::yaml::codec<lynx::app2D>::decode(node["Lynx app"], app);
        node["Engine"].as<ppx::world2D>(app.world);

        app.sleep_greyout = node["Sleep greyout"].as<float>();
        if (node["Shape colors"])
            for (auto &[collider, repr] : app.shapes())
                app.color(collider, lynx::color(node["Shape colors"][collider->index].as<glm::vec4>()));

        app.paused = node["Paused"].as<bool>();
        app.sync_timestep = node["Sync timestep"].as<bool>();
        app.sync_speed = node["Sync speed"].as<float>();
        app.integrations_per_frame = node["Integrations per frame"].as<std::uint32_t>();
        app.limit_framerate(node["Framerate"].as<std::uint32_t>());

        app.window()->camera()->transform.position = node["Camera position"].as<glm::vec2>();
        app.window()->camera()->transform.scale = node["Camera scale"].as<glm::vec2>();
        app.window()->camera()->transform.rotation = node["Camera rotation"].as<float>();
        return true;
    }
};