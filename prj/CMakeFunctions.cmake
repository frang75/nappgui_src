# CMake build functions script
# This file is part of NAppGUI-SDK project
# See README.txt and LICENSE.txt

#------------------------------------------------------------------------------

function(getSubDirectories dir _ret)
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

function(resourceGlobPatterns dir _ret)
    set(list_res "")

    foreach (item ${RES_EXTENSION})
        list(APPEND list_res ${dir}/${item})
    endforeach()

    set(${_ret} ${list_res} PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------

function(isSourceSubDir subDirName _ret)

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
    elseif (${subDirLower} STREQUAL gtk3)
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

function(appendSrcFile targetName file)

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

function(appendPublicHeader targetName file extLower publicHeaders)

    if (publicHeaders)
        list (FIND PUBLIC_HEADER_EXTENSION "*${extLower}" index)
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

function(appendSrcSubDir targetName subDir)

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

function(getRecursiveSourceFiles targetName dir group publicHeaders)

    file(GLOB children RELATIVE ${dir} ${dir}/[a-zA-z_]*)

    foreach(child ${children})
        if (IS_DIRECTORY ${dir}/${child})
            isSourceSubDir(${child} isSource)
            if (${isSource})
                getRecursiveSourceFiles(${targetName} "${dir}/${child}" "${group}/${child}" FALSE)
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
                list (FIND SRC_EXTENSION "*${extLower}" index)
            endif()

            if (${index} GREATER -1)
                string(REPLACE "/" "\\" groupname ${group})
                source_group(${groupname} FILES ${dir}/${child})

                # Force header files 'Build errors with CMake >= 3.21.2'
                # https://gitlab.kitware.com/cmake/cmake/-/merge_requests/5926
                list (FIND HEADER_EXTENSION "*${extLower}" index)
                if (${index} GREATER -1)
                    set_source_files_properties(${dir}/${child} PROPERTIES HEADER_FILE_ONLY ON)
                    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
                        set_source_files_properties(${dir}/${child} PROPERTIES XCODE_EXPLICIT_FILE_TYPE sourcecode.c.h)
                    endif()
                endif()

                appendSrcSubDir(${targetName} ${dir})
                appendSrcFile(${targetName} ${dir}/${child})
                appendPublicHeader(${targetName} ${dir}/${child} ${extLower} ${publicHeaders})

            endif()

        endif()

    endforeach()

endfunction()

#------------------------------------------------------------------------------

function(getResourceFiles targetName targetType dir _ret _include_dir)
    # All resource files in package
    set(res_files "")
    set(resPath ${dir}/res)

    if (EXISTS ${resPath})
		# Process Win32 .rc files
		if (targetType STREQUAL WIN_DESKTOP)
			# VS2005 does not support .ico with 256 res
			if(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION LESS 1400)
                file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/res.rc "APPLICATION_ICON ICON \"res\\\\logo48.ico\"\n")
                set(globalRes ${CMAKE_CURRENT_BINARY_DIR}/res.rc ${resPath}/logo48.ico)
			else()
                file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/res.rc "APPLICATION_ICON ICON \"res\\\\logo256.ico\"\n")
                set(globalRes ${CMAKE_CURRENT_BINARY_DIR}/res.rc ${resPath}/logo256.ico)
			endif()
		endif()

        if (EXISTS ${resPath}/license.txt)
            list(APPEND globalRes ${resPath}/license.txt)
        endif()

        if (EXISTS ${resPath}/pack.txt)
            list(APPEND globalRes ${resPath}/pack.txt)
        endif()

        source_group(res FILES ${globalRes})
        list(APPEND res_files ${globalRes})

    endif()

    # Target Resources
    set(nrc_mode ${NAPPGUI_CACHE_TARGET_NRC_MODE_${targetName}})
    if (NOT ${nrc_mode} STREQUAL "NRC_NONE")
        getSubDirectories(${resPath} resPackDirs)

        # Clean the resource destiny directory
        set(DEST_RESDIR ${CMAKE_CURRENT_BINARY_DIR}/resgen)
        set(CMAKE_OUTPUT ${DEST_RESDIR}/NRCLog.txt)
        file(REMOVE_RECURSE ${DEST_RESDIR})
        file(MAKE_DIRECTORY ${DEST_RESDIR})

	    foreach(resPack ${resPackDirs})
            # Add resources to IDE
            set(resPackPath ${resPath}/${resPack})
            resourceGlobPatterns(${resPackPath} resGlob)
            file(GLOB resPackPathFiles ${resGlob})
            source_group("res\\${resPack}" FILES ${resPackPathFiles})
            list(APPEND res_files ${resPackPathFiles})

            # Add localized resources to IDE
            getSubDirectories(${resPath}/${resPack} resLocalDirs)
            foreach(resLocalDir ${resLocalDirs})
                set(resLocalPath ${resPath}/${resPack}/${resLocalDir})
                resourceGlobPatterns(${resLocalPath} resLocalGlob)
                file(GLOB resLocalPathFiles ${resLocalGlob})
                source_group("res\\${resPack}\\${resLocalDir}" FILES ${resLocalPathFiles})
                list(APPEND res_files ${resLocalPathFiles})
            endforeach()

            if (${nrc_mode} STREQUAL "NRC_EMBEDDED")
                set(NRC_OPTION "-dc")
            # '*.res' package will be copied in executable location
            elseif (${nrc_mode} STREQUAL "NRC_PACKED")
                set(NRC_OPTION "-dp")
            else()
                message (FATAL_ERROR "Unknown nrc mode")
            endif()

			file(TO_NATIVE_PATH \"${CMAKE_NRC}\" CMAKE_NRC)
			file(TO_NATIVE_PATH ${resPackPath} RESPACK_NATIVE)
			file(TO_NATIVE_PATH ${DEST_RESDIR}/${resPack}.c RESDEST_NATIVE)
            execute_process(COMMAND "${CMAKE_NRC}" "${NRC_OPTION}" "${RESPACK_NATIVE}" "${RESDEST_NATIVE}" RESULT_VARIABLE nrcRes OUTPUT_VARIABLE nrcOut ERROR_VARIABLE nrcErr)
            file(APPEND ${CMAKE_OUTPUT} ${nrcOut})
            file(APPEND ${CMAKE_OUTPUT} ${nrcErr})
            if (NOT ${nrcRes} EQUAL "0")
                if (${nrcRes} EQUAL "1")
                    message("- nrc '${resPack}' warnings (See ${CMAKE_OUTPUT})")
                else()
                    message("- nrc '${resPack}' errors (${nrcRes}) (See ${CMAKE_OUTPUT})")
                    message("- ${nrcOut}")
                    message("- ${nrcErr}")
                endif()
            endif()

            list(APPEND resCompiled ${DEST_RESDIR}/${resPack}.c)
            list(APPEND resCompiled ${DEST_RESDIR}/${resPack}.h)
            source_group("res\\${resPack}\\gen" FILES ${resCompiled})

            list(APPEND res_files ${resCompiled})

            set(${_include_dir} ${DEST_RESDIR} PARENT_SCOPE)

	    endforeach()

    endif()

    set(${_ret} ${res_files} PARENT_SCOPE)

endfunction()

#------------------------------------------------------------------------------

function(installResourcePacks targetName targetType sourceDir)
    set (resourcePath ${sourceDir}/res)
    toolsetName(toolset)

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

            # Copy the icon into install directory
            foreach(config ${CMAKE_CONFIGURATION_TYPES})
                install(FILES $<TARGET_FILE_DIR:${targetName}>/${targetName}.ico CONFIGURATIONS "${config}" DESTINATION "bin/${toolset}/${config}")
            endforeach()

        else()
            message(WARNING "logo48.ico doesn't exists in '${resourcePath}'")
        endif()

    endif()

    set(nrc_mode ${NAPPGUI_CACHE_TARGET_NRC_MODE_${targetName}})
    if (nrc_mode)
    if (${nrc_mode} STREQUAL "NRC_PACKED")
        set(resPath ${sourceDir}/res)
        set(DEST_RESDIR ${CMAKE_CURRENT_BINARY_DIR}/resgen)

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

        getSubDirectories(${resPath} resPackDirs)

        # Create a 'res' directory in install bin dir (for 'PACKED' resource packages)
        foreach(config ${CMAKE_CONFIGURATION_TYPES})
            install(DIRECTORY CONFIGURATIONS "${config}" DESTINATION "bin/${toolset}/${config}/res")
        endforeach()

        # Copy all resource packs
	    foreach(resSubDir ${resPackDirs})
            add_custom_command(TARGET ${bundleName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DEST_RESDIR}/${resSubDir}.res $<TARGET_FILE_DIR:${targetName}>/${resRelative})

            # Copy the resource pack into install directory
            foreach(config ${CMAKE_CONFIGURATION_TYPES})
                install(FILES ${DEST_RESDIR}/${resSubDir}.res CONFIGURATIONS "${config}" DESTINATION "bin/${toolset}/${config}/res")
            endforeach()

	    endforeach()

    endif()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(getDirectDepends targetName _ret)

    if (NAPPGUI_CACHE_DEPENDS_${targetName})
        set(${_ret} ${NAPPGUI_CACHE_DEPENDS_${targetName}} PARENT_SCOPE)

	else()
        set(${_ret} "" PARENT_SCOPE)

    endif()

endfunction()

#------------------------------------------------------------------------------

function(appendIncludeDependency targetName depend)

    if ("${${targetName}_INCLUDEDEPENDS}" STREQUAL "")
        set(${targetName}_INCLUDEDEPENDS "${depend}" CACHE INTERNAL "")
    else()
        list (FIND ${targetName}_INCLUDEDEPENDS ${depend} index)
        if (${index} EQUAL -1)
            set(${targetName}_INCLUDEDEPENDS "${${targetName}_INCLUDEDEPENDS};${depend}" CACHE INTERNAL "")
        endif ()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(getRecursiveTargetIncludes targetName dependList)

	foreach(depend ${dependList})
		if (TARGET ${depend})
            appendIncludeDependency(${targetName} ${CMAKE_SRC_PATH}/${NAPPGUI_CACHE_LIBPATH_${depend}})
            getDirectDepends(${depend} childDependList)
        elseif (EXISTS ${CMAKE_INC_PATH}/${NAPPGUI_CACHE_LIBPATH_${depend}})
            appendIncludeDependency(${targetName} ${CMAKE_INC_PATH}/${NAPPGUI_CACHE_LIBPATH_${depend}})
            getDirectDepends(${depend} childDependList)

        else()
			# May exists binaries in 'lib' without include files
			set(childDependList "")

        endif()

        if (childDependList)
            getRecursiveTargetIncludes(${targetName} "${childDependList}")
        endif()

	endforeach()

endfunction()

#------------------------------------------------------------------------------

function(targetDependsIncludeDirectories targetName firstLevelDepends)

    set(${targetName}_INCLUDEDEPENDS "" CACHE INTERNAL "")
    getRecursiveTargetIncludes(${targetName} "${firstLevelDepends}")

    if (${targetName}_INCLUDEDEPENDS)
        target_include_directories(${targetName} PUBLIC "${${targetName}_INCLUDEDEPENDS}")
    endif()

endfunction()

#------------------------------------------------------------------------------

function(toolsetName _ret)

    if (NOT NAPPGUI_COMPILER_TOOLSET)
        message(FATAL_ERROR "NAPPGUI_COMPILER_TOOLSET Not Set")
    endif()

    if (NOT NAPPGUI_ARCH)
        message(FATAL_ERROR "NAPPGUI_ARCH Not Set")
    endif()

    set(${_ret} "${NAPPGUI_COMPILER_TOOLSET}_${NAPPGUI_ARCH}" PARENT_SCOPE)

endfunction()

#------------------------------------------------------------------------------

function(processTarget targetName targetType dependList)

    # Get Files
    set(${targetName}_SRCFILES "" CACHE INTERNAL "")
    set(${targetName}_SRCSUBDIRS "" CACHE INTERNAL "")
    set(${targetName}_PUBLICHEADERS "" CACHE INTERNAL "")
    getRecursiveSourceFiles(${targetName} ${CMAKE_CURRENT_SOURCE_DIR} "src" TRUE)
    set(srcFiles ${${targetName}_SRCFILES})
    set(srcSubDirs ${${targetName}_SRCSUBDIRS})
    set(publicHeaders ${${targetName}_PUBLICHEADERS})
    getResourceFiles(${targetName} ${targetType} ${CMAKE_CURRENT_SOURCE_DIR} resFiles resIncludeDir)

    # TARGET Build
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
	    set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS NAPPGUI_SHARED_LIB)

        # Clang, GNU, Intel, MSVC
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	        target_compile_options(${targetName} PUBLIC "-fPIC;-fvisibility=hidden")
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

    # IDE Properties for TARGET
    if (IDE_PROPERTIES)
        set_target_properties(${targetName} PROPERTIES ${IDE_PROPERTIES})
    endif()

    # Output directories
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} configUpper)
        set_property(TARGET ${targetName} APPEND PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/lib")
        set_property(TARGET ${targetName} APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/bin")
        set_property(TARGET ${targetName} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/bin")
    endforeach()

    # Install binaries and headers
    toolsetName(toolset)
    targetSourceDir(${targetName} targetSource)
    set(RTPERM "OWNER_READ;OWNER_WRITE;OWNER_EXECUTE;GROUP_READ;GROUP_EXECUTE;WORLD_READ;WORLD_EXECUTE")
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        install(TARGETS ${targetName} CONFIGURATIONS "${config}" ARCHIVE DESTINATION "lib/${toolset}/${config}" PERMISSIONS ${RTPERM} LIBRARY DESTINATION "bin/${toolset}/${config}" PERMISSIONS ${RTPERM} RUNTIME DESTINATION "bin/${toolset}/${config}" PERMISSIONS ${RTPERM} BUNDLE DESTINATION "bin/${toolset}/${config}" PUBLIC_HEADER DESTINATION "inc/${targetSource}")

        # Install the .pdb files
        if (targetType STREQUAL STATIC_LIB)
            install(FILES "$<TARGET_FILE_DIR:${targetName}>/${targetName}.pdb" CONFIGURATIONS "${config}" DESTINATION "lib/${toolset}/${config}" PERMISSIONS ${RTPERM} OPTIONAL)
        else()
            install(FILES "$<TARGET_FILE_DIR:${targetName}>/${targetName}.pdb" CONFIGURATIONS "${config}" DESTINATION "bin/${toolset}/${config}" PERMISSIONS ${RTPERM} OPTIONAL)
        endif()

        # Install the .exp files
        # install(FILES "$<TARGET_LINKER_FILE_DIR:${targetName}>/${targetName}.exp" CONFIGURATIONS "${config}" DESTINATION "lib/${toolset}/${config}" PERMISSIONS ${RTPERM} OPTIONAL)

    endforeach()

    # Resource packs
    installResourcePacks(${targetName} ${targetType} ${CMAKE_CURRENT_SOURCE_DIR})

    get_target_property(TARGET_TYPE ${targetName} TYPE)

    # Target Definitions
	set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:CMAKE_DEBUG>)
    set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Release>:CMAKE_RELEASE>)
    set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:ReleaseWithAssert>:CMAKE_RELEASEWITHASSERT>)

    if (WIN32)
        # Visual Studio 2005/2008 doesn't have <stdint.h>
        if(MSVC_VERSION EQUAL 1500 OR MSVC_VERSION LESS 1500)
            target_include_directories(${targetName} PUBLIC ${CMAKE_PRJ_PATH}/depend)
        endif()

        # Platform toolset macro
        set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS VS_PLATFORM=${VS_TOOLSET_NUMBER})

        # Disable linker '4099' "pdb" warnings
        # Disable linker '4098' mixed (static/dynamic) runtime library warnings
        set_target_properties(${targetName} PROPERTIES LINK_FLAGS "/ignore:4099 /ignore:4098")

    endif()

    # GTK Include directories
    if (CMAKE_TOOLKIT)
        if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
            if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/gtk3)
                # Use the package PkgConfig to detect GTK+ headers/library files
                find_package(PkgConfig REQUIRED)
                pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
                target_include_directories(${targetName} PUBLIC ${GTK3_INCLUDE_DIRS})
                set_target_properties(${targetName} PROPERTIES COMPILE_FLAGS "-D__GTK3_TOOLKIT__")
            endif()
        endif()
    endif()

    # TARGET local directory include
    target_include_directories(${targetName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    # Include dir for target generated resources
    if (resIncludeDir)
        target_include_directories(${targetName} PUBLIC ${resIncludeDir})
    endif()

    if (dependList)
        foreach(dependency ${dependList})
            get_filename_component(dependencyName ${dependency} NAME)
            if (TARGET ${dependencyName})
                add_dependencies(${targetName} ${dependencyName})
            endif()
        endforeach()

        # Recursive directory includes
        targetDependsIncludeDirectories(${targetName} "${dependList}")
    endif()

endfunction()

#------------------------------------------------------------------------------

function(targetRPath targetName isMacOsBundle rpath)

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

function(targetOptions targetName options)
    set(c_lang_standard "90")
    set(cxx_lang_standard "98")

    foreach(opt ${options})

        if (${opt} STREQUAL "C90")
            set(c_lang_standard "90")

        elseif(${opt} STREQUAL "C99")
            set(c_lang_standard "99")

        elseif(${opt} STREQUAL "C11")
            set(c_lang_standard "11")

        elseif(${opt} STREQUAL "C17")
            # New in version 3.21.
            if(${CMAKE_VERSION} VERSION_GREATER "3.20.999")
                set(c_lang_standard "17")
            else()
                set(c_lang_standard "11")
            endif()

        elseif(${opt} STREQUAL "C23")
            # New in version 3.21.
            if(${CMAKE_VERSION} VERSION_GREATER "3.20.999")
                set(c_lang_standard "23")
            else()
                set(c_lang_standard "11")
            endif()

        elseif(${opt} STREQUAL "C++98")
            set(cxx_lang_standard "98")

        elseif(${opt} STREQUAL "C++11")
            set(cxx_lang_standard "11")

        elseif(${opt} STREQUAL "C++14")
            set(cxx_lang_standard "14")

        elseif(${opt} STREQUAL "C++17")
            # New in version 3.8.
            if(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
                set(cxx_lang_standard "17")
            else()
                set(cxx_lang_standard "14")
            endif()

        elseif(${opt} STREQUAL "C++20")
            # New in version 3.12.
            if(${CMAKE_VERSION} VERSION_GREATER "3.11.999")
                set(cxx_lang_standard "20")
            elseif(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
                set(cxx_lang_standard "17")
            else()
                set(cxx_lang_standard "14")
            endif()

        elseif(${opt} STREQUAL "C++23")
            # New in version 3.20.
            if(${CMAKE_VERSION} VERSION_GREATER "3.19.999")
                set(cxx_lang_standard "23")
            elseif(${CMAKE_VERSION} VERSION_GREATER "3.11.999")
                set(cxx_lang_standard "20")
            elseif(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
                set(cxx_lang_standard "17")
            else()
                set(cxx_lang_standard "14")
            endif()

        endif()

    endforeach()

    # Language standard support in CMake 3.1
    if(${CMAKE_VERSION} VERSION_GREATER "3.0.999")
        set_property(TARGET ${targetName} PROPERTY C_STANDARD ${c_lang_standard})
        set_property(TARGET ${targetName} PROPERTY CXX_STANDARD ${cxx_lang_standard})
    endif()

endfunction()

#------------------------------------------------------------------------------

function(processStaticLib libName dependList)
    set(dependencies "")
    list(APPEND dependencies "${dependList}")
    processTarget("${libName}" "STATIC_LIB" "${dependencies}")

    # In Linux, static libs must link with other libs
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        targetLinkWithLibraries(${libName} "${dependencies}")
    endif()

    targetOptions(${libName} "${options}")

endfunction()

#------------------------------------------------------------------------------

function(processDynamicLib libName dependList)
    set(dependencies "")
    list(APPEND dependencies "${dependList}")
    processTarget("${libName}" "DYNAMIC_LIB" "${dependList}")
    targetLinkWithLibraries(${libName} "${dependencies}")
    targetOptions(${libName} "${options}")
    targetRPath(${libName} NO "")

endfunction()

#------------------------------------------------------------------------------

function(windowsBundle bundleName bundleType dependList)
    processTarget("${bundleName}" "${bundleType}" "${dependList}")
endfunction()

#------------------------------------------------------------------------------

function(bundleProp bundleName property value)
    # Fails in XCode 3.2, 4
    # /bin/bash -c "plutil -replace NSHumanReadableCopyright -string 2019\ NAppGUI .../Info.plist"
    # unrecognized option: -replace
    if(${XCODE_VERSION} VERSION_GREATER "5.99")
        add_custom_command(TARGET ${bundleName} POST_BUILD COMMAND /bin/bash -c \"plutil -replace ${property} -string "${value}" $<TARGET_FILE_DIR:${bundleName}>/../Info.plist\")
    endif()
endfunction()

#------------------------------------------------------------------------------

function(macOSBundle bundleName dependList)

    processTarget("${bundleName}" APPLE_BUNDLE "${dependList}")

    # Info.plist configure
    # Proyect provides its own Info.plist?
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
        set_target_properties(${bundleName} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
    # Use default template
    else()
        set_target_properties(${bundleName} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_PRJ_PATH}/templates/Info.plist)
    endif()

    # Overwrite some properties
    # bundleProp(${bundleName} "NSHumanReadableCopyright" "${CURRENT_YEAR} ${PACK_VENDOR}")
    # bundleProp(${bundleName} "CFBundleVersion" "${PACK_VERSION}")

endfunction()

#------------------------------------------------------------------------------

function(linuxBundle bundleName dependList)
    processTarget("${bundleName}" LINUX_DESKTOP "${dependList}")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set_target_properties(${bundleName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
endfunction()

#------------------------------------------------------------------------------

function(appendLinkDependency targetName depend)

    if ("${${targetName}_LINKDEPENDS}" STREQUAL "")
        set(${targetName}_LINKDEPENDS "${depend}" CACHE INTERNAL "")
    else()
        set(${targetName}_LINKDEPENDS "${${targetName}_LINKDEPENDS};${depend}" CACHE INTERNAL "")
    endif()

endfunction()

#------------------------------------------------------------------------------

function(existsPrecompiledStaticLib libName _ret)

    foreach(config ${CMAKE_CONFIGURATION_TYPES})
	    set(libBinPath ${CMAKE_LIB_PATH}/${NAPPGUI_COMPILER_TOOLSET}_${NAPPGUI_ARCH}/${config}/${NAPPGUI_STATIC_LIB_PREFIX}${libName}${NAPPGUI_STATIC_LIB_SUFFIX})
        if (EXISTS ${libBinPath})
            set(${_ret} "TRUE" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    set(${_ret} "FALSE" PARENT_SCOPE)

endfunction()

#------------------------------------------------------------------------------

function(existsPrecompiledDynamicLib libName _ret)

    foreach(config ${CMAKE_CONFIGURATION_TYPES})
	    set(libBinPath ${CMAKE_BIN_PATH}/${NAPPGUI_COMPILER_TOOLSET}_${NAPPGUI_ARCH}/${config}/${NAPPGUI_DYNAMIC_LIB_PREFIX}${libName}${NAPPGUI_DYNAMIC_LIB_SUFFIX})
        if (EXISTS ${libBinPath})
            set(${_ret} "TRUE" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    set(${_ret} "FALSE" PARENT_SCOPE)

endfunction()

#------------------------------------------------------------------------------

function(getRecursiveTargetLinks targetName dependList)

	foreach(depend ${dependList})
		# Dependency is a Target of this solution
		if (TARGET ${depend})
            get_target_property(TARGET_TYPE ${depend} TYPE)
            if (${TARGET_TYPE} STREQUAL "STATIC_LIBRARY" OR ${TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
                appendLinkDependency(${targetName} ${depend})
                getDirectDepends(${depend} childDependList)
            else()
                message(FATAL_ERROR "- ${targetName}: Unknown dependency type '${depend}-${TARGET_TYPE}'")
            endif ()

        else()
            existsPrecompiledDynamicLib(${depend} existsDynamic)
            existsPrecompiledStaticLib(${depend} existsStatic)

            # Dependency is a precompiled dynamic library
            if (existsDynamic)
                if (WIN32)
                    if (NOT existsStatic)
                        message(FATAL_ERROR "- ${targetName}: Dinamic dependency '${depend}' has not present .lib import library.")
                    endif()
                endif()

                appendLinkDependency(${targetName} "__DYNAMIC_LIB__${depend}")
                getDirectDepends(${depend} childDependList)

            # Dependency is a precompiled static library
            elseif (existsStatic)
                appendLinkDependency(${targetName} "__STATIC_LIB__${depend}")
				getDirectDepends(${depend} childDependList)

            else()
				message(FATAL_ERROR "- ${targetName}: Unknown dependency '${depend}'")

			endif()

        endif()

        if (childDependList)
            getRecursiveTargetLinks(${targetName} "${childDependList}")
        endif()

	endforeach()

endfunction()

#------------------------------------------------------------------------------

function(targetSourceDir targetName _ret)
    set(${_ret} "" PARENT_SCOPE)
    foreach(path ${NAPPGUI_CACHE_SOURCE_DIRECTORIES})
        list (FIND NAPPGUI_CACHE_PATH_TARGETS_${path} ${targetName} index)
        if (NOT ${index} EQUAL -1)
            set(${_ret} ${path} PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

#------------------------------------------------------------------------------

function(targetDependOnLib targetName libName _ret)

    set(${_ret} "NO" PARENT_SCOPE)
    targetSourceDir(${targetName} targetDir)
    targetSourceDir(${libName} libDir)

    if (${targetDir} STREQUAL "${libDir}")
        set(${_ret} "YES" PARENT_SCOPE)
        return()
    endif()

    foreach(depend ${${targetName}_LINKDEPENDS})

        if (depend MATCHES "__STATIC_LIB__*")
            string(SUBSTRING ${depend} 14 -1 dependName)
        elseif(depend MATCHES "__DYNAMIC_LIB__*")
            string(SUBSTRING ${depend} 15 -1 dependName)
        else()
            set(dependName ${depend})
        endif()

        targetSourceDir(${dependName} dependDir)
        if (${dependDir} STREQUAL "${libDir}")
            set(${_ret} "YES" PARENT_SCOPE)
            return()
        endif()

    endforeach()

endfunction()

#------------------------------------------------------------------------------

function(installPrecompiledSharedLibrary targetName libName)
    set(DYNLIB_SOURCE "${CMAKE_BIN_PATH}/${NAPPGUI_COMPILER_TOOLSET}_${NAPPGUI_ARCH}/$<CONFIG>/${NAPPGUI_DYNAMIC_LIB_PREFIX}${libName}${NAPPGUI_DYNAMIC_LIB_SUFFIX}")
    set(DEST_PATH "${CMAKE_BINARY_DIR}/$<CONFIG>/bin/${NAPPGUI_DYNAMIC_LIB_PREFIX}${libName}${NAPPGUI_DYNAMIC_LIB_SUFFIX}")
    add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DYNLIB_SOURCE} ${DEST_PATH})
endfunction()

#------------------------------------------------------------------------------

function(targetLinkWithLibraries targetName firstLevelDepends)

    set(${targetName}_LINKDEPENDS "" CACHE INTERNAL "")
    getRecursiveTargetLinks(${targetName} "${firstLevelDepends}")
    get_target_property(TARGET_TYPE ${targetName} TYPE)

    toolsetName(toolset)

    if (${targetName}_LINKDEPENDS)

        foreach(depend ${${targetName}_LINKDEPENDS})
            set(DEPEND_IS_SHAREDLIB FALSE)

            # The dependency is a precompiled static library
            if (depend MATCHES "__STATIC_LIB__*")
                string(SUBSTRING ${depend} 14 -1 dependName)
                set(linkerName ${CMAKE_LIB_PATH}/${toolset}/$<CONFIG>/${NAPPGUI_STATIC_LIB_PREFIX}${dependName}${NAPPGUI_STATIC_LIB_SUFFIX})

            # The dependency is a precompiled dynamic library
            elseif(depend MATCHES "__DYNAMIC_LIB__*")
                string(SUBSTRING ${depend} 15 -1 dependName)
                # In Win32 dynamic libraries link with .lib (static symbol-export library)
                if (WIN32)
                    set(linkerName ${CMAKE_LIB_PATH}/${toolset}/$<CONFIG>/${NAPPGUI_STATIC_LIB_PREFIX}${dependName}${NAPPGUI_STATIC_LIB_SUFFIX})

                elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
                    set(linkerName ${CMAKE_BIN_PATH}/${toolset}/$<CONFIG>/${NAPPGUI_DYNAMIC_LIB_PREFIX}${dependName}${NAPPGUI_DYNAMIC_LIB_SUFFIX})

                endif()

                # Copy the precompiled shared library into executable directory
                if (${TARGET_TYPE} STREQUAL "EXECUTABLE")
                    installPrecompiledSharedLibrary(${targetName} ${dependName})
                endif()

                set(DEPEND_IS_SHAREDLIB TRUE)

            # The dependency is a target
            else()
                set(linkerName ${depend})
                set(dependName ${depend})
                get_target_property(DEPEND_TARGET_TYPE ${depend} TYPE)
                if (${DEPEND_TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
                    set(DEPEND_IS_SHAREDLIB TRUE)
                endif()

            endif()

            if (linkerName)
                target_link_libraries(${targetName} ${linkerName})
            else()
                message(FATAL_ERROR "Cannot resolve the dependendy ${depend} for target ${targetName}")
            endif()

            # If target depends on shared library, we define the 'TARGET_IMPORT' preprocessor macro
            if (DEPEND_IS_SHAREDLIB)
                targetSourceDir(${dependName} targetPath)
                get_filename_component(targetSource ${targetPath} NAME)
                string(TOUPPER ${targetSource} targetSourceUpper)
                set(TARGET_IMPORT ${targetSourceUpper}_IMPORT)
	            set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS ${TARGET_IMPORT})
            endif()

        endforeach()

    endif()

    # Target should link with math
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries(${targetName} "m")
    endif()

    # Target should link with pthread
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        targetDependOnLib(${targetName} "osbs" _depends)
        if (_depends)
            find_package(Threads)
            if (Threads_FOUND)
                target_link_libraries(${targetName} ${CMAKE_THREAD_LIBS_INIT})
            else()
                message(ERROR "- PThread library not found")
            endif()

            target_link_libraries(${targetName} ${CMAKE_DL_LIBS})
        endif()
    endif()

    # Target should link with GTK3
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
            targetDependOnLib(${targetName} "draw2d" _depends)
            if (_depends)
                # Use the package PkgConfig to detect GTK+ headers/library files
                find_package(PkgConfig REQUIRED)
                pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
                target_link_libraries(${targetName} ${GTK3_LIBRARIES})
            endif()
        endif()
    endif()

    # Target should link with libCurl
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        targetDependOnLib(${targetName} "inet" _depends)
        if (_depends)
            find_package(CURL)
            if (${CURL_FOUND})
                target_link_libraries(${targetName} ${CURL_LIBRARY})
            else()
                message(ERROR "- libCURL is required. Try 'sudo apt-get install libcurl4-openssl-dev'")
            endif()
        endif()
    endif()

    # Target should link with Cocoa
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        targetDependOnLib(${targetName} "draw2d" _depends1)
        targetDependOnLib(${targetName} "inet" _depends2)
        if (_depends1 OR _depends2)
            if (NOT ${TARGET_TYPE} STREQUAL "STATIC_LIBRARY")
    			target_link_libraries(${targetName} ${COCOA_LIB})
            endif()
        endif()
	endif()

    # In GCC the g++ linker must be used
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set_target_properties(${targetName} PROPERTIES LINKER_LANGUAGE CXX)
    endif()

endfunction()

#------------------------------------------------------------------------------

function(readPackFile)

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/res/pack.txt)
        file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/res/pack.txt PackContents)
        foreach(NameAndValue ${PackContents})
            # Strip leading spaces
            string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
            # Find variable name
            string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
            # Find the value
            string(REPLACE "${Name}=" "" Value ${NameAndValue})
            # Set the variable
            set(PACK_${Name} "${Value}" PARENT_SCOPE)
       endforeach()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(installerScript targetName platform generator config)
    set(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/CPack-${targetName}-${config}.cmake)
    file(WRITE ${SCRIPT} "# CPack script for ${targetName} project\n")
    file(APPEND ${SCRIPT} "# Automatic generated by NAppGUI\n")
    file(APPEND ${SCRIPT} "# You can configure the installer from /res/pack.txt\n")
    file(APPEND ${SCRIPT} "set(CPACK_GENERATOR \"${generator}\")\n")
    file(APPEND ${SCRIPT} "set(CPACK_INSTALLED_DIRECTORIES \"${CMAKE_CURRENT_BINARY_DIR}/${config};.\")\n")
    file(APPEND ${SCRIPT} "set(CPACK_SYSTEM_NAME \"${platform}_${NAPPGUI_ARCH}\")\n")
    file(APPEND ${SCRIPT} "set(CPACK_PACKAGE_NAME \"${targetName}\")\n")
    file(APPEND ${SCRIPT} "set(CPACK_PACKAGE_VENDOR \"${PACK_VENDOR}\")\n")
    file(APPEND ${SCRIPT} "set(CPACK_PACKAGE_VERSION \"${PACK_VERSION}\")\n")
    file(APPEND ${SCRIPT} "set(CPACK_PACKAGE_DESCRIPTION \"${PACK_DESC}\")\n")
    file(APPEND ${SCRIPT} "set(CPACK_PACKAGE_FILE_NAME \"\${CPACK_PACKAGE_NAME}\")\n")
    #file(APPEND ${SCRIPT} "set(CPACK_PACKAGE_FILE_NAME \"\${CPACK_PACKAGE_NAME}-\${CPACK_PACKAGE_VERSION}-\${CPACK_SYSTEM_NAME}\")\n")
    file(APPEND ${SCRIPT} "set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY \"\${PACK_VENDOR}/\${CPACK_PACKAGE_NAME}-${NAPPGUI_ARCH}\")\n")

    if (${generator} STREQUAL "NSIS")
        file(APPEND ${SCRIPT} "\n")
        file(APPEND ${SCRIPT} "# NSIS generator specific options\n")
        file(APPEND ${SCRIPT} "set(CPACK_NSIS_PACKAGE_NAME \"\${CPACK_PACKAGE_FILE_NAME}\")\n")
        file(APPEND ${SCRIPT} "set(CPACK_NSIS_DISPLAY_NAME \"\${CPACK_PACKAGE_NAME} (${NAPPGUI_ARCH})\")\n")
        file(APPEND ${SCRIPT} "set(CPACK_NSIS_MUI_ICON \"${CMAKE_CURRENT_SOURCE_DIR}/res\\\\logo256.ico\")\n")
        file(APPEND ${SCRIPT} "set(CPACK_NSIS_MUI_UNIICON \"${CMAKE_CURRENT_SOURCE_DIR}/res\\\\logo256.ico\")\n")
        file(APPEND ${SCRIPT} "set(CPACK_PACKAGE_ICON \"${CMAKE_CURRENT_SOURCE_DIR}/res\\\\banner.bmp\")\n")
        file(APPEND ${SCRIPT} "set(CPACK_RESOURCE_FILE_LICENSE \"${CMAKE_CURRENT_SOURCE_DIR}/res/license.txt\")\n")

        if (${NAPPGUI_ARCH} STREQUAL "x64")
            file(APPEND ${SCRIPT} "set(CPACK_NSIS_INSTALL_ROOT \"\$PROGRAMFILES64\\\\\${CPACK_PACKAGE_VENDOR}\\\\\${CPACK_PACKAGE_NAME}\")\n")
        else()
            file(APPEND ${SCRIPT} "set(CPACK_NSIS_INSTALL_ROOT \"\$PROGRAMFILES\\\\\${CPACK_PACKAGE_VENDOR}\\\\\${CPACK_PACKAGE_NAME}\")\n")
        endif()

        file(APPEND ${SCRIPT} "set(CPACK_NSIS_INSTALLED_ICON_NAME \"\${CPACK_PACKAGE_NAME}.exe\")\n")
        file(APPEND ${SCRIPT} "set(CPACK_NSIS_MENU_LINKS \"\${CPACK_PACKAGE_NAME}.exe\" \"\${CPACK_PACKAGE_NAME} (${NAPPGUI_ARCH})\")\n")
        file(APPEND ${SCRIPT} "set(CPACK_NSIS_DEFINES \"!define MUI_STARTMENUPAGE_DEFAULTFOLDER \\\\\${CPACK_PACKAGE_VENDOR}\\\\\${CPACK_PACKAGE_NAME}\")\n")

    elseif (${generator} STREQUAL "DragNDrop")
        file(APPEND ${SCRIPT} "set(CPACK_DMG_VOLUME_NAME \"\${CPACK_PACKAGE_NAME}\")\n")
        add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/res/license.txt ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

    elseif (${generator} STREQUAL "TGZ")
        add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/res/license.txt ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    endif()

endfunction()

#------------------------------------------------------------------------------

function(processInstaller targetName)
    if (CMAKE_PACKAGE_PATH)

        if (NOT EXISTS ${CMAKE_PACKAGE_PATH}/Debug)
            file(MAKE_DIRECTORY ${CMAKE_PACKAGE_PATH}/Debug)
        endif()

        if (NOT EXISTS ${CMAKE_PACKAGE_PATH}/Release)
            file(MAKE_DIRECTORY ${CMAKE_PACKAGE_PATH}/Release)
        endif()

        if (NOT EXISTS ${CMAKE_PACKAGE_PATH}/ReleaseWithAssert)
            file(MAKE_DIRECTORY ${CMAKE_PACKAGE_PATH}/ReleaseWithAssert)
        endif()

	    if (WIN32)
            installerScript(${targetName} "win" ${CMAKE_PACKAGE_GEN} "Debug")
            installerScript(${targetName} "win" ${CMAKE_PACKAGE_GEN} "Release")
            installerScript(${targetName} "win" ${CMAKE_PACKAGE_GEN} "ReleaseWithAssert")
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND cd ${CMAKE_PACKAGE_PATH}/$<CONFIG> & cpack --config ${CMAKE_CURRENT_BINARY_DIR}/CPack-${targetName}-$<CONFIG>.cmake)

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
            installerScript(${targetName} "macos" ${CMAKE_PACKAGE_GEN} "Debug")
            installerScript(${targetName} "macos" ${CMAKE_PACKAGE_GEN} "Release")
            installerScript(${targetName} "macos" ${CMAKE_PACKAGE_GEN} "ReleaseWithAssert")
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND /bin/bash -c \"cd ${CMAKE_PACKAGE_PATH}/$<CONFIG> && ${CPACK_COMMAND} --config ${CMAKE_CURRENT_BINARY_DIR}/CPack-${targetName}-$<CONFIG>.cmake\")

        else()
            installerScript(${targetName} "linux" ${CMAKE_PACKAGE_GEN} "Debug")
            installerScript(${targetName} "linux" ${CMAKE_PACKAGE_GEN} "Release")
            installerScript(${targetName} "linux" ${CMAKE_PACKAGE_GEN} "ReleaseWithAssert")
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND /bin/bash -c \"cd ${CMAKE_PACKAGE_PATH}/$<CONFIG> && ${CPACK_COMMAND} --config ${CMAKE_CURRENT_BINARY_DIR}/CPack-${targetName}-$<CONFIG>.cmake\")

        endif()
    else()
        message("- ${targetName}: set CMAKE_PACKAGE_PATH to create packages")
    endif()
endfunction()

#------------------------------------------------------------------------------

function(processDesktopApp appName dependList options)

    readPackFile()
    set(dependencies "")
    list(APPEND dependencies "${dependList}")
	if (WIN32)
		windowsBundle(${appName} WIN_DESKTOP "${dependencies}")
		set_target_properties(${appName} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
		set_target_properties(${appName} PROPERTIES LINK_FLAGS_RELASE "/SUBSYSTEM:WINDOWS")
		set_target_properties(${appName} PROPERTIES LINK_FLAGS_RELASEWITHASSERT "/SUBSYSTEM:WINDOWS")
        set(macOSBundle NO)
    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        macOSBundle(${appName} "${dependencies}")
        set(macOSBundle YES)

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        linuxBundle(${appName} "${dependencies}")
        set(macOSBundle NO)

    else()
        message("Unknown platform")

	endif()

    targetLinkWithLibraries(${appName} "${dependencies}")
    targetOptions(${appName} "${options}")
    targetRPath(${appName} ${macOSBundle} "")

    if (${CMAKE_PACKAGE})
        processInstaller(${appName})
    endif()

endfunction()

#------------------------------------------------------------------------------

function(macOSCommand appName dependList)
    processTarget("${appName}" APPLE_CONSOLE "${dependList}")
endfunction()

#------------------------------------------------------------------------------

function(linuxCommand appName dependList)
    processTarget("${appName}" LINUX_CONSOLE "${dependList}")
endfunction()

#------------------------------------------------------------------------------

function(processCommandApp appName dependList options)

    set(dependencies "")
    list(APPEND dependencies "${dependList}")

    if (WIN32)
        windowsBundle(${appName} WIN_CONSOLE "${dependencies}")
        set_target_properties(${appName} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
        set_target_properties(${appName} PROPERTIES LINK_FLAGS_RELASE "/SUBSYSTEM:CONSOLE")
        set_target_properties(${appName} PROPERTIES LINK_FLAGS_RELEASEWITHASSERT "/SUBSYSTEM:CONSOLE")

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        macOSCommand("${appName}" "${dependencies}")

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        linuxCommand("${appName}" "${dependencies}")

    else()
        message(ERROR "- ${appName} Unknown system")

    endif()

    targetLinkWithLibraries(${appName} "${dependencies}")
    targetOptions(${appName} "${options}")
    targetRPath(${appName} NO "")

endfunction()

#------------------------------------------------------------------------------

function(setTargetFilter targetName targetPath)

    get_filename_component(filter ${targetPath} PATH)

    if (filter)
        set_target_properties(${targetName} PROPERTIES FOLDER ${filter})
    endif()

endfunction()

#------------------------------------------------------------------------------

function(addTargetCache targetName targetPath nrcMode)

    # Add a new target to target list
    if (NAPPGUI_CACHE_TARGETS)
        list (FIND NAPPGUI_CACHE_TARGETS ${targetName} index)
        if (${index} EQUAL -1)
            set(NAPPGUI_CACHE_TARGETS "${NAPPGUI_CACHE_TARGETS};${targetName}" CACHE INTERNAL "")
        else()
            message(FATAL_ERROR "Target: ${targetName} already defined.")
        endif()
    else()
        set(NAPPGUI_CACHE_TARGETS "${targetName}" CACHE INTERNAL "")
    endif()

    # Append the path to sources
    set(FIRST_TARGET_IN_PATH False)
    if (NAPPGUI_CACHE_SOURCE_DIRECTORIES)
        list (FIND NAPPGUI_CACHE_SOURCE_DIRECTORIES ${targetPath} index)
        if (${index} EQUAL -1)
            set(NAPPGUI_CACHE_SOURCE_DIRECTORIES "${NAPPGUI_CACHE_SOURCE_DIRECTORIES};${targetPath}" CACHE INTERNAL "")
            set(NAPPGUI_CACHE_PATH_TARGETS_${targetPath} "" CACHE INTERNAL "")
            set(FIRST_TARGET_IN_PATH True)
        endif()

    else()
        set(NAPPGUI_CACHE_SOURCE_DIRECTORIES "${targetPath}" CACHE INTERNAL "")
        set(NAPPGUI_CACHE_PATH_TARGETS_${targetPath} "" CACHE INTERNAL "")
        set(FIRST_TARGET_IN_PATH True)

    endif()

    if (FIRST_TARGET_IN_PATH)
        # The target can be a precompiled one
        if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${targetPath})
            file(REMOVE "${targetPath}/CMakeLists.txt")
        endif()
    endif()

    # Append the target to this source path
    if (NAPPGUI_CACHE_PATH_TARGETS_${targetPath})
        set(NAPPGUI_CACHE_PATH_TARGETS_${targetPath} "${NAPPGUI_CACHE_PATH_TARGETS_${targetPath}};${targetName}" CACHE INTERNAL "")
    else()
        set(NAPPGUI_CACHE_PATH_TARGETS_${targetPath} "${targetName}" CACHE INTERNAL "")
    endif()

    # Set NRC mode for target
    set(NAPPGUI_CACHE_TARGET_NRC_MODE_${targetName} ${nrcMode} CACHE INTERNAL "")

endfunction()

#------------------------------------------------------------------------------

function(createLibFiles libPath dependencies)

    get_filename_component(libName ${libPath} NAME)
    set(libPathName ${libPath}/${libName})
    string(TOUPPER ${libName} libNameUpper)

    # New directory
    file(MAKE_DIRECTORY ${libPath})

    # lib.def file
    file(WRITE ${libPathName}.def "/* ${libName} library import/export */\n\n")
    file(APPEND ${libPathName}.def "#if defined(${libNameUpper}_IMPORT)\n")
    file(APPEND ${libPathName}.def "   #if defined(__GNUC__)\n")
    file(APPEND ${libPathName}.def "       #define _${libName}_api\n")
    file(APPEND ${libPathName}.def "   #elif defined(_MSC_VER)\n")
    file(APPEND ${libPathName}.def "       #define _${libName}_api __declspec(dllimport)\n")
    file(APPEND ${libPathName}.def "   #else\n")
    file(APPEND ${libPathName}.def "       #error Unknown compiler\n")
    file(APPEND ${libPathName}.def "   #endif\n")
    file(APPEND ${libPathName}.def "#elif defined(NAPPGUI_SHARED_LIB)\n")
    file(APPEND ${libPathName}.def "   #if defined(__GNUC__)\n")
    file(APPEND ${libPathName}.def "       #define _${libName}_api __attribute__((visibility(\"default\")))\n")
    file(APPEND ${libPathName}.def "   #elif defined(_MSC_VER)\n")
    file(APPEND ${libPathName}.def "       #define _${libName}_api __declspec(dllexport)\n")
    file(APPEND ${libPathName}.def "   #else\n")
    file(APPEND ${libPathName}.def "       #error Unknown compiler\n")
    file(APPEND ${libPathName}.def "   #endif\n")
    file(APPEND ${libPathName}.def "#else\n")
    file(APPEND ${libPathName}.def "   #define _${libName}_api\n")
    file(APPEND ${libPathName}.def "#endif\n")

    # lib.hxx file
    file(WRITE ${libPathName}.hxx "/* ${libName} */\n\n")
    file(APPEND ${libPathName}.hxx "#ifndef __${libNameUpper}_HXX__\n")
    file(APPEND ${libPathName}.hxx "#define __${libNameUpper}_HXX__\n\n")

    foreach(dependency ${dependencies})
        file(APPEND ${libPathName}.hxx "#include \"${dependency}.hxx\"\n")
    endforeach()

    file(APPEND ${libPathName}.hxx "#include \"${libName}.def\"\n")
    file(APPEND ${libPathName}.hxx "\n")
    file(APPEND ${libPathName}.hxx "/* TODO: Define data types here */\n\n")
    file(APPEND ${libPathName}.hxx "#endif\n")

    # lib.h file
    file(WRITE ${libPathName}.h "/* ${libName} */\n\n")
    file(APPEND ${libPathName}.h "#include \"${libName}.hxx\"\n\n")
    file(APPEND ${libPathName}.h "__EXTERN_C\n\n")
    file(APPEND ${libPathName}.h "_${libName}_api void ${libName}_start(void);\n\n")
    file(APPEND ${libPathName}.h "_${libName}_api void ${libName}_finish(void);\n\n")
    file(APPEND ${libPathName}.h "__END_C\n")

    # lib.c file
    file(WRITE ${libPathName}.c "/* ${libName} */\n\n")
    file(APPEND ${libPathName}.c "#include \"${libName}.h\"\n\n")
    file(APPEND ${libPathName}.c "/*---------------------------------------------------------------------------*/\n\n")
    file(APPEND ${libPathName}.c "void ${libName}_start(void)\n")
    file(APPEND ${libPathName}.c "{\n")
    file(APPEND ${libPathName}.c "    /*TODO: Implement library initialization code here */\n")
    file(APPEND ${libPathName}.c "}\n\n")
    file(APPEND ${libPathName}.c "/*---------------------------------------------------------------------------*/\n\n")
    file(APPEND ${libPathName}.c "void ${libName}_finish(void)\n")
    file(APPEND ${libPathName}.c "{\n")
    file(APPEND ${libPathName}.c "    /*TODO: Implement library ending code here */\n")
    file(APPEND ${libPathName}.c "}\n\n")

endfunction()

#------------------------------------------------------------------------------

function(staticLib libName libPath dependencies nrcMode)

    # Cache variables
	set(NAPPGUI_CACHE_DEPENDS_${libName} "${dependencies}" CACHE INTERNAL "")
	set(NAPPGUI_CACHE_LIBPATH_${libName} "${libPath}" CACHE INTERNAL "")
    addTargetCache(${libName} ${libPath} ${nrcMode})

	# Library exists as precompiled binary
    existsPrecompiledStaticLib(${libName} existsBin)
    if (${existsBin})
        message(STATUS "- [OK] ${libName}: Precompiled static library")

    else()
        # Library source location
        set(libSourcePath ${CMAKE_CURRENT_SOURCE_DIR}/${libPath})

		# New Library (doesn't exists yet)
		if (NOT EXISTS ${libSourcePath})
            createLibFiles(${libSourcePath} ${dependencies})
			message("- [WARN] ${libName}: Library not found. Create a new project")
		endif()

		file(APPEND ${libSourcePath}/CMakeLists.txt "processStaticLib(${libName} \"${dependencies}\" \"${ARGV4}\")\n")
        configure_file("${libSourcePath}/CMakeLists.txt" "${libSourcePath}/CMakeLists.txt" NEWLINE_STYLE UNIX)

	endif()

endfunction()

#------------------------------------------------------------------------------

function(dynamicLib libName libPath dependencies nrcMode)

    # Cache variables
	set(NAPPGUI_CACHE_DEPENDS_${libName} "${dependencies}" CACHE INTERNAL "")
	set(NAPPGUI_CACHE_LIBPATH_${libName} "${libPath}" CACHE INTERNAL "")
    addTargetCache(${libName} ${libPath} ${nrcMode})

	# Library exists as precompiled binary
    existsPrecompiledDynamicLib(${libName} existsBin)
    if (${existsBin})
        message(STATUS "- [OK] ${libName}: Precompiled dynamic library")

    else()
        # Library source location
        set(libSourcePath ${CMAKE_CURRENT_SOURCE_DIR}/${libPath})

		# New Library (doesn't exists yet)
		if (NOT EXISTS ${libSourcePath})
            createLibFiles(${libSourcePath} ${dependencies})
			message("- [WARN] ${libName}: Library not found. Create a new project")
		endif()

		file(APPEND ${libSourcePath}/CMakeLists.txt "processDynamicLib(${libName} \"${dependencies}\" \"${ARGV4}\")\n")
        configure_file("${libSourcePath}/CMakeLists.txt" "${libSourcePath}/CMakeLists.txt" NEWLINE_STYLE UNIX)

	endif()

endfunction()

#------------------------------------------------------------------------------

function(commandApp appName appPath dependencies nrcMode)

    addTargetCache(${appName} ${appPath} ${nrcMode})
    set(appFullPath ${CMAKE_CURRENT_SOURCE_DIR}/${appPath})

    if (NOT EXISTS ${appFullPath})
        file(MAKE_DIRECTORY ${appFullPath})

        # main.c
        string(TOLOWER ${appName} appNameLower)
        file(COPY ${CMAKE_PRJ_PATH}/templates/cmdmain.c DESTINATION ${appFullPath})
        file(RENAME ${appFullPath}/cmdmain.c ${appFullPath}/${appNameLower}.c)

        message(NOTICE "- [WARN] ${appName}: Application not found. Create a new project")
    endif()

    file(APPEND ${appFullPath}/CMakeLists.txt "processCommandApp(${appName} \"${dependencies}\" \"${ARGV4}\")\n")
    configure_file("${appFullPath}/CMakeLists.txt" "${appFullPath}/CMakeLists.txt" NEWLINE_STYLE UNIX)

endfunction()

#------------------------------------------------------------------------------

function(desktopApp appName appPath dependencies nrcMode)

    addTargetCache(${appName} ${appPath} ${nrcMode})
    set(appFullPath ${CMAKE_CURRENT_SOURCE_DIR}/${appPath})

    if (NOT EXISTS ${appFullPath})
        set(resPath ${appFullPath}/res)
        file(MAKE_DIRECTORY ${appFullPath})
        file(MAKE_DIRECTORY ${resPath})

        # logo
        file(COPY ${CMAKE_PRJ_PATH}/templates/logo.icns DESTINATION ${resPath})
        file(COPY ${CMAKE_PRJ_PATH}/templates/logo256.ico DESTINATION ${resPath})
        file(COPY ${CMAKE_PRJ_PATH}/templates/logo48.ico DESTINATION ${resPath})

        # Disabled support for installers
        #file(COPY ${CMAKE_PRJ_PATH}/templates/banner.bmp DESTINATION ${resPath})

        # license
        #file(COPY ${CMAKE_PRJ_PATH}/templates/license.txt DESTINATION ${resPath})

        # pack.txt
        #file(WRITE ${resPath}/pack.txt "VENDOR=NAppGUI\n")
        #file(APPEND ${resPath}/pack.txt "VERSION=1.0\n")
        #file(APPEND ${resPath}/pack.txt "DESC=Write here the application description\n")

        # main.c
        file(COPY ${CMAKE_PRJ_PATH}/templates/main.c DESTINATION ${appFullPath})
        string(TOLOWER ${appName} appNameLower)
        file(RENAME ${appFullPath}/main.c ${appFullPath}/${appNameLower}.c)

        message(NOTICE "- [WARN] ${appName}: Application not found. Create a new project")
    endif()

    file(APPEND ${appFullPath}/CMakeLists.txt "processDesktopApp(${appName} \"${dependencies}\" \"${ARGV4}\")\n")
    configure_file("${appFullPath}/CMakeLists.txt" "${appFullPath}/CMakeLists.txt" NEWLINE_STYLE UNIX)

endfunction()

#------------------------------------------------------------------------------

function(generateSolution)

    foreach(sourcePath ${NAPPGUI_CACHE_SOURCE_DIRECTORIES})

        # Precompiled targets don't have to be added
        if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${sourcePath})
            add_subdirectory(${sourcePath})

            foreach(target ${NAPPGUI_CACHE_PATH_TARGETS_${sourcePath}})
                setTargetFilter(${target} ${sourcePath})
            endforeach()

            # We can't remove project specific 'CMakeLists.txt' because problems in macOS
            # file(REMOVE "${sourcePath}/CMakeLists.txt")
        endif()

    endforeach()

endfunction()
