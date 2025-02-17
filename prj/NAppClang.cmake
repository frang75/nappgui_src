#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

macro(nap_clang_flags arch)

    # Warnings
    set(FLAGS "-Wall -Wextra -pedantic -fPIE")
    set(FLAGS "${FLAGS} -Wimplicit-function-declaration")
    set(FLAGS "${FLAGS} -Wunused")
    set(FLAGS "${FLAGS} -Wunused-function")
    set(FLAGS "${FLAGS} -Wunused-variable")
    set(FLAGS "${FLAGS} -Wunused-value")
    set(FLAGS "${FLAGS} -Wuninitialized")

    # Disable warnings
    set(FLAGS "${FLAGS} -Wno-long-long -Wno-overlength-strings -Wno-newline-eof -Wno-undefined-var-template")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

    # Architecture
    if (${arch} STREQUAL "x86")
        set(ARCH_FLAG "-m32")

    elseif (${arch} STREQUAL "x64")
        set(ARCH_FLAG "-m64")

    elseif (${arch} STREQUAL "arm64")
        # Ok!

    elseif (${arch} STREQUAL "arm")
        set(ARCH_FLAG "-march=armv7-a")

    endif()

    if (ARCH_FLAG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH_FLAG}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAG}")
    endif()


    # # Force ANSI-C for old compilers
    # if(${CMAKE_VERSION} VERSION_LESS "3.1.0" OR ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "5.0.0")
    #     set(${options} "${${options}};-std=gnu90")
    # endif()

endmacro()

