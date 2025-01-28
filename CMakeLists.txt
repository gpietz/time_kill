cmake_minimum_required(VERSION 3.16)
project(TimeKill LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent
include(FetchContent)

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

# Find Vulkan SDK (needs to be installed!)
find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK.")
endif()

# Export library targets
set(GLFW_LIB glfw CACHE INTERNAL "GLFW library target")
set(VULKAN_LIB Vulkan::Vulkan CACHE INTERNAL "Vulkan library target")

# Debug Logging (Optional)
option(ENABLE_DEBUG_LOGGING "Enable debug logging" OFF)

# Target Compile Definitions (For Debug Logging)
if(ENABLE_DEBUG_LOGGING)
    target_compile_definitions(timekill PRIVATE DEBUG_LOGGING_ENABLED)
    message(STATUS "Debug logging is ENABLED (via ENABLE_DEBUG_LOGGING option).")
#else
    message(STATUS "Debug logging is DISABLED.")
endif()

# G#obal paths
add_subdirectory(src)
add_subdirectory(examples/basic_window)
add_subdirectory(examples/vulkan_window)
