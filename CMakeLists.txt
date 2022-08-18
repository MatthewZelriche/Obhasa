cmake_minimum_required(VERSION 3.20)

project(Obhasa CXX)

add_subdirectory("${PROJECT_SOURCE_DIR}/Vendor/glfw")
set(DILIGENT_BUILD_FX OFF)
set(DILIGENT_BUILD_SAMPLES OFF)
set(DILIGENT_BUILD_TESTS OFF)
set(DILIGENT_INSTALL_SAMPLES OFF)
add_subdirectory("${PROJECT_SOURCE_DIR}/Vendor/DiligentEngine")

add_executable(Obhasa src/main.cpp)
set_target_properties(Obhasa PROPERTIES 
                        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/bin"
                        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/bin")

# Set RPATH to be the bin directory on linux
# so we can ship a single, ready-to-go folder
if (PLATFORM_LINUX)
    set(CMAKE_BUILD_RPATH "./")
endif()

target_link_libraries(Obhasa
    Diligent-GraphicsEngineVk-shared
    Diligent-GraphicsEngineOpenGL-shared
)
target_link_libraries(Obhasa glfw)
target_include_directories(Obhasa PRIVATE "${CMAKE_SOURCE_DIR}/Vendor/glfw/include/")
target_include_directories(Obhasa PRIVATE "${CMAKE_SOURCE_DIR}/Vendor/DiligentEngine/DiligentCore/")

# Copy shared libs
if (PLATFORM_WIN32)
    include("${PROJECT_SOURCE_DIR}/Vendor/DiligentEngine/DiligentCore/BuildUtils.cmake")
    copy_required_dlls(Obhasa)
elseif(PLATFORM_LINUX)
    add_custom_command(TARGET Obhasa POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "\"$<TARGET_FILE:Diligent-GraphicsEngineVk-shared>\""
        "\"$<TARGET_FILE_DIR:Obhasa>\"")
    add_custom_command(TARGET Obhasa POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "\"$<TARGET_FILE:Diligent-GraphicsEngineOpenGL-shared>\""
        "\"$<TARGET_FILE_DIR:Obhasa>\"")
endif()

# Copy resources
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    "${CMAKE_SOURCE_DIR}/Resources/" "$<TARGET_FILE_DIR:Obhasa>/Resources")
add_dependencies(Obhasa copy_assets)