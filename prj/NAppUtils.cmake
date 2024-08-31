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
        message(STATUS "* ${optName}: ${opt}")
        if (NAPPGUI_INSTALL_OPTIONS)
            if(EXISTS "${NAPPGUI_INSTALL_OPTIONS}")
                file(APPEND "${NAPPGUI_INSTALL_OPTIONS}" "${optName}:${opt}\n")
            endif()
        endif()
    endif()
endfunction()

#------------------------------------------------------------------------------

function(nap_find_webview_linux _found _headers _libs)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKITGTK QUIET webkit2gtk-4.1)
    if (WEBKITGTK_FOUND)
        set(${_found} "YES" PARENT_SCOPE)
        set(${_headers} "${WEBKITGTK_INCLUDE_DIRS}" PARENT_SCOPE)
        set(${_libs} "${WEBKITGTK_LIBRARIES}" PARENT_SCOPE)
        return()
    endif()

    pkg_check_modules(WEBKITGTK QUIET webkit2gtk-4.0)
    if (WEBKITGTK_FOUND)
        set(${_found} "YES" PARENT_SCOPE)
        set(${_headers} "${WEBKITGTK_INCLUDE_DIRS}" PARENT_SCOPE)
        set(${_libs} "${WEBKITGTK_LIBRARIES}" PARENT_SCOPE)
        return()
    endif()

    pkg_check_modules(WEBKITGTK QUIET webkit2gtk-3.0)
    if (WEBKITGTK_FOUND)
        set(${_found} "YES" PARENT_SCOPE)
        set(${_headers} "${WEBKITGTK_INCLUDE_DIRS}" PARENT_SCOPE)
        set(${_libs} "${WEBKITGTK_LIBRARIES}" PARENT_SCOPE)
        return()
    endif()

    set(${_found} "NO" PARENT_SCOPE)
    set(${_headers} "NOT-FOUND" PARENT_SCOPE)
    set(${_libs} "NOT-FOUND" PARENT_SCOPE)

endfunction()

#------------------------------------------------------------------------------

function(nap_check_webview_support)
    # Web support disabled by user
    if (NOT NAPPGUI_WEB)
        set(WEB_SUPPORT "NO" CACHE INTERNAL "")
        return()
    endif()

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
        nap_find_webview_linux(WEBVIEW_FOUND WEBVIEW_HEADERS WEBVIEW_LIBS)
        if (NOT WEBVIEW_FOUND)
            set(WEB_SUPPORT "NO" CACHE INTERNAL "")
            return()
        endif()

    endif()

    set(WEB_SUPPORT "YES" CACHE INTERNAL "")
endfunction()
