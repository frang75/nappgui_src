#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

set(NAP_TARGET_PUBLIC_HEADER_EXTENSION "*.h;*.hxx;*.hpp;*.def")
set(NAP_TARGET_HEADER_EXTENSION "${NAP_TARGET_PUBLIC_HEADER_EXTENSION};*.inl;*.ixx;*.ipp")
set(NAP_TARGET_SRC_EXTENSION "${NAP_TARGET_HEADER_EXTENSION};*.c;*.cpp")
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(NAP_TARGET_SRC_EXTENSION "${NAP_TARGET_SRC_EXTENSION};*.m")
endif()

# https://cmake.org/cmake/help/latest/policy/CMP0068.html
# RPATH settings on macOS do not affect install_name.
# CMake 3.9 and newer remove any effect the following settings may have on the
# install_name of a target on macOS
if(${CMAKE_VERSION} VERSION_GREATER "3.8.999")
    cmake_policy(SET CMP0068 NEW)
endif()

if (NOT NAPPGUI_ROOT_PATH)
    message(FATAL_ERROR "NAPPGUI_ROOT_PATH is not set.")
endif()

# Defines required by NAppGUI-based targets after installation
set(NAPPGUI_INSTALL_DEFINES "${CMAKE_BINARY_DIR}/NAppGUITargetsDefines.txt")
if(EXISTS "${NAPPGUI_INSTALL_DEFINES}")
    file(REMOVE "${NAPPGUI_INSTALL_DEFINES}")
endif()
file(WRITE "${NAPPGUI_INSTALL_DEFINES}" "")

#------------------------------------------------------------------------------

# 90, 99, 11, 17, 23
function(nap_target_c_standard targetName std)

    if (${std} STREQUAL "90")
        # Ok!

    elseif (${std} STREQUAL "99")
        # Ok!

    elseif (${std} STREQUAL "11")
        # Ok!

    # New in version 3.21.
    elseif (${std} STREQUAL "17")
        if(${CMAKE_VERSION} VERSION_GREATER "3.20.999")
            # Ok!
        else()
            set(std "11")
        endif()

    # New in version 3.21.
    elseif (${std} STREQUAL "23")
        if(${CMAKE_VERSION} VERSION_GREATER "3.20.999")
            # Ok!
        else()
            set(std "11")
        endif()

    else()
        message(FATAL_ERROR "Unknown C standard")

    endif()

    # Language standard support in CMake 3.1
    if(${CMAKE_VERSION} VERSION_GREATER "3.0.999")
        set_property(TARGET ${targetName} PROPERTY C_STANDARD ${std})
    endif()

endfunction()

#------------------------------------------------------------------------------

# 98, 11, 14, 17, 20, 23, 26
function(nap_target_cxx_standard targetName std)

    if (${std} STREQUAL "98")
        # Ok!

    elseif (${std} STREQUAL "11")
        # Ok!

    elseif (${std} STREQUAL "14")
        # Ok!

    # New in version 3.8.
    elseif (${std} STREQUAL "17")
        if(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
            # Ok!
        else()
            set(std "14")
        endif()

    # New in version 3.12.
    elseif (${std} STREQUAL "20")
        if(${CMAKE_VERSION} VERSION_GREATER "3.11.999")
            # Ok!
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
            set(std "17")
        else()
            set(std "14")
        endif()

    # New in version 3.20.
    elseif(${std} STREQUAL "23")
        if(${CMAKE_VERSION} VERSION_GREATER "3.19.999")
            # Ok!
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.11.999")
            set(std "20")
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
            set(std "17")
        else()
            set(std "14")
        endif()

    # New in version 3.25.
    elseif(${std} STREQUAL "26")
        if(${CMAKE_VERSION} VERSION_GREATER "3.24.999")
            # Ok!
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.19.999")
            set(std "23")
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.11.999")
            set(std "20")
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
            set(std "17")
        else()
            set(std "14")
        endif()

    else()
        message(FATAL_ERROR "Unknown C++ standard")

    endif()

    # Language standard support in CMake 3.1
    if(${CMAKE_VERSION} VERSION_GREATER "3.0.999")
        set_property(TARGET ${targetName} PROPERTY CXX_STANDARD ${std})
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_target_rpath targetName isMacOsBundle rpath)

    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(RUNPATH "\${ORIGIN}")

        foreach(path ${rpath})
            set (RUNPATH "${RUNPATH}:${path}")
        endforeach(path )

        # Will disable the CMake automatic setting of the RPATH
        # RPaths included in target will be the current directory (ORIGIN)
        # and the provided in 'rpath' parameter
        set_property(TARGET ${targetName} PROPERTY SKIP_BUILD_RPATH FALSE)
        set_property(TARGET ${targetName} PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)
        set_property(TARGET ${targetName} PROPERTY INSTALL_RPATH "${RUNPATH}")

    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        # otool -L libdraw2d.dylib
        # @rpath/libgeom2d.dylib (compatibility version 0.0.0, current version 0.0.0)
        # Force to use paths relative to @rpath in dylibs and execs
        set_property(TARGET ${targetName} PROPERTY MACOSX_RPATH TRUE)

        if (isMacOsBundle)
            set(RUNPATH "@executable_path/../../..")
        else()
            set(RUNPATH "@executable_path/.")
        endif()

        set_property(TARGET ${targetName} PROPERTY SKIP_BUILD_RPATH FALSE)
        set_property(TARGET ${targetName} PROPERTY BUILD_RPATH ${RUNPATH})
        set_property(TARGET ${targetName} PROPERTY INSTALL_RPATH ${RUNPATH})

        # Delete Build RPATH manually (only if bundle have dynamic lib dependencies)
        # if (isMacOsBundle)
        #     add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -delete_rpath "${CMAKE_BINARY_DIR}/$<CONFIG>/bin" $<TARGET_FILE:${targetName}>)
        # endif()

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_get_subdirectories dir _ret)
    set(dirList "")

    file(GLOB children RELATIVE ${dir} ${dir}/[a-zA-z_]*)

    foreach(child ${children})
        if(IS_DIRECTORY ${dir}/${child})
            list(APPEND dirList ${child})
        endif()
    endforeach()

    set(${_ret} ${dirList} PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------

function(nap_is_source_subdir subDirName _ret)

    string(TOLOWER ${subDirName} subDirLower)
    if (${subDirLower} STREQUAL win)
        if (WIN32)
            set(${_ret} TRUE PARENT_SCOPE)
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL unix)
        if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin"
            OR ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            set(${_ret} TRUE PARENT_SCOPE)
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL osx)
        if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
            set(${_ret} TRUE PARENT_SCOPE)
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL linux)
        if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            set(${_ret} TRUE PARENT_SCOPE)
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL gtk)
        if (CMAKE_TOOLKIT)
            if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
                set(${_ret} TRUE PARENT_SCOPE)
            else()
                set(${_ret} FALSE PARENT_SCOPE)
            endif()
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL res)
        set(${_ret} FALSE PARENT_SCOPE)
    else ()
        set(${_ret} TRUE PARENT_SCOPE)
    endif ()

endfunction()

#------------------------------------------------------------------------------

function(nap_add_source_subdir targetName subDir)

    if ("${${targetName}_SRCSUBDIRS}" STREQUAL "")
        set(${targetName}_SRCSUBDIRS "${subDir}" CACHE INTERNAL "")
    else()
        list (FIND ${targetName}_SRCSUBDIRS ${subDir} index)
        if (${index} EQUAL -1)
            set(${targetName}_SRCSUBDIRS "${${targetName}_SRCSUBDIRS};${subDir}" CACHE INTERNAL "")
        endif ()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_add_source_file targetName file)

    if ("${${targetName}_SRCFILES}" STREQUAL "")
        set(${targetName}_SRCFILES "${file}" CACHE INTERNAL "")
    else()
        list (FIND ${targetName}_SRCFILES ${file} index)
        if (${index} EQUAL -1)
            set(${targetName}_SRCFILES "${${targetName}_SRCFILES};${file}" CACHE INTERNAL "")
        else()
            message(FATAL_ERROR "Duplicated source file '${file}'")
        endif()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_add_public_header targetName file extLower publicHeaders)

    if (publicHeaders)
        list (FIND NAP_TARGET_PUBLIC_HEADER_EXTENSION "*${extLower}" index)
        if (${index} GREATER -1)
            if ("${${targetName}_PUBLICHEADERS}" STREQUAL "")
                set(${targetName}_PUBLICHEADERS "${file}" CACHE INTERNAL "")
            else()
                list (FIND ${targetName}_PUBLICHEADERS ${file} index)
                if (${index} EQUAL -1)
                    set(${targetName}_PUBLICHEADERS "${${targetName}_PUBLICHEADERS};${file}" CACHE INTERNAL "")
                else()
                    message(FATAL_ERROR "Duplicated public header file '${file}'")
                endif()
            endif()
        endif()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_source_files targetName dir group publicHeaders)

    file(GLOB children RELATIVE ${dir} ${dir}/[a-zA-z_]*)

    foreach(child ${children})
        if (IS_DIRECTORY ${dir}/${child})
            nap_is_source_subdir(${child} isSource)
            if (${isSource})
                nap_source_files(${targetName} "${dir}/${child}" "${group}/${child}" FALSE)
            endif()
        else()
            get_filename_component(ext ${child} EXT)
            string(TOLOWER ${ext} extLower)

            # VisualStudio 2005 treat all .def files as module definitions
            # Even if you mark then as headers
            # .def files will not be shown as source files in VS2005
            if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 8 2005" AND ${extLower} STREQUAL ".def")
                set(index -1)
            else()
                list (FIND NAP_TARGET_SRC_EXTENSION "*${extLower}" index)
            endif()

            if (${index} GREATER -1)
                string(REPLACE "/" "\\" groupname ${group})
                source_group(${groupname} FILES ${dir}/${child})

                # Force header files 'Build errors with CMake >= 3.21.2'
                # https://gitlab.kitware.com/cmake/cmake/-/merge_requests/5926
                list (FIND NAP_TARGET_HEADER_EXTENSION "*${extLower}" index)
                if (${index} GREATER -1)
                    set_source_files_properties(${dir}/${child} PROPERTIES HEADER_FILE_ONLY ON)
                    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
                        set_source_files_properties(${dir}/${child} PROPERTIES XCODE_EXPLICIT_FILE_TYPE sourcecode.c.h)
                    endif()
                endif()

                nap_add_source_subdir(${targetName} ${dir})
                nap_add_source_file(${targetName} ${dir}/${child})
                nap_add_public_header(${targetName} ${dir}/${child} ${extLower} ${publicHeaders})

            endif()

        endif()

    endforeach()

endfunction()

#------------------------------------------------------------------------------

function(nap_resource_pattern dir _ret)
    set(list_res "")

    foreach (item ${RES_EXTENSION})
        list(APPEND list_res ${dir}/${item})
    endforeach()

    set(${_ret} ${list_res} PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------

function(nap_resource_packs targetName targetType nrcMode dir _resFiles _resIncludeDir)
    # All resource files in package
    set(resFiles "")
    set(resPath ${dir}/res)

    if (EXISTS ${resPath})
        # Process Win32 .rc files
        if (targetType STREQUAL WIN_DESKTOP)

            # VS2005 does not support .ico with 256 res
            if(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION LESS 1400)
                file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/res.rc "APPLICATION_ICON ICON \"${resPath}/logo48.ico\"\n")
                set(globalRes ${resPath}/logo48.ico)
            else()
                file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/res.rc "APPLICATION_ICON ICON \"${resPath}/logo256.ico\"\n")
                set(globalRes ${resPath}/logo256.ico)
            endif()

            # Reference the manifest file (required by MinGW)
            if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
                # https://geekthis.net/post/visual-styles-in-win32-api-c-gcc-mingw/
                set(MANIFEST_FILE "${NAPPGUI_ROOT_PATH}/prj/templates/Application.manifest")
                file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/res.rc "1 24 \"${MANIFEST_FILE}\"\n")
            endif()

            set(globalRes ${globalRes} ${CMAKE_CURRENT_BINARY_DIR}/res.rc)
        endif()

        if (EXISTS ${resPath}/license.txt)
            list(APPEND globalRes ${resPath}/license.txt)
        endif()

        if (EXISTS ${resPath}/pack.txt)
            list(APPEND globalRes ${resPath}/pack.txt)
        endif()

        source_group(res FILES ${globalRes})
        list(APPEND resFiles ${globalRes})

    endif()

    # Target Resources
    if (${nrcMode} STREQUAL "NRC_EMBEDDED" OR ${nrcMode} STREQUAL "NRC_PACKED")

        if (NOT NAPPGUI_NRC)
            message(FATAL_ERROR "NAPPGUI_NRC is not set")
        endif()

        nap_get_subdirectories(${resPath} resPackDirs)

        # Resource destiny directory
        set(DEST_RESDIR ${CMAKE_CURRENT_BINARY_DIR}/resgen)
        set(CMAKE_OUTPUT ${DEST_RESDIR}/NRCLog.txt)
        if (NOT EXISTS ${DEST_RESDIR})
            file(MAKE_DIRECTORY ${DEST_RESDIR})
        endif()

        foreach(resPack ${resPackDirs})
            # Add resources to IDE
            set(resPackPath ${resPath}/${resPack})
            nap_resource_pattern(${resPackPath} resGlob)
            file(GLOB resPackPathFiles ${resGlob})
            source_group("res\\${resPack}" FILES ${resPackPathFiles})
            list(APPEND resFiles ${resPackPathFiles})

            # Add localized resources to IDE
            nap_get_subdirectories(${resPath}/${resPack} resLocalDirs)
            foreach(resLocalDir ${resLocalDirs})
                set(resLocalPath ${resPath}/${resPack}/${resLocalDir})
                nap_resource_pattern(${resLocalPath} resLocalGlob)
                file(GLOB resLocalPathFiles ${resLocalGlob})
                source_group("res\\${resPack}\\${resLocalDir}" FILES ${resLocalPathFiles})
                list(APPEND resFiles ${resLocalPathFiles})
            endforeach()

            if (${nrcMode} STREQUAL "NRC_EMBEDDED")
                set(NRC_OPTION "-dc")
            # '*.res' package will be copied in executable location
            elseif (${nrcMode} STREQUAL "NRC_PACKED")
                set(NRC_OPTION "-dp")
            else()
                message (FATAL_ERROR "Unknown nrc mode")
            endif()

            file(TO_NATIVE_PATH ${resPackPath} RESPACK_NATIVE)
            file(TO_NATIVE_PATH ${DEST_RESDIR}/${resPack}.c RESDEST_NATIVE)
            execute_process(COMMAND "${NAPPGUI_NRC}" "${NRC_OPTION}" "${RESPACK_NATIVE}" "${RESDEST_NATIVE}" RESULT_VARIABLE nrcRes OUTPUT_VARIABLE nrcOut ERROR_VARIABLE nrcErr)
            file(WRITE ${CMAKE_OUTPUT} ${nrcOut})
            file(APPEND ${CMAKE_OUTPUT} ${nrcErr})

            if (${nrcRes} EQUAL "0")
                message(STATUS "- [OK] ${resPack}: Resource pack recompiled.")
            elseif (${nrcRes} EQUAL "1")
                message(STATUS "- [OK] ${resPack}: Resource pack is up-to-date.")
            elseif (${nrcRes} EQUAL "-1")
                message("- [RES] ${resPack}: warnings (See ${CMAKE_OUTPUT})")
            else()
                message("- [RES] ${resPack}: errors (${nrcRes}) (See ${CMAKE_OUTPUT})")
                message("- [RES] OutStd: ${nrcOut}")
                message("- [RES] OutErr: ${nrcErr}")
            endif()

            list(APPEND resCompiled ${DEST_RESDIR}/${resPack}.c)
            list(APPEND resCompiled ${DEST_RESDIR}/${resPack}.h)
            source_group("res\\${resPack}\\gen" FILES ${resCompiled})

            list(APPEND resFiles ${resCompiled})

            set(${_resIncludeDir} ${DEST_RESDIR} PARENT_SCOPE)

            endforeach()

    endif()

    set(${_resFiles} ${resFiles} PARENT_SCOPE)

endfunction()

#------------------------------------------------------------------------------

function(nap_install_resource_packs targetName targetType sourceDir nrcMode)
    set (resourcePath ${sourceDir}/res)

    # Apple Bundle always have a resource dir
    if (targetType STREQUAL APPLE_BUNDLE)
        set(resourceDestPath "../resources")
        add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${targetName}>/${resourceDestPath})
        add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${targetName}>/${resourceDestPath}/en.lproj)

        # Bundle icon
        if (EXISTS ${resourcePath}/logo.icns)
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${resourcePath}/logo.icns $<TARGET_FILE_DIR:${targetName}>/${resourceDestPath})
        else()
            message(WARNING "logo.icns doesn't exists in '${resourcePath}'")
        endif()

    # Linux needs the app icon near the executable
    elseif (targetType STREQUAL LINUX_DESKTOP)

        if (EXISTS ${resourcePath}/logo48.ico)
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${resourcePath}/logo48.ico $<TARGET_FILE_DIR:${targetName}>/${targetName}.ico)
            install(FILES $<TARGET_FILE_DIR:${targetName}>/${targetName}.ico DESTINATION "bin")
        else()
            message(WARNING "logo48.ico doesn't exists in '${resourcePath}'")
        endif()

    endif()

    if (${nrcMode} STREQUAL "NRC_PACKED")
        set(resPath ${sourceDir}/res)
        set(destResDir ${CMAKE_CURRENT_BINARY_DIR}/resgen)

        # Create 'res' directory for packed resources
        # In the same location as executable
        if (WIN32)
            set(resRelative "res")
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${targetName}>/${resRelative})

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
            # For macOS bundles, resource dir is created in 'macOSBundle'
            set(resRelative "../resources")

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            set(resRelative "res")
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${targetName}>/${resRelative})

            else()
            message(FATAL_ERROR "Unknown system")

        endif()

        nap_get_subdirectories(${resPath} resPackDirs)

        # Copy all resource packs
        foreach(resSubDir ${resPackDirs})
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${destResDir}/${resSubDir}.res $<TARGET_FILE_DIR:${targetName}>/${resRelative})
            install(FILES ${destResDir}/${resSubDir}.res DESTINATION "bin/res")
            endforeach()

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_target_relpath targetSrcDir _ret)
    string(REPLACE "${CMAKE_SOURCE_DIR}/" "" relPath ${targetSrcDir})
    set(${_ret} ${relPath} PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------

function(nap_add_dependency targetName depend)

    if ("${${targetName}_LINKDEPENDS}" STREQUAL "")
        set(${targetName}_LINKDEPENDS "${depend}" CACHE INTERNAL "")
    else()
        set(${targetName}_LINKDEPENDS "${${targetName}_LINKDEPENDS};${depend}" CACHE INTERNAL "")
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_direct_dependencies targetName _ret)

    if (NAPPGUI_CACHE_DEPENDS_${targetName})
        set(${_ret} ${NAPPGUI_CACHE_DEPENDS_${targetName}} PARENT_SCOPE)
    else()
        set(${_ret} "" PARENT_SCOPE)
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_target_dependencies targetName dependList)

    foreach(depend ${dependList})

        # Dependency is a Target of this solution
        if (TARGET ${depend})
            get_target_property(TARGET_TYPE ${depend} TYPE)
            if (${TARGET_TYPE} STREQUAL "STATIC_LIBRARY" OR ${TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
                nap_add_dependency(${targetName} ${depend})
                nap_direct_dependencies(${depend} childDependList)
            else()
                message(FATAL_ERROR "- ${targetName}: Unknown dependency type '${depend}-${TARGET_TYPE}'")
            endif ()
        else()
            message(FATAL_ERROR "- ${targetName}: Unknown dependency '${depend}'")
        endif()

        if (childDependList)
            nap_target_dependencies(${targetName} "${childDependList}")
        endif()

    endforeach()

endfunction()

#------------------------------------------------------------------------------

function(nap_exists_dependency targetName libName _ret)

    set(${_ret} "NO" PARENT_SCOPE)

    if (${targetName} STREQUAL "${libName}")
        set(${_ret} "YES" PARENT_SCOPE)
        return()
    endif()

    foreach(depend ${${targetName}_LINKDEPENDS})

        if (${depend} STREQUAL "${libName}")
            set(${_ret} "YES" PARENT_SCOPE)
            return()
        endif()

    endforeach()

endfunction()

#------------------------------------------------------------------------------

function(nap_web_libs _weblibs)
    if (WEB_SUPPORT)
        if (WIN32)
            if (${CMAKE_SIZEOF_VOID_P} STREQUAL 4)
                set(WEBVIEW_LIBPATH "${NAPPGUI_ROOT_PATH}/prj/depend/web/win/x86/WebView2LoaderStatic.lib")
            elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
                set(WEBVIEW_LIBPATH "${NAPPGUI_ROOT_PATH}/prj/depend/web/win/x64/WebView2LoaderStatic.lib")
            endif()

            # 'version' is required by WebView2Loader
            set(${_weblibs} "${WEBVIEW_LIBPATH};version" PARENT_SCOPE)

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
            set(WEBVIEW_FRAMEWORK ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/WebKit.framework)
            set(${_weblibs} "${WEBVIEW_FRAMEWORK}" PARENT_SCOPE)

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            nap_find_webview_linux(WEBVIEW_FOUND WEBVIEW_HEADERS WEBVIEW_LIBS)
            if (WEBVIEW_FOUND)
                set(${_weblibs} "${WEBVIEW_LIBS}" PARENT_SCOPE)
            endif()
        endif()
    endif()
endfunction()

#------------------------------------------------------------------------------

function(nap_link_inet targetName)

    if(NAPPGUI_IS_PACKAGE)
        target_link_libraries(${targetName} nappgui::inet)
    endif()

    if(WIN32)
        target_link_libraries(${targetName} wininet)

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        find_package(CURL)
        if (${CURL_FOUND})
            target_link_libraries(${targetName} ${CURL_LIBRARY})
        else()
            message(ERROR "- libCURL is required. Try 'sudo apt-get install libcurl4-openssl-dev'")
        endif()

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_link_opengl targetName)

    if(NAPPGUI_IS_PACKAGE)
        target_link_libraries(${targetName} nappgui::ogl3d)
    endif()

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        find_package(OpenGL REQUIRED EGL)
        target_link_libraries(${targetName} ${OPENGL_LIBRARY})

        if (OPENGL_egl_LIBRARY)
            target_link_libraries(${targetName} ${OPENGL_egl_LIBRARY})

        else()
            # CMake older than 3.10 not detect EGL
            if (NOT EGL_INCLUDE_DIR OR NOT EGL_LIBRARY)
                message("-- Direct search of EGL")
                find_path(EGL_INCLUDE_DIR "egl.h" PATHS "/usr/include/*" "/usr/local/include/*")
                find_file(EGL_LIBRARY "libEGL.so" PATHS "/usr/lib/*" "/usr/local/lib/*")

                if (EGL_INCLUDE_DIR)
                    message(STATUS "-- EGL_INCLUDE_DIR: ${EGL_INCLUDE_DIR}")
                else()
                    message(FATAL_ERROR "-- EGL_INCLUDE_DIR: NOT FOUND")
                endif()

                if (EGL_LIBRARY)
                    message(STATUS "-- EGL_LIBRARY: ${EGL_LIBRARY}")
                else()
                    message(FATAL_ERROR "-- EGL_LIBRARY: NOT FOUND")
                endif()

            endif()

            target_include_directories(${targetName} PRIVATE ${EGL_INCLUDE_DIR})
            target_link_libraries(${targetName} ${EGL_LIBRARY})

        endif()

    else()
        find_package(OpenGL REQUIRED)
        target_link_libraries(${targetName} ${OPENGL_LIBRARY})

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_link_with_libraries targetName targetType firstLevelDepends)

    #
    # Link with direct target dependencies
    #
    set(${targetName}_LINKDEPENDS "" CACHE INTERNAL "")
    nap_target_dependencies(${targetName} "${firstLevelDepends}")

    if (${targetName}_LINKDEPENDS)
        foreach(depend ${${targetName}_LINKDEPENDS})
            target_link_libraries(${targetName} ${depend})
            get_target_property(DEPEND_TARGET_TYPE ${depend} TYPE)
            if (${DEPEND_TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
                nap_target_relpath(${${depend}_SRCPATH} dependPath)
                get_filename_component(dependPathUpper ${dependPath} NAME)
                string(TOUPPER ${dependPathUpper} dependPathUpper)
                set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS NAPPGUI_${dependPathUpper}_IMPORT_DLL)
            endif()
        endforeach()

    endif()

    #
    # Link with precompiled NAppGUI
    #
    if (NAPPGUI_IS_PACKAGE)
        if (${targetType} STREQUAL "DESKTOP_APP" OR ${targetType} STREQUAL "DYNAMIC_LIB")
            target_link_libraries(${targetName} "nappgui::osapp;nappgui::gui;nappgui::osgui;nappgui::draw2d;nappgui::geom2d;nappgui::core;nappgui::osbs;nappgui::sewer")
        elseif (${targetType} STREQUAL "COMMAND_APP")
            target_link_libraries(${targetName} "nappgui::draw2d;nappgui::geom2d;nappgui::core;nappgui::osbs;nappgui::sewer")
        endif()
    endif()

    #
    # Link with system libraries
    #
    if (NOT NAPPGUI_IS_PACKAGE)
        nap_exists_dependency(${targetName} "osbs" _depend_osbs)
        nap_exists_dependency(${targetName} "draw2d" _depend_draw2d)
        nap_exists_dependency(${targetName} "osgui" _depend_osgui)
    else()
        set(_depend_osbs True)
        set(_depend_draw2d True)
        if (${targetType} STREQUAL "DESKTOP_APP" OR ${targetType} STREQUAL "DYNAMIC_LIB")
            set(_depend_osgui True)
        else()
            set(_depend_osgui False)
        endif()
    endif()

    if(WIN32)
        # Target should link with WinSockets
        if (_depend_osbs)
            target_link_libraries(${targetName} ws2_32)
        endif()

        # Target should link with gdiplus
        if (_depend_draw2d)
            target_link_libraries(${targetName} gdiplus shlwapi)
        endif()

        # Target should link with comctl32
        if (_depend_osgui)
            target_link_libraries(${targetName} comctl32 uxtheme)
        endif()

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        # Target should link with math always
        target_link_libraries(${targetName} "m")

        # Target should link with pthread and dl
        if (_depend_osbs)
            find_package(Threads)
            if (Threads_FOUND)
                target_link_libraries(${targetName} ${CMAKE_THREAD_LIBS_INIT})
            else()
                message(ERROR "- PThread library not found")
            endif()

            target_link_libraries(${targetName} ${CMAKE_DL_LIBS})
        endif()

        if (_depend_draw2d)
            # Graphics toolkit
            if (NOT CMAKE_TOOLKIT)
                message(FATAL_ERROR "CMAKE_TOOLKIT is not set")
            endif()

            # The target has to link with GTK+3
            if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
                # Use the package PkgConfig to detect GTK+ headers/library files
                find_package(PkgConfig REQUIRED)
                pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
                target_link_libraries(${targetName} ${GTK3_LIBRARIES})
            endif()
        endif()

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        if (NOT ${targetType} STREQUAL "STATIC_LIB")
            target_link_libraries(${targetName} ${COCOA_LIB})
        endif()

    endif()

    # Target should link with WebView
    nap_web_libs(_weblibs)
    if (_weblibs)
        target_link_libraries(${targetName} ${_weblibs})
    endif()

    # Target should link with network libraries
    # Apps that use NAppGUI package must call 'nap_link_inet()'
    if (NOT NAPPGUI_IS_PACKAGE)
        nap_exists_dependency(${targetName} "inet" _depends)
        if (_depends)
            nap_link_inet(${targetName})
        endif()
    endif()

    # Target should link with OpenGL
    # Apps that use NAppGUI package must call 'nap_link_opengl()'
    if (NOT NAPPGUI_IS_PACKAGE)
        nap_exists_dependency(${targetName} "ogl3d" _depends)
        if (_depends)
            nap_link_opengl(${targetName})
        endif()
    endif()

    # In GCC the g++ linker must be used
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set_target_properties(${targetName} PROPERTIES LINKER_LANGUAGE CXX)
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_target targetName targetType dependList nrcMode)

    # Get source files
    set(${targetName}_SRCFILES "" CACHE INTERNAL "")
    set(${targetName}_SRCSUBDIRS "" CACHE INTERNAL "")
    set(${targetName}_PUBLICHEADERS "" CACHE INTERNAL "")
    nap_source_files(${targetName} ${CMAKE_CURRENT_SOURCE_DIR} "src" TRUE)
    set(srcFiles ${${targetName}_SRCFILES})
    set(srcSubDirs ${${targetName}_SRCSUBDIRS})
    set(publicHeaders ${${targetName}_PUBLICHEADERS})

    # Get resources
    set(${targetName}_SRCPATH "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")
    nap_resource_packs(${targetName} ${targetType} ${nrcMode} ${CMAKE_CURRENT_SOURCE_DIR} resFiles resIncludeDir)
    nap_target_relpath(${${targetName}_SRCPATH} targetPath)

    # Generate target (library, executable)
    if (targetType STREQUAL STATIC_LIB)
        message(STATUS "- [OK] ${targetName}: Static library")
        add_library(${targetName} STATIC ${srcFiles} ${resFiles})

        # Clang, GNU, Intel, MSVC
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${targetName} PUBLIC "-fPIC")
        endif()

        # Install the public headers
        if (publicHeaders)
            set_target_properties(${targetName} PROPERTIES PUBLIC_HEADER "${publicHeaders}")
        endif()

    elseif (targetType STREQUAL DYNAMIC_LIB)
        message(STATUS "- [OK] ${targetName}: Dynamic library")
        add_library(${targetName} SHARED ${srcFiles} ${resFiles})

        get_filename_component(targetPathUpper ${targetPath} NAME)
        string(TOUPPER ${targetPathUpper} targetPathUpper)
        set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS NAPPGUI_${targetPathUpper}_EXPORT_DLL)

        # Append import for use in NAppGUI-based future targets
        file(APPEND "${NAPPGUI_INSTALL_DEFINES}" "NAPPGUI_${targetPathUpper}_IMPORT_DLL\n")

        # Clang, GNU, Intel, MSVC
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${targetName} PUBLIC "-fPIC;-fvisibility=hidden")
            set_target_properties(${targetName} PROPERTIES LINK_FLAGS "-fPIC")
        endif()

        # Install the public headers
        if (publicHeaders)
            set_target_properties(${targetName} PROPERTIES PUBLIC_HEADER "${publicHeaders}")
        endif()

    elseif (targetType STREQUAL WIN_DESKTOP)
        message(STATUS "- [OK] ${targetName}: Desktop application")
        add_executable(${targetName} WIN32 ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL WIN_CONSOLE)
        message(STATUS "- [OK] ${targetName}: Command-line application")
        add_executable(${targetName} ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL APPLE_BUNDLE)
        message(STATUS "- [OK] ${targetName}: Desktop application")
        add_executable(${targetName} MACOSX_BUNDLE ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL APPLE_CONSOLE)
        message(STATUS "- [OK] ${targetName}: Command-line application")
        add_executable(${targetName} ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL LINUX_DESKTOP)
        message(STATUS "- [OK] ${targetName}: Desktop application")
        add_executable(${targetName} ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL LINUX_CONSOLE)
        message(STATUS "- [OK] ${targetName}: Command-line application")
        add_executable(${targetName} ${srcFiles} ${resFiles})

    else()
        message(FATAL_ERROR "Unknown target type")

    endif()

    # Output directories for generated binaries
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} configUpper)
        set_property(TARGET ${targetName} APPEND PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/lib")
        set_property(TARGET ${targetName} APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/bin")
        set_property(TARGET ${targetName} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/bin")
    endforeach()

    # Install binaries and headers
    get_filename_component(targetPathSingle ${targetPath} NAME)
    install(TARGETS ${targetName} EXPORT nappgui-targets
                LIBRARY DESTINATION "bin" PERMISSIONS ${INSTALL_PERM}
                RUNTIME DESTINATION "bin" PERMISSIONS ${INSTALL_PERM}
                ARCHIVE DESTINATION "lib" PERMISSIONS ${INSTALL_PERM}
                BUNDLE DESTINATION "bin"
                PUBLIC_HEADER DESTINATION "inc/${targetPathSingle}")

    # Install the .pdb files
    if (targetType STREQUAL STATIC_LIB)
        install(FILES "$<TARGET_FILE_DIR:${targetName}>/${targetName}.pdb" DESTINATION "lib" PERMISSIONS ${INSTALL_PERM} OPTIONAL)
    else()
        install(FILES "$<TARGET_FILE_DIR:${targetName}>/${targetName}.pdb" DESTINATION "bin" PERMISSIONS ${INSTALL_PERM} OPTIONAL)
    endif()

    # Install the .exp files
    # install(FILES "$<TARGET_LINKER_FILE_DIR:${targetName}>/${targetName}.exp" CONFIGURATIONS "${config}" DESTINATION "lib/${config}" PERMISSIONS ${INSTALL_PERM} OPTIONAL)

    # Install resource packs
    nap_install_resource_packs(${targetName} ${targetType} ${CMAKE_CURRENT_SOURCE_DIR} ${nrcMode})

    # Target Definitions
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} configUpper)
        set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:${config}>:CMAKE_${configUpper}>)
    endforeach()

    if (WIN32)
        # Visual Studio 2005/2008 doesn't have <stdint.h>
        if(MSVC_VERSION EQUAL 1500 OR MSVC_VERSION LESS 1500)
            target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${NAPPGUI_ROOT_PATH}/prj/depend>)
        endif()

        # Platform toolset macro
        #set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS VS_PLATFORM=${VS_TOOLSET_NUMBER})

        # Force the name of the pdb (vc110.pdb in VS2012)
        set_target_properties(${targetName} PROPERTIES COMPILE_PDB_NAME ${targetName})
    endif()

    # WebView support
    if (${targetName} STREQUAL "osgui")
        if (WEB_SUPPORT)
            if (WIN32)
                target_include_directories("osgui" PRIVATE $<BUILD_INTERFACE:${NAPPGUI_ROOT_PATH}/prj/depend/web/win>)
                target_compile_definitions("osgui" PUBLIC "-DNAPPGUI_WEB_SUPPORT")

            elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
                target_compile_definitions("osgui" PUBLIC "-DNAPPGUI_WEB_SUPPORT")

            elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
                nap_find_webview_linux(WEBVIEW_FOUND WEBVIEW_HEADERS WEBVIEW_LIBS)
                if (WEBVIEW_FOUND)
                    foreach(dir ${WEBVIEW_HEADERS})
                        target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${dir}>)
                    endforeach()

                    target_compile_definitions("osgui" PUBLIC "-DNAPPGUI_WEB_SUPPORT")
                endif()

            endif()
        endif()
    endif()

    # GTK Include directories
    if (CMAKE_TOOLKIT)
        if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
            if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/gtk)
                # Use the package PkgConfig to detect GTK+ headers/library files
                find_package(PkgConfig REQUIRED)
                pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
                foreach(dir ${GTK3_INCLUDE_DIRS})
                    target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${dir}>)
                endforeach()
                target_compile_definitions(${targetName} PUBLIC "-D__GTK3_TOOLKIT__")
            endif()
        endif()
    endif()

    # Target global include directories
    if (NOT NAPPGUI_IS_PACKAGE)
        target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${NAPPGUI_ROOT_PATH}/src>)
    else()
        target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${NAPPGUI_INCLUDE_PATH}>)
    endif()

    target_include_directories(${targetName} PRIVATE $<INSTALL_INTERFACE:inc>)

    # Include dir for target generated resources
    if (resIncludeDir)
        target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${resIncludeDir}>)
    endif()

    # Target dependency for compile order
    if (dependList)
        foreach(depend ${dependList})
            add_dependencies(${targetName} ${depend})
        endforeach()
    endif()

    # Target default C/C++ standards
    nap_target_c_standard(${targetName} "90")
    nap_target_cxx_standard(${targetName} "98")

endfunction()

#------------------------------------------------------------------------------

function(nap_library libName dependList buildShared nrcMode)

    if (buildShared)
        nap_target(${libName} DYNAMIC_LIB "${dependList}" ${nrcMode})
        nap_link_with_libraries(${libName} DYNAMIC_LIB "${dependList}")
        nap_target_rpath(${libName} NO "")

    else()
        nap_target(${libName} STATIC_LIB "${dependList}" ${nrcMode})

        # # In Linux, static libs must link with other libs
        # if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        #     nap_link_with_libraries(${libName} STATIC_LIB "${dependList}")
        # endif()
    endif()

    set(NAPPGUI_CACHE_DEPENDS_${libName} "${dependList}" CACHE INTERNAL "")

endfunction()

#------------------------------------------------------------------------------

function(nap_command_app appName dependList nrcMode)

    if (WIN32)
        nap_target("${appName}" WIN_CONSOLE "${dependList}" ${nrcMode})

        if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
            foreach(config ${CMAKE_CONFIGURATION_TYPES})
                string(TOUPPER ${config} configUpper)
                set_target_properties(${appName} PROPERTIES LINK_FLAGS_${configUpper} "/SUBSYSTEM:CONSOLE")
            endforeach()
        endif()

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        nap_target("${appName}" APPLE_CONSOLE "${dependList}" ${nrcMode})

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        nap_target("${appName}" LINUX_CONSOLE "${dependList}" ${nrcMode})

    else()
        message(ERROR "- ${appName} Unknown system")

    endif()

    nap_link_with_libraries(${appName} COMMAND_APP "${dependList}")
    nap_target_rpath(${appName} NO "")

endfunction()

#------------------------------------------------------------------------------

function(nap_desktop_app appName dependList nrcMode)

    if (WIN32)
        nap_target(${appName} WIN_DESKTOP "${dependList}" ${nrcMode})
        if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
            foreach(config ${CMAKE_CONFIGURATION_TYPES})
                string(TOUPPER ${config} configUpper)
                set_target_properties(${appName} PROPERTIES LINK_FLAGS_${configUpper} "/SUBSYSTEM:WINDOWS")
            endforeach()
        endif()
        set(macOSBundle NO)

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        nap_target(${appName} APPLE_BUNDLE "${dependList}" ${nrcMode})

        # Info.plist configure
        # Proyect provides its own Info.plist?
        if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
            set_target_properties(${appName} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
        # Use default template
        else()
            set_target_properties(${appName} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${NAPPGUI_ROOT_PATH}/prj/templates/Info.plist)
        endif()

        # Overwrite some properties
        # bundleProp(${bundleName} "NSHumanReadableCopyright" "${CURRENT_YEAR} ${PACK_VENDOR}")
        # bundleProp(${bundleName} "CFBundleVersion" "${PACK_VERSION}")

        set(macOSBundle YES)

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        nap_target("${appName}" LINUX_DESKTOP "${dependList}" ${nrcMode})
        set(macOSBundle NO)

    else()
        message("Unknown platform")

    endif()

    nap_link_with_libraries(${appName} DESKTOP_APP "${dependList}")
    nap_target_rpath(${appName} ${macOSBundle} "")

endfunction()
