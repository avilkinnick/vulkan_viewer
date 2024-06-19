cmake_minimum_required(VERSION 3.5)
project(vulkan_viewer LANGUAGES C)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(vulkan_viewer src/main.c
    include/Application.h
    include/base.h

    src/Application.c
    src/base.c
)

target_include_directories(vulkan_viewer PRIVATE include)
target_link_libraries(vulkan_viewer PRIVATE Vulkan::Vulkan SDL2::SDL2 SDL2::SDL2main)

include(GNUInstallDirs)
install(TARGETS vulkan_viewer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
