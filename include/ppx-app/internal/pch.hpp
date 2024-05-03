
#include <string>
#include <vector>
#include <functional>
#include <imgui.h>
#include <filesystem>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#ifdef LYNX_ENABLE_IMGUI
#include <imgui.h>
#ifdef LYNX_ENABLE_IMPLOT
#include <implot.h>
#endif
#endif
#include "ppx/internal/pch.hpp"
