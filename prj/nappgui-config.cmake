#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------

macro(read_definition def)
    set(DEFINITION "")
    set(defFile "${CMAKE_CURRENT_LIST_DIR}/${def}")
    if (EXISTS "${defFile}")
        file(READ "${CMAKE_CURRENT_LIST_DIR}/${def}" DEFINITION)
        string(REPLACE "\r\n" "" DEFINITION "${DEFINITION}")
        string(REPLACE "\n" "" DEFINITION "${DEFINITION}")
        if (DEFINITION)
            set(${def} ${DEFINITION})
        endif()
    endif()
endmacro()

#------------------------------------------------------------------------------

# Config file for use find_package(NAppGUI)
include("${CMAKE_CURRENT_LIST_DIR}/nappgui-targets.cmake")

set(NAPPGUI_IS_PACKAGE True)
if (NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE INTERNAL "Configuration Types" FORCE)
endif()

get_filename_component(NAPPGUI_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
set(NAPPGUI_INCLUDE_PATH "${NAPPGUI_ROOT_PATH}/inc")
set(NAPPGUI_NRC "${NAPPGUI_ROOT_PATH}/bin/nrc${CMAKE_EXECUTABLE_SUFFIX}")
set(NAPPGUI_LIBRARIES "nappgui::inet;nappgui::osapp;nappgui::gui;nappgui::osgui;nappgui::draw2d;nappgui::geom2d;nappgui::core;nappgui::osbs;nappgui::sewer")

# Project definitions
read_definition(CMAKE_TOOLKIT)
read_definition(CMAKE_OSX_DEPLOYMENT_TARGET)
read_definition(NAPPGUI_SHARED)
read_definition(NAPPGUI_VERSION)

# Link with GTK3
if (CMAKE_TOOLKIT)
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
            set(NAPPGUI_LIBRARIES "${NAPPGUI_LIBRARIES};${GTK3_LIBRARIES}")
        endif()
    endif()
endif()

# Link with Cocoa
if (CMAKE_OSX_DEPLOYMENT_TARGET)
    if (NOT CMAKE_OSX_SYSROOT)
        message(FATAL_ERROR "CMAKE_OSX_SYSROOT is not set")
    endif()

    set(COCOA_LIB ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/Cocoa.framework)
    if (CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER 11.9999)
        set(COCOA_LIB ${COCOA_LIB};${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/UniformTypeIdentifiers.framework)
    endif()
    set(NAPPGUI_LIBRARIES "${NAPPGUI_LIBRARIES};${COCOA_LIB}")
endif()

# Target definitions
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/NAppGUITargetsDefines.txt" NAppGUIDefines)
foreach(define ${NAppGUIDefines})
    if (define)
        add_definitions("-D${define}")
    endif()
endforeach()

# Messsage
if (NAPPGUI_SHARED)
    set(LINK_MODE "dynamic")
else()
    set(LINK_MODE "static")
endif()

message("Found NAppGUI at '${NAPPGUI_ROOT_PATH}'")
message("   Version: ${NAPPGUI_VERSION}")
message("   Linker: ${LINK_MODE}")
