cmake_minimum_required(VERSION 3.5)
project(vulkan_viewer LANGUAGES C)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(vulkan_viewer main.c)

target_link_libraries(vulkan_viewer PRIVATE SDL2::SDL2 SDL2::SDL2main Vulkan::Vulkan)

include(GNUInstallDirs)
install(TARGETS vulkan_viewer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
