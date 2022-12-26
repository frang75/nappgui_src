# CMake build globals script
# This file is part of NAppGUI-SDK project
# See README.txt and LICENSE.txt

add_definitions(-DNAPPGUI_MAJOR="${NAPPGUI_MAJOR}")
add_definitions(-DNAPPGUI_MINOR="${NAPPGUI_MINOR}")
add_definitions(-DNAPPGUI_REVISION="${NAPPGUI_REVISION}")
add_definitions(-DNAPPGUI_BUILD="${NAPPGUI_BUILD}")
add_definitions(-DNAPPGUI_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_definitions(-DNAPPGUI_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Welcome
#------------------------------------------------------------------------------
message (STATUS "---------------------------------------------")
if(NOT ${NAPPGUI_BUILD} STREQUAL "")
	message (STATUS "NAppGUI Cross-Platform SDK ${NAPPGUI_MAJOR}.${NAPPGUI_MINOR}.${NAPPGUI_REVISION}.${NAPPGUI_BUILD}")
else()
	message (STATUS "NAppGUI Cross-Platform SDK ${NAPPGUI_MAJOR}.${NAPPGUI_MINOR}.${NAPPGUI_REVISION}")
endif()

string(TIMESTAMP CURRENT_YEAR %Y)
message (STATUS "2015-${CURRENT_YEAR} Francisco Garcia Collado")
message (STATUS "MIT License")
message (STATUS "---------------------------------------------")
message (STATUS "${CMAKE_GENERATOR}")

# Build configurations
#------------------------------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReleaseWithAssert" CACHE INTERNAL "Configuration Types" FORCE)

# foreach(config ${CMAKE_CONFIGURATION_TYPES})
#     string(TOUPPER ${config} configUpper)
#     set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/lib")
#     set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/bin")
#     set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/bin")
# endforeach()

# RELEASEWITHASSERT inherits from RELEASE
set(CMAKE_CXX_FLAGS_RELEASEWITHASSERT ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_RELEASEWITHASSERT ${CMAKE_C_FLAGS_RELEASE})
set(CMAKE_EXE_LINKER_FLAGS_RELEASEWITHASSERT ${CMAKE_EXE_LINKER_FLAGS_RELEASE})

# Global variables
#------------------------------------------------------------------------------
set(PUBLIC_HEADER_EXTENSION "*.h;*.hxx;*.hpp;*.def")
set(HEADER_EXTENSION "${PUBLIC_HEADER_EXTENSION};*.inl;*.ixx;*.ipp")
set(SRC_EXTENSION "${HEADER_EXTENSION};*.c;*.cpp")
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(SRC_EXTENSION "${SRC_EXTENSION};*.m")
endif()

set(RES_EXTENSION "*.png" "*.jpg" "*.gif" "*.msg" "*.*")
set(NAPPGUI_CACHE_SOURCE_DIRECTORIES "" CACHE INTERNAL "")
set(NAPPGUI_CACHE_TARGETS "" CACHE INTERNAL "")

# Global options
#------------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
get_filename_component(CPACK_COMMAND ${CMAKE_COMMAND} PATH)
set(CPACK_COMMAND ${CPACK_COMMAND}/cpack)

# Policies
#------------------------------------------------------------------------------

# https://cmake.org/cmake/help/latest/policy/CMP0068.html
# RPATH settings on macOS do not affect install_name.
# CMake 3.9 and newer remove any effect the following settings may have on the
# install_name of a target on macOS
if(${CMAKE_VERSION} VERSION_GREATER "3.8.999")
    cmake_policy(SET CMP0068 NEW)
endif()
