cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME "DemoApp")
set(APP_NAME "DemoApp")

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${PROJECT_NAME})

find_package(glm REQUIRED)

find_package(Qt5 REQUIRED COMPONENTS Quick)
find_package(GPUEngine REQUIRED geGL)

set(DEFAULT_RESOURCES_PATH "${CMAKE_CURRENT_LIST_DIR}/resources")
set(${APP_NAME}_RESOURCES "${DEFAULT_RESOURCES_PATH}" CACHE PATH "Relative or absolute path to Application resources.")


set(CORE_FILES
    src/core/main.cpp
    src/core/QuickRendererBase.cpp src/core/QuickRendererBase.h
    src/core/GERendererBase.cpp src/core/GERendererBase.h
    src/core/Renderer.cpp src/core/Renderer.h
    src/core/MouseEventHandler.cpp src/core/MouseEventHandler.h
    src/core/SceneBuilder.cpp src/core/SceneBuilder.h
)

set(GRAPHICS_FILES
    src/graphics/VisualizationTechnique.h
    src/graphics/RaytracingTechnique.h src/graphics/RaytracingTechnique.cpp
    src/graphics/Scene.h
    src/graphics/Light.h
)

SET(GRAPHICS_SHAPES_FILES
    src/graphics/Shapes/Shape.h 
    src/graphics/Shapes/AABB.h
    src/graphics/Shapes/Cylinder.h
    src/graphics/Shapes/Sphere.h
)

set(UTIL_FILES
    src/util/ShaderReader.h src/util/ShaderReader.cpp
)


add_executable(${APP_NAME} ${UTIL_FILES} ${CORE_FILES} ${GRAPHICS_FILES} )
target_link_libraries(${APP_NAME} Qt5::Quick geGL geUtil)

target_include_directories(${APP_NAME} PUBLIC "src/")

set_target_properties(${APP_NAME} PROPERTIES COMPILE_DEFINITIONS "APP_RESOURCES=\"${${APP_NAME}_RESOURCES}\"")
set_property(TARGET ${APP_NAME} PROPERTY FOLDER "${examples_folder_name}")
