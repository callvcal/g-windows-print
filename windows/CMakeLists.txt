cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME "g_windows_print")
project(${PROJECT_NAME} LANGUAGES CXX)

cmake_policy(VERSION 3.14...3.25)

set(PLUGIN_NAME "g_windows_print_plugin")

add_library(${PLUGIN_NAME} SHARED
        "g_windows_print_plugin.cpp"
        "g_windows_print_plugin_c_api.cpp"
)

apply_standard_settings(${PLUGIN_NAME})

set_target_properties(${PLUGIN_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden)

target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)

target_include_directories(${PLUGIN_NAME} INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)

set(g_windows_print_bundled_libraries
        ""
        PARENT_SCOPE
)
