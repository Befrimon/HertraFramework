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

target_include_directories(${PROJECT_NAME} PRIVATE
  ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  Vulkan::Vulkan glfw
)

if(UNIX AND NOT APPLE)
  target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
  target_include_directories(${PROJECT_NAME} PRIVATE ${WAYLAND_CLIENT_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${WAYLAND_CLIENT_LIBRARIES})
endif()
