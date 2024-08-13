#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------

if (NOT NAPPGUI_ROOT_PATH)
    message(FATAL_ERROR "NAPPGUI_ROOT_PATH is not set.")
endif()

#------------------------------------------------------------------------------

# Configurations
include(${NAPPGUI_ROOT_PATH}/prj/NAppGenerators.cmake)
nap_generator_multiconfig(isMultiConfig)

if (isMultiConfig)
    set(CMAKE_BUILD_TYPE "Multiconfig [${CMAKE_CONFIGURATION_TYPES}]")
else()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${CMAKE_CONFIGURATION_TYPES}")
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug")
    endif()
endif()

#------------------------------------------------------------------------------
# Compiler Settings for all targets in the solution
#------------------------------------------------------------------------------
macro(nap_config_compiler)

include(${NAPPGUI_ROOT_PATH}/prj/NAppUtils.cmake)

if (WIN32)

    if (${CMAKE_SIZEOF_VOID_P} STREQUAL 4)
        set(CMAKE_HOST_ARCHITECTURE "x86")
        set(CMAKE_ARCHITECTURE "x86")
    elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
        set(CMAKE_HOST_ARCHITECTURE "x64")
        set(CMAKE_ARCHITECTURE "x64")
    else ()
        message (FATAL_ERROR "Unknown processor architecture")
    endif()

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)

        include(${NAPPGUI_ROOT_PATH}/prj/NAppMSVC.cmake)

        # Standard C Library Static or Dynamic
        # set(CMAKE_RUNTIME_LIBRARY static CACHE STRING "C Standard library static or dynamic linking.")
        # set_property(CACHE CMAKE_RUNTIME_LIBRARY PROPERTY STRINGS "static;dynamic")
        set(CMAKE_RUNTIME_LIBRARY "static")
        nap_msvc_flags(${CMAKE_RUNTIME_LIBRARY})

        # msvc definitions
        nap_msvc_warnings(msvcWarnList)
        string(REPLACE ";" " " msvcWarnList "${msvcWarnList}")
        string(REGEX REPLACE "/W[1-3]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast ${msvcWarnList}")
        string(REGEX REPLACE "/W[1-3]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:fast ${msvcWarnList}")
        add_definitions(-DUNICODE -D_UNICODE /nologo)

        # For Non-Visual Studio generators
        if (NOT CMAKE_VS_PLATFORM_TOOLSET)
            nap_msvc_toolset()
        endif()

        # Enhaced instruction set x86 Processors
        if (${CMAKE_ARCHITECTURE} STREQUAL "x86")
            if (${CMAKE_VS_PLATFORM_TOOLSET} STREQUAL "v80")
                # Not Set in VS2005 is /arch:IA32
            elseif (${CMAKE_VS_PLATFORM_TOOLSET} STREQUAL "v90")
                add_definitions(/arch:SSE)
            else()
                add_definitions(/arch:SSE2)
            endif()
        endif()

        set(CMAKE_COMPILER_TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET})

    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        include(${NAPPGUI_ROOT_PATH}/prj/NAppGCC.cmake)

        # Required macros for Windows (defined in MSVC)
        add_definitions(-D_WINDOWS -DUNICODE -D_UNICODE)

        if (${CMAKE_SIZEOF_VOID_P} STREQUAL 4)
            add_definitions(-D_M_IX86)
        elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
            add_definitions(-D_M_AMD64)
        endif()

        # GCC Version
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
            message(FATAL_ERROR "GCC 4.6 is the minimum supported version in Linux.")
        endif()

        # Compiler toolset
        set(CMAKE_COMPILER_TOOLSET mwgcc${CMAKE_CXX_COMPILER_VERSION})
        string(REPLACE "." "_" CMAKE_COMPILER_TOOLSET ${CMAKE_COMPILER_TOOLSET})

        # Set extra flags for GCC 'CMAKE_CXX_FLAGS' 'CMAKE_C_FLAGS'
        nap_gcc_flags(${CMAKE_ARCHITECTURE})

    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        include(${NAPPGUI_ROOT_PATH}/prj/NAppClang.cmake)

        # Required macros for Windows (defined in MSVC)
        add_definitions(-D_WINDOWS -DUNICODE -D_UNICODE)

        if (${CMAKE_SIZEOF_VOID_P} STREQUAL 4)
            add_definitions(-D_M_IX86)
        elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
            add_definitions(-D_M_AMD64)
        endif()

        # Clang Version
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
            message(FATAL_ERROR "Clang 5.0 is the minimum supported version in Windows.")
        endif()

        # Compiler toolset
        set(CMAKE_COMPILER_TOOLSET mwclang${CMAKE_CXX_COMPILER_VERSION})
        string(REPLACE "." "_" CMAKE_COMPILER_TOOLSET ${CMAKE_COMPILER_TOOLSET})

        # Set extra flags for Clang 'CMAKE_CXX_FLAGS' 'CMAKE_C_FLAGS'
        nap_clang_flags(${CMAKE_ARCHITECTURE})

    else()
        message (FATAL_ERROR "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")

    endif()

# Apple configuration
#------------------------------------------------------------------------------
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")

    include(${NAPPGUI_ROOT_PATH}/prj/NAppMacOS.cmake)

    # Get the Base SDK
    if (NOT CMAKE_OSX_SYSROOT)
        message(FATAL_ERROR "CMAKE_OSX_SYSROOT is not set")
    endif()

    get_filename_component(CMAKE_BASE_OSX_SDK ${CMAKE_OSX_SYSROOT} NAME)
    string(REPLACE "MacOSX" "" CMAKE_BASE_OSX_SDK ${CMAKE_BASE_OSX_SDK})
    string(REPLACE ".sdk" "" CMAKE_BASE_OSX_SDK ${CMAKE_BASE_OSX_SDK})

    # Get the Deployment target
    set(CMAKE_OSX_DEPLOYMENT_TARGET ${CMAKE_BASE_OSX_SDK} CACHE STRING "Minimun macOS SDK to support")
    if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET ${CMAKE_BASE_OSX_SDK})
    endif()

    # Compiler toolset
    set(CMAKE_COMPILER_TOOLSET sdk${CMAKE_OSX_DEPLOYMENT_TARGET})
    string(REPLACE "." "_" CMAKE_COMPILER_TOOLSET ${CMAKE_COMPILER_TOOLSET})

    # Print BaseSDK/Deployment messages
    nap_macos_sdk_name("${CMAKE_BASE_OSX_SDK}" baseSDKName)
    nap_macos_sdk_name("${CMAKE_OSX_DEPLOYMENT_TARGET}" deploySDKName)
    nap_build_opt("BASE_OSX" "${CMAKE_BASE_OSX_SDK}")
    nap_build_opt("BASE_OSX_NAME" "${baseSDKName}")
    nap_build_opt("DEPLOYMENT_TARGET_OSX" "${CMAKE_OSX_DEPLOYMENT_TARGET}")
    nap_build_opt("DEPLOYMENT_TARGET_OSX_NAME" "${deploySDKName}")

    # Invalid deployment target
    if (CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER CMAKE_BASE_OSX_SDK)
        message(FATAL_ERROR "Deployment target '${CMAKE_OSX_DEPLOYMENT_TARGET}' greater than base SDK '${CMAKE_BASE_OSX_SDK}'")
    endif()

    # Host architecture
    if (CMAKE_HOST_SYSTEM_PROCESSOR)
        nap_osx_arch(${CMAKE_HOST_SYSTEM_PROCESSOR} CMAKE_HOST_ARCHITECTURE)
    else()
        message(FATAL_ERROR "- Unknown 'CMAKE_HOST_SYSTEM_PROCESSOR'")
    endif()

    # Build architecture
    if (NOT CMAKE_ARCHITECTURE)
        set(CMAKE_ARCHITECTURE ${CMAKE_HOST_ARCHITECTURE})
    endif()
    nap_osx_build_arch(${CMAKE_ARCHITECTURE} CMAKE_OSX_ARCHITECTURES)

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
        include(${NAPPGUI_ROOT_PATH}/prj/NAppAppleClang.cmake)
        nap_apple_clang_flags()

    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        include(${NAPPGUI_ROOT_PATH}/prj/NAppGCC.cmake)

        # GCC Version
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.2)
            message(FATAL_ERROR "GCC 4.2 is the minimum supported version in macOS.")
        endif()

        nap_gcc_flags("")

    else()
        message (FATAL_ERROR "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")

    endif()

    if(CMAKE_GENERATOR STREQUAL "Xcode")
        nap_build_opt("XCODE_VERSION" "${XCODE_VERSION}")
    endif()

    # Libraries
    set(COCOA_LIB ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/Cocoa.framework)
    if (CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER 11.9999)
        set(COCOA_LIB ${COCOA_LIB};${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/UniformTypeIdentifiers.framework)
    endif()

# Linux configuration
#------------------------------------------------------------------------------
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

    # Look for Linux platform
    find_program(LSB_RELEASE lsb_release)
    execute_process(COMMAND ${LSB_RELEASE} -is OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${LSB_RELEASE} -rs OUTPUT_VARIABLE LSB_RELEASE_VERSION_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    unset(LSB_RELEASE CACHE)

    if (LSB_RELEASE_ID_SHORT AND LSB_RELEASE_VERSION_SHORT)
        nap_build_opt("LINUX_PLATFORM" "${LSB_RELEASE_ID_SHORT} ${LSB_RELEASE_VERSION_SHORT}")
    else()
        nap_build_opt("LINUX_PLATFORM" "Unknown")
    endif()

    # Host architecture
    # Intel 32 bits
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "i386" OR
        ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "i486" OR
        ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "i586" OR
        ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "i686")
        set(CMAKE_HOST_ARCHITECTURE "x86")
        set(CMAKE_ARCHITECTURE "x86" CACHE STRING "Processor architecture")
        set_property(CACHE CMAKE_ARCHITECTURE PROPERTY STRINGS x86)

    # Intel 64 bits
    elseif (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(CMAKE_HOST_ARCHITECTURE "x64")
        set(CMAKE_ARCHITECTURE "x64" CACHE STRING "Processor architecture")
        set_property(CACHE CMAKE_ARCHITECTURE PROPERTY STRINGS x86 x64)

    # ARM 64 bits
    elseif (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64")
        set(CMAKE_HOST_ARCHITECTURE "arm64")
        set(CMAKE_ARCHITECTURE "arm64" CACHE STRING "Processor architecture")
        set_property(CACHE CMAKE_ARCHITECTURE PROPERTY STRINGS arm64)

    # ARM 32 bits
    elseif (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "armv7.*")
        set(CMAKE_HOST_ARCHITECTURE "arm")
        set(CMAKE_ARCHITECTURE "arm" CACHE STRING "Processor architecture")
        set_property(CACHE CMAKE_ARCHITECTURE PROPERTY STRINGS arm)

    else()
        message(FATAL_ERROR "- Unknown 'CMAKE_HOST_SYSTEM_PROCESSOR:${CMAKE_HOST_SYSTEM_PROCESSOR}'")

    endif()

    # GCC Compiler is used
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        include(${NAPPGUI_ROOT_PATH}/prj/NAppGCC.cmake)

        # GCC Version
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
            message(FATAL_ERROR "GCC 4.6 is the minimum supported version in Linux.")
        endif()

        # Compiler toolset
        set(CMAKE_COMPILER_TOOLSET gcc${CMAKE_CXX_COMPILER_VERSION})
        string(REPLACE "." "_" CMAKE_COMPILER_TOOLSET ${CMAKE_COMPILER_TOOLSET})

        # Set extra flags for GCC 'CMAKE_CXX_FLAGS' 'CMAKE_C_FLAGS'
        nap_gcc_flags(${CMAKE_ARCHITECTURE})

    # Clang Compiler is used
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        include(${NAPPGUI_ROOT_PATH}/prj/NAppClang.cmake)

        # Clang Version
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.0)
            message(FATAL_ERROR "Clang 3.0 is the minimum supported version in Linux.")
        endif()

        # Compiler toolset
        set(CMAKE_COMPILER_TOOLSET clang${CMAKE_CXX_COMPILER_VERSION})
        string(REPLACE "." "_" CMAKE_COMPILER_TOOLSET ${CMAKE_COMPILER_TOOLSET})

        # Set extra flags for Clang 'CMAKE_CXX_FLAGS' 'CMAKE_C_FLAGS'
        nap_clang_flags(${CMAKE_ARCHITECTURE})

    else()
        message (FATAL_ERROR "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")

    endif()

    # ToolKit
    set(CMAKE_TOOLKIT "GTK3" CACHE STRING "User interface Toolkit")
    set_property(CACHE CMAKE_TOOLKIT PROPERTY STRINGS "GTK3;None")

    if (${CMAKE_TOOLKIT} STREQUAL "None")
        nap_build_opt("TOOLKIT" "None")

    elseif (${CMAKE_TOOLKIT} STREQUAL "GTK3")
        nap_build_opt("TOOLKIT" "GTK3")
        set(CMAKE_COMPILER_TOOLSET ${CMAKE_COMPILER_TOOLSET}_gtk3)

    else()
        message(FATAL_ERROR "Unknown toolkit ${CMAKE_TOOLKIT}")

    endif()

else()
    message(FATAL_ERROR "Unknown Platform (${CMAKE_SYSTEM_NAME})")

endif()

set(CMAKE_PACKAGE_ID "${CMAKE_COMPILER_TOOLSET}_${CMAKE_ARCHITECTURE}")

endmacro()
