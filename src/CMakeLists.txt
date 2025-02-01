# Headers and sources
set(SOURCES
    core/logger.cpp
    core/window.cpp
    graphics/vulkan_context.cpp
    graphics/vulkan_mappings.cpp
    graphics/vulkan_swapchain.cpp
    graphics/vulkan_tools.cpp
    graphics/vulkan_render_pass.cpp
    graphics/vulkan_graphics_pipeline.cpp
    utils/string_utils.cpp
    ${SPIRV-Reflect_SOURCE_DIR}/spirv_reflect.c
)

set(HEADERS
    prerequisites.hpp
    core/logger.hpp
    core/window.hpp
    core/window_config.hpp
    graphics/graphic_types.hpp
    graphics/vulkan_context.hpp
    graphics/vulkan_mappings.hpp
    graphics/vulkan_resources.hpp
    graphics/vulkan_swapchain.hpp
    graphics/vulkan_render_pass.hpp
    graphics/vulkan_graphics_pipeline.hpp
    graphics/vulkan_configuration.hpp
    utils/string_utils.hpp
    ${SPIRV-Reflect_SOURCE_DIR}/spirv_reflect.h)

# Add sources and headers to library
add_library(time_kill STATIC ${SOURCES} ${HEADERS})

# Set the output directory for the library
set_target_properties(time_kill PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories(time_kill PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${glfw_SOURCE_DIR}/include
    ${SPIRV-Reflect_SOURCE_DIR}
)
target_link_libraries(time_kill PUBLIC glfw Vulkan::Vulkan)
