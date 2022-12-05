#pragma once

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

#include "coil/Coil.h"

#include "nstl/vector.h"
#include "nstl/optional.h"
#include "nstl/array.h"
#include "nstl/string.h"
#include "nstl/string_view.h"
#include "nstl/unordered_map.h"
#include "nstl/span.h"
#include "nstl/algorithm.h"
#include "nstl/function.h"
#include "nstl/unique_ptr.h"
#include "nstl/string_builder.h"

#include <assert.h>
