cmake_minimum_required(VERSION 3.31)
project(HertraFramework
        DESCRIPTION ""
        VERSION 1.0
        LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 20)

## Configure project source and headers
file(GLOB_RECURSE PROJECT_SRC ${PROJECT_SOURCE_DIR}/src/*)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.cpp ${PROJECT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_precompile_headers(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/precompile.hpp)

## Find and include libraries
find_package(Vulkan REQUIRED)
find_package(glfw3  REQUIRED)
find_package(glm    REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  Vulkan::Vulkan glfw glm::glm
)

## Add platform-specific definitions for Wayland
if(UNIX AND NOT APPLE)
  target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
  target_include_directories(${PROJECT_NAME} PRIVATE ${WAYLAND_CLIENT_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${WAYLAND_CLIENT_LIBRARIES})
endif()

#№ Compile shaders
find_program(GLSLC glslc)
if(GLSLC)
  set(SHADER_SOURCE_DIR ${PROJECT_SOURCE_DIR}/shaders)
  set(SHADER_BINARY_DIR ${PROJECT_BINARY_DIR}/shaders)
  file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

  # Vertex shader
  add_custom_command(
    OUTPUT ${SHADER_BINARY_DIR}/vert.spv
    COMMAND ${GLSLC} -fshader-stage=vertex ${SHADER_SOURCE_DIR}/vert.glsl -o ${SHADER_BINARY_DIR}/vert.spv
    DEPENDS ${SHADER_SOURCE_DIR}/vert.glsl
    COMMENT "Compiling vertex shader"
  )

  # Fragment shader
  add_custom_command(
    OUTPUT ${SHADER_BINARY_DIR}/frag.spv
    COMMAND ${GLSLC} -fshader-stage=fragment ${SHADER_SOURCE_DIR}/frag.glsl -o ${SHADER_BINARY_DIR}/frag.spv
    DEPENDS ${SHADER_SOURCE_DIR}/frag.glsl
    COMMENT "Compiling fragment shader"
  )

  add_custom_target(Shaders DEPENDS
    ${SHADER_BINARY_DIR}/vert.spv
    ${SHADER_BINARY_DIR}/frag.spv
  )
  add_dependencies(${PROJECT_NAME} Shaders)

  # Copy shaders to build directory root for easy access
  add_custom_command(TARGET Shaders POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADER_BINARY_DIR} ${PROJECT_BINARY_DIR}/shaders
  )
else()
  message(WARNING "glslc not found - shaders will not be compiled automatically")
endif()
