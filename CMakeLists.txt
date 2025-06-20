#------------------------------------------------------------------------------
# NAppGUI CMake build script
# Copyright (C) 2018 Francisco García Collado
# MIT License (See LICENSE.txt)
#------------------------------------------------------------------------------

# https://gitlab.kitware.com/cmake/cmake/-/issues/25196
if(${CMAKE_VERSION} VERSION_GREATER "3.99.999")
    cmake_minimum_required(VERSION 3.10)
elseif(${CMAKE_VERSION} VERSION_GREATER "3.26.999")
    cmake_minimum_required(VERSION 3.5)
else()
    cmake_minimum_required(VERSION 3.0)
endif()

project(NAppGUI)

set(NAPPGUI_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Globals
set(NAPPGUI_SHARED False CACHE BOOL "Build NAppGUI shared libraries.")
set(NAPPGUI_DEMO True CACHE BOOL "Build NAppGUI demo examples.")
set(NAPPGUI_WEB True CACHE BOOL "Build NAppGUI WebView support.")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(INSTALL_PERM "OWNER_READ;OWNER_WRITE;OWNER_EXECUTE;GROUP_READ;GROUP_EXECUTE;WORLD_READ;WORLD_EXECUTE")

# Welcome
string(TIMESTAMP CURRENT_YEAR %Y)
include(${NAPPGUI_ROOT_PATH}/prj/NAppVersion.cmake)
include(${NAPPGUI_ROOT_PATH}/prj/NAppCompilers.cmake)
include(${NAPPGUI_ROOT_PATH}/prj/NAppUtils.cmake)
message (STATUS "---------------------------------------------")
message (STATUS "NAppGUI Cross-Platform SDK")
message (STATUS "2015-${CURRENT_YEAR} Francisco Garcia Collado")
message (STATUS "MIT License")
message (STATUS "---------------------------------------------")
nap_build_opt_init()
nap_config_compiler()
nap_check_webview_support()

nap_build_opt("VERSION" "${NAPPGUI_MAJOR}.${NAPPGUI_MINOR}.${NAPPGUI_PATCH}.${NAPPGUI_BUILD}")
nap_build_opt("GENERATOR" "${CMAKE_GENERATOR}")
nap_build_opt("BUILD_SHARED" "${NAPPGUI_SHARED}")
nap_build_opt("COMPILER" "${CMAKE_CXX_COMPILER_ID}")
nap_build_opt("COMPILER_VERSION" "${CMAKE_CXX_COMPILER_VERSION}")
nap_build_opt("WEB_SUPPORT" "${WEB_SUPPORT}")
nap_build_opt("BUILD_TYPE" "${CMAKE_BUILD_TYPE}")
nap_build_opt("BUILD_ARCH" "${CMAKE_ARCHITECTURE}")
nap_build_opt("HOST_ARCH" "${CMAKE_HOST_ARCHITECTURE}")
nap_build_opt("PACKAGE_ID" "${CMAKE_PACKAGE_ID}")
message (STATUS "---------------------------------------------")

# Tools required to generate NAppGUI projets ('nrc')
# They need to be compiled before NAppGUI configure/generate
include(${NAPPGUI_ROOT_PATH}/prj/NAppGenerateTools.cmake)
nap_generate_tools()

# Generate targets
include(${NAPPGUI_ROOT_PATH}/prj/NAppTarget.cmake)
include(${NAPPGUI_ROOT_PATH}/CMakeTargets.cmake)

foreach (target ${ALL_TARGETS})
    add_subdirectory(${target})
endforeach()

# Install global files
install(FILES "${NAPPGUI_ROOT_PATH}/src/nappgui.h" DESTINATION "inc" PERMISSIONS ${INSTALL_PERM})
install(FILES "${NAPPGUI_NRC}" DESTINATION "bin" PERMISSIONS ${INSTALL_PERM})
install(DIRECTORY "${NAPPGUI_ROOT_PATH}/prj" DESTINATION ".")
install(EXPORT nappgui-targets DESTINATION "cmake" NAMESPACE nappgui::)
install(FILES "${NAPPGUI_ROOT_PATH}/prj/nappgui-config.cmake" DESTINATION "cmake")
install(FILES "${NAPPGUI_INSTALL_DEFINES}" DESTINATION "cmake")
install(FILES "${NAPPGUI_INSTALL_OPTIONS}" DESTINATION "cmake")
