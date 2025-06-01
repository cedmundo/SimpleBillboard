cmake_minimum_required(VERSION 3.20)
project(SimpleBillboard VERSION "0.0.1" LANGUAGES C)

# General configuration / variables
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Vendor dependencies
# add_subdirectory(vendor)

# System packages
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

# Assets
add_subdirectory(assets)

# Main executbale
set(MAIN_EXEC SimpleBillboard${CMAKE_BUILD_TYPE})
add_executable(${MAIN_EXEC})
add_dependencies(${MAIN_EXEC} grid_shader billboard_shader)
target_sources(${MAIN_EXEC} PRIVATE xmath.c shader.c grid.c camera.c billboard.c simulation.c main.c)
target_link_libraries(${MAIN_EXEC} PRIVATE SDL3::SDL3)
target_compile_options(${MAIN_EXEC} PRIVATE -g -Wall)
