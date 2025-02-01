cmake_minimum_required(VERSION 3.16)
project(TimeKill LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent
include(FetchContent)

# Find Vulkan SDK (needs to be installed!)
find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK.")
endif()

# SPIRV-Reflect
FetchContent_Declare(
        SPIRV-Reflect
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
        GIT_TAG main
)
message(STATUS ">>> Fetching SPIRV-Reflect...")
FetchContent_MakeAvailable(SPIRV-Reflect)

# Debug: Check if FetchContent actually worked
message(STATUS ">>> Checking SPIRV-Reflect Directories:")
message(STATUS "SPIRV-Reflect Source Dir: ${SPIRV-Reflect_SOURCE_DIR}")
message(STATUS "SPIRV-Reflect Binary Dir: ${SPIRV-Reflect_BINARY_DIR}")

if (EXISTS "${CMAKE_BINARY_DIR}/_deps/spirv-reflect-src/spirv_reflect.h")
    set(SPIRV-Reflect_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/spirv-reflect-src" CACHE INTERNAL "")
    message(STATUS "✅ SPIRV-Reflect header found!")
    message(STATUS "🔧 Manually setting SPIRV-Reflect Source Dir: ${SPIRV-Reflect_SOURCE_DIR}")
else()
    message(FATAL_ERROR "❌ SPIRV-Reflect header is missing! Check FetchContent.")
endif()

if (NOT SPIRV-Reflect_SOURCE_DIR)
    message(FATAL_ERROR "SPIRV-Reflect source directory not set! Check FetchContent_Populate.")
endif()

# Fetch GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_MakeAvailable(glfw)

endif()

# Export library targets
set(GLFW_LIB glfw CACHE INTERNAL "GLFW library target")
set(VULKAN_LIB Vulkan::Vulkan CACHE INTERNAL "Vulkan library target")

# G#obal paths
add_subdirectory(src)
add_subdirectory(examples/basic_window)
add_subdirectory(examples/vulkan_window)

# Debug Logging (Optional)
option(ENABLE_DEBUG_LOGGING "Enable debug logging" OFF)

# Target Compile Definitions (For Debug Logging)
if(ENABLE_DEBUG_LOGGING)
    target_compile_definitions(time_kill PRIVATE DEBUG_LOGGING_ENABLED)
    message(STATUS "Debug logging is ENABLED (via ENABLE_DEBUG_LOGGING option).")
#else
    message(STATUS "Debug logging is DISABLED.")
endif()
