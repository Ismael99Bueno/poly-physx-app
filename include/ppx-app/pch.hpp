#ifndef PPX_APP_PCH_HPP
#define PPX_APP_PCH_HPP

#include <string>
#include <vector>
#include <functional>
#include <imgui.h>
#include <filesystem>
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#ifdef LYNX_ENABLE_IMGUI
#include <imgui.h>
#ifdef LYNX_ENABLE_IMPLOT
#include <implot.h>
#endif
#endif
#include "ppx/internal/pch.hpp"

#endif