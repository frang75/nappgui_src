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

    endif()

    set(WEB_SUPPORT "YES" CACHE INTERNAL "")
endfunction()
