# CMake Linux/GCC Version Checker
# This file is part of NAppGUI-SDK project
# See README.txt and LICENSE.txt

# Check for valid non-MSVC compiler version
#------------------------------------------------------------------------------
macro(checkNonMSVCVersion)
    if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
            OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        string(REPLACE "." "_" COMPILER_VERSION "${CMAKE_C_COMPILER_VERSION}")
        set(COMPILER_VERSION "clang${COMPILER_VERSION}")
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
       if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
            message(FATAL_ERROR "GCC 4.6 is the minimum supported version in Linux.")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
            set(COMPILER_VERSION "gcc4_6")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
            set(COMPILER_VERSION "gcc4_8")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
            set(COMPILER_VERSION "gcc4_9")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6)
            set(COMPILER_VERSION "gcc5")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
            set(COMPILER_VERSION "gcc6")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
            set(COMPILER_VERSION "gcc7")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
            set(COMPILER_VERSION "gcc8")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
            set(COMPILER_VERSION "gcc9")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
            set(COMPILER_VERSION "gcc10")
       else()
            set(COMPILER_VERSION "gcc11")
       endif()

    else()
        message(FATAL_ERROR "GCC Is not present.")

    endif()

endmacro()

