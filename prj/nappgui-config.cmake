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
                set(${optName} ${opt})
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
set(NAPPGUI_LIBRARIES "nappgui::inet;nappgui::osapp;nappgui::gui;nappgui::osgui;nappgui::draw2d;nappgui::geom2d;nappgui::core;nappgui::osbs;nappgui::sewer")

# Welcome and options
message(STATUS "Found NAppGUI at '${NAPPGUI_ROOT_PATH}'")
message(STATUS "NAppGUI build options")
message(STATUS "---------------------")
read_options()
message(STATUS "---------------------")
include("${NAPPGUI_ROOT_PATH}/prj/NAppProject.cmake")

# Link with Windows
if (WIN32)
    set(NAPPGUI_LIBRARIES "${NAPPGUI_LIBRARIES};ws2_32;wininet;gdiplus;shlwapi;comctl32;uxtheme")
    if (WEB_SUPPORT)
        if (${CMAKE_SIZEOF_VOID_P} STREQUAL 4)
            set(NAPPGUI_LIBRARIES "${NAPPGUI_LIBRARIES};${NAPPGUI_ROOT_PATH}/prj/depend/web/win/x86/WebView2LoaderStatic.lib;version")
        elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
            set(NAPPGUI_LIBRARIES "${NAPPGUI_LIBRARIES};${NAPPGUI_ROOT_PATH}/prj/depend/web/win/x64/WebView2LoaderStatic.lib;version")
        endif()
    endif()
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(CMAKE_TOOLKIT ${TOOLKIT})
    # Link with GTK3
    if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
        set(NAPPGUI_LIBRARIES "${NAPPGUI_LIBRARIES};${GTK3_LIBRARIES}")
    endif()

    # Link with WEBKIT
    if (WEB_SUPPORT)
        nap_find_webview_linux(WEBVIEW_FOUND WEBVIEW_HEADERS WEBVIEW_LIBS)
        if (WEBVIEW_FOUND)
            set(NAPPGUI_LIBRARIES "${NAPPGUI_LIBRARIES};${WEBVIEW_LIBS}")
        else()
            message(FATAL_ERROR "Required WEBKITGTK not found on this machine")
        endif()
    endif()

endif()

# Link with Cocoa
if (DEPLOYMENT_TARGET_OSX)
    if (NOT CMAKE_OSX_SYSROOT)
        message(FATAL_ERROR "CMAKE_OSX_SYSROOT is not set")
    endif()

    set(COCOA_LIB ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/Cocoa.framework)
    if (DEPLOYMENT_TARGET_OSX VERSION_GREATER 11.9999)
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
