#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

if (NOT NAPPGUI_ROOT_PATH)
    message(FATAL_ERROR "NAPPGUI_ROOT_PATH is not set.")
endif()

set (NAPPGUI_PRJ_PATH "${NAPPGUI_ROOT_PATH}/prj")
include("${NAPPGUI_PRJ_PATH}/NAppTarget.cmake")

#------------------------------------------------------------------------------

function(nap_project_desktop_app appName appPath)

    set(appFullPath "${CMAKE_CURRENT_SOURCE_DIR}/${appPath}")

    if (NOT EXISTS "${appFullPath}")
        set(resPath "${appFullPath}/res")
        file(MAKE_DIRECTORY "${appFullPath}")
        file(MAKE_DIRECTORY "${resPath}")

        # logo
        file(COPY "${NAPPGUI_PRJ_PATH}/templates/logo.icns" DESTINATION "${resPath}")
        file(COPY "${NAPPGUI_PRJ_PATH}/templates/logo256.ico" DESTINATION "${resPath}")
        file(COPY "${NAPPGUI_PRJ_PATH}/templates/logo48.ico" DESTINATION "${resPath}")

        # main.c
        file(COPY "${NAPPGUI_PRJ_PATH}/templates/main-desktop.c" DESTINATION "${appFullPath}")
        string(TOLOWER "${appName}" appNameLower)
        file(RENAME "${appFullPath}/main-desktop.c" "${appFullPath}/${appNameLower}.c")

        # CMakeLists.txt
        file(APPEND "${appFullPath}/CMakeLists.txt" "nap_desktop_app(${appName} \"\" NRC_NONE)\n")
    endif()

    add_subdirectory("${appPath}")

endfunction()

#------------------------------------------------------------------------------

function(nap_project_command_app appName appPath)

    set(appFullPath "${CMAKE_CURRENT_SOURCE_DIR}/${appPath}")

    if (NOT EXISTS "${appFullPath}")
        file(MAKE_DIRECTORY "${appFullPath}")

        # main.c
        string(TOLOWER "${appName}" appNameLower)
        file(COPY "${NAPPGUI_PRJ_PATH}/templates/main-cmd.c" DESTINATION "${appFullPath}")
        file(RENAME "${appFullPath}/main-cmd.c" "${appFullPath}/${appNameLower}.c")

        # CMakeLists.txt
        file(APPEND "${appFullPath}/CMakeLists.txt" "nap_command_app(${appName} \"\" NRC_NONE)\n")
    endif()

    add_subdirectory("${appPath}")

endfunction()

#------------------------------------------------------------------------------

function(nap_project_library libName libPath)

    set(libFullPath "${CMAKE_CURRENT_SOURCE_DIR}/${libPath}")

    if (NOT EXISTS "${libFullPath}")
        file(MAKE_DIRECTORY "${libFullPath}")

        set(libPathName "${libFullPath}/${libName}")
        string(TOUPPER ${libName} libNameUpper)

        # lib.def file
        file(WRITE ${libPathName}.def "/* ${libName} library import/export */\n\n")
        file(APPEND ${libPathName}.def "#if defined(NAPPGUI_SHARED)\n")
        file(APPEND ${libPathName}.def "    #if defined(NAPPGUI_BUILD_${libNameUpper}_LIB)\n")
        file(APPEND ${libPathName}.def "        #define NAPPGUI_${libNameUpper}_EXPORT_DLL\n")
        file(APPEND ${libPathName}.def "    #else\n")
        file(APPEND ${libPathName}.def "        #define NAPPGUI_${libNameUpper}_IMPORT_DLL\n")
        file(APPEND ${libPathName}.def "    #endif\n")
        file(APPEND ${libPathName}.def "#endif\n")
        file(APPEND ${libPathName}.def "\n")
        file(APPEND ${libPathName}.def "#if defined(__GNUC__)\n")
        file(APPEND ${libPathName}.def "    #if defined(NAPPGUI_${libNameUpper}_EXPORT_DLL)\n")
        file(APPEND ${libPathName}.def "        #define _${libName}_api __attribute__((visibility(\"default\")))\n")
        file(APPEND ${libPathName}.def "    #else\n")
        file(APPEND ${libPathName}.def "        #define _${libName}_api\n")
        file(APPEND ${libPathName}.def "    #endif\n")
        file(APPEND ${libPathName}.def "#elif defined(_MSC_VER)\n")
        file(APPEND ${libPathName}.def "    #if defined(NAPPGUI_${libNameUpper}_IMPORT_DLL)\n")
        file(APPEND ${libPathName}.def "        #define _${libName}_api __declspec(dllimport)\n")
        file(APPEND ${libPathName}.def "    #elif defined(NAPPGUI_${libNameUpper}_EXPORT_DLL)\n")
        file(APPEND ${libPathName}.def "        #define _${libName}_api __declspec(dllexport)\n")
        file(APPEND ${libPathName}.def "    #else\n")
        file(APPEND ${libPathName}.def "        #define _${libName}_api\n")
        file(APPEND ${libPathName}.def "    #endif\n")
        file(APPEND ${libPathName}.def "#else\n")
        file(APPEND ${libPathName}.def "    #error Unknown compiler\n")
        file(APPEND ${libPathName}.def "#endif\n")

        # lib.hxx file
        file(WRITE ${libPathName}.hxx "/* ${libName} */\n\n")
        file(APPEND ${libPathName}.hxx "#ifndef __${libNameUpper}_HXX__\n")
        file(APPEND ${libPathName}.hxx "#define __${libNameUpper}_HXX__\n\n")
        file(APPEND ${libPathName}.hxx "#include \"${libName}.def\"\n")
        file(APPEND ${libPathName}.hxx "#include <core/core.hxx>\n")
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

        # CMakeLists.txt
        file(APPEND "${libFullPath}/CMakeLists.txt" "nap_library(${libName} \"\" NO NRC_NONE)\n")
        if (NAPPGUI_IS_PACKAGE)
            file(APPEND "${libFullPath}/CMakeLists.txt" "target_include_directories(${libName} PUBLIC \"\${NAPPGUI_INCLUDE_PATH}\")\n")
        endif()

    endif()

    add_subdirectory("${libPath}")

endfunction()


