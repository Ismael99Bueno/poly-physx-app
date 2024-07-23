#pragma once

#include "lynx/drawing/drawable.hpp"
#include "lynx/drawing/line.hpp"
#include "lynx/app/window.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace ppx
{
class spring_line2D final : public lynx::line2D
{
  public:
    spring_line2D(const glm::vec2 &p1 = {0.f, 0.f}, const glm::vec2 &p2 = {1.f, 0.f},
                  const lynx::color &color = lynx::color::white, std::size_t supports_count = 6);
    spring_line2D(const lynx::color &color, std::size_t supports_count = 6);

    void draw(lynx::window2D &window) const override;

    const glm::vec2 &p1() const override;
    const glm::vec2 &p2() const override;

    void p1(const glm::vec2 &p1) override;
    void p2(const glm::vec2 &p2) override;

    const lynx::color &color() const override;
    void color(const lynx::color &color) override;

    const kit::transform2D<float> *parent() const;
    void parent(const kit::transform2D<float> *parent);

    std::size_t supports_count() const;
    float supports_length() const;
    float left_padding() const;
    float right_padding() const;
    float min_height() const;

    void supports_length(float supports_length);
    void left_padding(float left_padding);
    void right_padding(float right_padding);
    void min_height(float min_height);

  private:
    std::size_t m_supports_count;
    float m_supports_length = 0.8f;
    float m_left_padding = 0.f;
    float m_right_padding = 0.f;
    float m_min_height = 1.f;

    lynx::line_strip2D m_line_strip;

    std::vector<glm::vec2> build_line_points(const glm::vec2 &p1, const glm::vec2 &p2);
    void update_line_points(const glm::vec2 &p1, const glm::vec2 &p2);
};
} // namespace ppx
