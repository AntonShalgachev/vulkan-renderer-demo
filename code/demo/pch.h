#pragma once

#include "glm.h"
#include "vkgfx/Renderer.h"
#include "vkgfx/ResourceManager.h"
#include "vkgfx/Handles.h"
#include "vkgfx/PipelineKey.h"
#include "vkgfx/ImageMetadata.h"
#include "vkgfx/Texture.h"
#include "vkgfx/Mesh.h"
#include "vkgfx/Material.h"
#include "vkgfx/BufferMetadata.h"
#include "common/Utils.h"
#include "services/Services.h"
#include "services/DebugConsoleService.h"
#include "services/CommandLineService.h"
#include "services/DebugDrawService.h"
#include "ScopedDebugCommands.h"

#include "argparse/argparse.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "coil/Coil.h"
#include "magic_enum.hpp"

#include "nstl/vector.h"
#include "nstl/optional.h"
#include "nstl/array.h"
#include "nstl/string.h"
#include "nstl/string_view.h"
#include "nstl/unordered_map.h"
#include "nstl/span.h"

#include <functional>
#include <memory>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <set>
#include <map>
#include <algorithm>
#include <deque>
#include <cassert>
