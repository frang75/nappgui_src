# CMake NAppGUI main build script
# This file is part of NAppGUI-SDK project
# See README.txt and LICENSE.txt

# 'inc' path
get_filename_component(CMAKE_INC_PATH ${CMAKE_CURRENT_LIST_DIR} PATH)
set(CMAKE_INC_PATH ${CMAKE_INC_PATH}/inc)

# 'prj' path
get_filename_component(CMAKE_PRJ_PATH ${CMAKE_CURRENT_LIST_DIR} PATH)
set(CMAKE_PRJ_PATH ${CMAKE_PRJ_PATH}/prj)

# 'lib' path
get_filename_component(CMAKE_LIB_PATH ${CMAKE_CURRENT_LIST_DIR} PATH)
set(CMAKE_LIB_PATH ${CMAKE_LIB_PATH}/lib)

# 'bin' path
get_filename_component(CMAKE_BIN_PATH ${CMAKE_CURRENT_LIST_DIR} PATH)
set(CMAKE_BIN_PATH ${CMAKE_BIN_PATH}/bin)

# 'src' path
get_filename_component(CMAKE_SRC_PATH ${CMAKE_CURRENT_LIST_DIR} PATH)
set(CMAKE_SRC_PATH ${CMAKE_SRC_PATH}/src)

set(DIRECTX_SDK_FIND FALSE CACHE INTERNAL "")

# Scripts
include(${CMAKE_PRJ_PATH}/CMakeSDKVersion.cmake)
include(${CMAKE_PRJ_PATH}/CMakeGlobals.cmake)
include(${CMAKE_PRJ_PATH}/CMakeVSWarnings.cmake)
include(${CMAKE_PRJ_PATH}/CMakeXCWarnings.cmake)
include(${CMAKE_PRJ_PATH}/CMakeVSCheckVersion.cmake)
include(${CMAKE_PRJ_PATH}/CMakeXCCheckVersion.cmake)
include(${CMAKE_PRJ_PATH}/CMakeGCCheckVersion.cmake)
include(${CMAKE_PRJ_PATH}/CMakeCompilers.cmake)
include(${CMAKE_PRJ_PATH}/CMakeFunctions.cmake)

# nrc 'NAppGUI Resource Compiler' command
if (WIN32)
    set(CMAKE_NRC ${CMAKE_PRJ_PATH}/script/win/${NAPPGUI_ARCH}/nrc.exe)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(CMAKE_NRC ${CMAKE_PRJ_PATH}/script/osx/${NAPPGUI_HOST}/nrc)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(CMAKE_NRC ${CMAKE_PRJ_PATH}/script/linux/${NAPPGUI_HOST}/nrc)
else()
    message(FATAL_ERROR "CMAKE_NRC Not Set")
endif()
