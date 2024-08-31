#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------

macro(read_options)
    set(optsFile "${CMAKE_CURRENT_LIST_DIR}/NAppGUIOptions.txt")
    if (EXISTS "${optsFile}")
        file(STRINGS "${optsFile}" ALL_OPTIONS)
        foreach(option ${ALL_OPTIONS})
            if (option)
                string(REPLACE ":" ";" KEY_VALUE "${option}")
                list(GET KEY_VALUE 0 optName)
                list(GET KEY_VALUE 1 opt)
                set(${optName} ${opt} CACHE INTERNAL "")
                message(STATUS "* ${optName}: ${opt}")
            endif()
        endforeach()
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
set(NAPPGUI_LIBRARIES "nappgui::osapp;nappgui::gui;nappgui::osgui;nappgui::draw2d;nappgui::geom2d;nappgui::core;nappgui::osbs;nappgui::sewer")

# Welcome and options
message(STATUS "Found NAppGUI at '${NAPPGUI_ROOT_PATH}'")
message(STATUS "NAppGUI build options")
message(STATUS "---------------------")
read_options()
message(STATUS "---------------------")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    if (NOT DEPLOYMENT_TARGET_OSX)
        message(FATAL_ERROR "DEPLOYMENT_TARGET_OSX is not set")
    endif()

    if (NOT CMAKE_OSX_SYSROOT)
        message(FATAL_ERROR "CMAKE_OSX_SYSROOT is not set")
    endif()

    # Set COCOA_LIB paths
    set(COCOA_LIB ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/Cocoa.framework)
    if (DEPLOYMENT_TARGET_OSX VERSION_GREATER 11.9999)
        set(COCOA_LIB ${COCOA_LIB};${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/UniformTypeIdentifiers.framework)
    endif()

endif()

# Target definitions
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/NAppGUITargetsDefines.txt" NAppGUIDefines)
foreach(define ${NAppGUIDefines})
    if (define)
        add_definitions("-D${define}")
    endif()
endforeach()
