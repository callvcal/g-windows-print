cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME "g_windows_print")
project(${PROJECT_NAME} LANGUAGES CXX)

cmake_policy(VERSION 3.14...3.25)

set(PLUGIN_NAME "g_windows_print_plugin")

# Include all plugin sources and headers
list(APPEND PLUGIN_SOURCES
        "g_windows_print_plugin.cpp"
        "g_windows_print_plugin_c_api.cpp"
        "include/g_windows_print/g_windows_print_plugin.h"
        "include/g_windows_print/g_windows_print_plugin_c_api.h"
)

# Create the shared library (DLL + .lib)
add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES})

# Apply standard Flutter build settings
apply_standard_settings(${PLUGIN_NAME})

# Hide symbols by default
set_target_properties(${PLUGIN_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
)

# Define plugin macro for compilation
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)

# Include headers for this target
target_include_directories(${PLUGIN_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Link against Flutter engine
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)

# Export list of bundled libraries (none in this case)
set(g_windows_print_bundled_libraries
        ""
        PARENT_SCOPE
)
