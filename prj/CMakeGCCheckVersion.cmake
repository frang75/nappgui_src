# CMake Linux/GCC Version Checker
# This file is part of NAppGUI-SDK project
# See README.txt and LICENSE.txt

# Check for valid GCC version
#------------------------------------------------------------------------------
macro(checkGCCVersion)

    if (CMAKE_COMPILER_IS_GNUCC)
       if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
            message(FATAL_ERROR "GCC 4.6 is the minimum supported version in Linux.")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
            set(GCC_VERSION "gcc4_6")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
            set(GCC_VERSION "gcc4_8")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
            set(GCC_VERSION "gcc4_9")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6)
            set(GCC_VERSION "gcc5")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
            set(GCC_VERSION "gcc6")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
            set(GCC_VERSION "gcc7")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
            set(GCC_VERSION "gcc8")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
            set(GCC_VERSION "gcc9")
       elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
            set(GCC_VERSION "gcc10")
       else()
            set(GCC_VERSION "gcc11")
       endif()

    else()
        message(FATAL_ERROR "GCC Is not present.")

    endif()

endmacro()

