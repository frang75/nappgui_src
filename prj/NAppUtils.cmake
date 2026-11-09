#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------

# Build options shared in NAppGUI installation
macro(nap_build_opt_init)
    set(NAPPGUI_INSTALL_OPTIONS "${CMAKE_BINARY_DIR}/NAppGUIOptions.txt")
    if(EXISTS "${NAPPGUI_INSTALL_OPTIONS}")
        file(REMOVE "${NAPPGUI_INSTALL_OPTIONS}")
    endif()
    file(WRITE "${NAPPGUI_INSTALL_OPTIONS}" "")
endmacro()

#------------------------------------------------------------------------------

function(nap_build_opt optName opt)
    if(NOT NAPPGUI_IS_PACKAGE)
        message(STATUS "* ${optName}: ${opt} ${ARGV2}")
        if (NAPPGUI_INSTALL_OPTIONS)
            if(EXISTS "${NAPPGUI_INSTALL_OPTIONS}")
                file(APPEND "${NAPPGUI_INSTALL_OPTIONS}" "${optName}:${opt}\n")
            endif()
        endif()
    endif()
endfunction()

#------------------------------------------------------------------------------

function(nap_check_inet_support)
    set(INET_SUPPORT "YES" CACHE INTERNAL "")
    set(INET_SUPPORT_INFO "" CACHE INTERNAL "")

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        find_library(NAPPGUI_CURL_RUNTIME NAMES "libcurl.so.4" "libcurl-gnutls.so.4")
        if (NOT NAPPGUI_CURL_RUNTIME)
            set(INET_SUPPORT_INFO "(libcurl not found on this machine. See https://www.nappgui.com/en/inet/inet.html)" CACHE INTERNAL "")
        endif()
    endif()
endfunction()

#------------------------------------------------------------------------------

function(nap_check_webview_support)
    set(WEB_SUPPORT_INFO "" CACHE INTERNAL "")

    if (WIN32)
        if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
            # Visual Studio 2013 and lower doesn't support the WebView2 compilation
            if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "19.0.0")
                set(WEB_SUPPORT "NO" CACHE INTERNAL "")
                return()
            endif()

        else()
            # At the moment, WebView is disabled for MinGW
            set(WEB_SUPPORT "NO" CACHE INTERNAL "")
            return()
        endif()

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        # Only available from 10.10 Yosemite
        if (NOT CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER 10.9.9999)
            set(WEB_SUPPORT "NO" CACHE INTERNAL "")
            return()
        endif()

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        find_library(NAPPGUI_WEBKIT_RUNTIME NAMES "libwebkit2gtk-4.1.so.0" "libwebkit2gtk-4.0.so.37" "libwebkit2gtk-3.0.so.25")
        if (NOT NAPPGUI_WEBKIT_RUNTIME)
            set(WEB_SUPPORT_INFO "(WebKitGTK not found on this machine. See https://www.nappgui.com/en/gui/webview.html)" CACHE INTERNAL "")
        endif()

    endif()

    set(WEB_SUPPORT "YES" CACHE INTERNAL "")
endfunction()

#------------------------------------------------------------------------------

function(nap_find_opengl _found)

    set(${_found} "NO" PARENT_SCOPE)

    if (WIN32)
        set(${_found} "YES" PARENT_SCOPE)

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        find_package(OpenGL QUIET)
        if (OPENGL_FOUND)
            set(${_found} "YES" PARENT_SCOPE)
        endif()

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        find_package(OpenGL QUIET COMPONENTS EGL)
        if (NOT OPENGL_FOUND OR NOT OPENGL_GLU_FOUND)
            return()
        endif()

        if (NOT OPENGL_egl_LIBRARY)
            find_path(NAPPGUI_EGL_INCLUDE "EGL/egl.h")
            find_library(NAPPGUI_EGL_LIBRARY "EGL")
            if (NOT NAPPGUI_EGL_INCLUDE OR NOT NAPPGUI_EGL_LIBRARY)
                return()
            endif()
        endif()

        set(${_found} "YES" PARENT_SCOPE)

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_check_3d_support)
    nap_find_opengl(OPENGL_SUPPORT)
    if (OPENGL_SUPPORT)
        set(3D_SUPPORT "YES" CACHE INTERNAL "")
        set(3D_SUPPORT_INFO "" CACHE INTERNAL "")
    else()
        set(3D_SUPPORT "NO" CACHE INTERNAL "")
        set(3D_SUPPORT_INFO "(OpenGL not found. See https://www.nappgui.com/en/ogl3d/ogl3d.html)" CACHE INTERNAL "")
    endif()
endfunction()
