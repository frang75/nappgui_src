#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

function(nap_generate_tools)

    if (NOT NAPPGUI_ROOT_PATH)
        message(FATAL_ERROR "NAPPGUI_ROOT_PATH is not set.")
    endif()

    set(TOOLS_SRC_DIR "${NAPPGUI_ROOT_PATH}/tools")
    set(TOOLS_BUILD_DIR "${CMAKE_BINARY_DIR}/tools/build")
    set(NAPPGUI_NRC "${TOOLS_BUILD_DIR}/Debug/bin/nrc${CMAKE_EXECUTABLE_SUFFIX}")

    message(STATUS "- Generating NAppGUI Tools...")

    if (NOT EXISTS ${NAPPGUI_NRC})

        # Create the build folder
        execute_process(COMMAND ${CMAKE_COMMAND} "-E" make_directory "${TOOLS_BUILD_DIR}")

        include(${NAPPGUI_ROOT_PATH}/prj/NAppGenerators.cmake)
        nap_generator_multiconfig(isMultiConfig)
        if (isMultiConfig)
            set(CMAKE_BUILD_OPTS "--config Debug")
        endif()

        # CMake configure
        execute_process(
                    COMMAND ${CMAKE_COMMAND}
                    "-E" chdir "${TOOLS_BUILD_DIR}"
                    ${CMAKE_COMMAND} "-G" "${CMAKE_GENERATOR}"
                    "-DNAPPGUI_ROOT_PATH=${NAPPGUI_ROOT_PATH}"
                    "-DNAPPGUI_MAJOR=${NAPPGUI_MAJOR}"
                    "-DNAPPGUI_MINOR=${NAPPGUI_MINOR}"
                    "-DNAPPGUI_PATCH=${NAPPGUI_PATCH}"
                    "-DNAPPGUI_BUILD=${NAPPGUI_BUILD}"
                    ${TOOLS_SRC_DIR}
                    RESULT_VARIABLE CMakeConfigResult
                    OUTPUT_VARIABLE CMakeConfigOutput
                    ERROR_VARIABLE CMakeConfigError)

        # CMake returns non-zero
        if (CMakeConfigResult)
            message("- Error generating NAppGUI Tools")
            message("CMake Result: ${CMakeConfigResult}")
            message("CMake Output: ${CMakeConfigOutput}")
            message("CMake Error: ${CMakeConfigError}")
            message(FATAL_ERROR "Abort")
        endif()

        # CMake build
        execute_process(
                    COMMAND ${CMAKE_COMMAND}
                    "--build" "${TOOLS_BUILD_DIR}"
                    RESULT_VARIABLE CMakeBuildResult
                    OUTPUT_VARIABLE CMakeBuildOutput
                    ERROR_VARIABLE CMakeBuildError)

        # CMake build returns non-zero
        if (CMakeBuildResult)
            message("- Error building NAppGUI Tools")
            message("CMake Result: ${CMakeBuildResult}")
            message("CMake Output: ${CMakeBuildOutput}")
            message("CMake Error: ${CMakeBuildError}")
            message(FATAL_ERROR "Abort")
        endif()

        if (NOT EXISTS ${NAPPGUI_NRC})
            message(FATAL_ERROR "Error generating ${NAPPGUI_NRC}")
        endif()

        message(STATUS "- [NRC] tool successfully generated '${NAPPGUI_NRC}'")

    else()
        message(STATUS "- [NRC] tool already generated '${NAPPGUI_NRC}'")

    endif()

    set(NAPPGUI_NRC "${NAPPGUI_NRC}" PARENT_SCOPE)

endfunction()
