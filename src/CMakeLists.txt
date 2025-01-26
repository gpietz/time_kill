# Headers and sources
file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.hpp)

add_library(time_kill STATIC ${SOURCES} ${HEADERS})

# Set the output directory for the library
set_target_properties(time_kill PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories(time_kill PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${glfw_SOURCE_DIR}/include)
target_link_libraries(time_kill PUBLIC glfw Vulkan::Vulkan)
