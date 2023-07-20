CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

project(platform-info-binding VERSION 8.1.0 LANGUAGES C)

# define all CMAKE_INSTALL_XXXX vars
include(GNUInstallDirs)

add_compile_definitions(PLATFORM_INFO_DIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}/platform-info")

include(${CMAKE_CURRENT_SOURCE_DIR}/conf.d/cmake/config.cmake)
