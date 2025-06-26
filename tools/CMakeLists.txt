if(${CMAKE_VERSION} VERSION_GREATER "3.99.999")
    cmake_minimum_required(VERSION 3.10)
elseif(${CMAKE_VERSION} VERSION_GREATER "3.26.999")
    cmake_minimum_required(VERSION 3.5)
else()
    cmake_minimum_required(VERSION 3.0)
endif()

project(NAppGUITools)

if (NOT NAPPGUI_ROOT_PATH)
    message(FATAL_ERROR "NAPPGUI_ROOT_PATH is not set.")
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE INTERNAL "Configuration Types" FORCE)
set(CMAKE_BUILD_TYPE "Debug" CACHE INTERNAL "Build type" FORCE)

include(${NAPPGUI_ROOT_PATH}/prj/NAppCompilers.cmake)
nap_config_compiler()

include(${NAPPGUI_ROOT_PATH}/prj/NAppTarget.cmake)
set(NAPPGUI_SHARED False)
add_subdirectory(../src/sewer sewer_bin)
add_subdirectory(../src/osbs osbs_bin)
add_subdirectory(../src/core core_bin)
add_subdirectory(nrc)
