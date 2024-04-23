#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

macro(nap_apple_clang_flags)

    # Warnings
    set(FLAGS "-Wall -Wextra -pedantic -fPIE -Wno-long-long -Wno-overlength-strings -Wno-newline-eof")

    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "10.0.0")
    	set(FLAGS "${FLAGS} -Wno-extended-offsetof")
    endif()

    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "7.99.9999")
    	set(FLAGS "${FLAGS} -Wno-undefined-var-template")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Xlinker -w")

endmacro()
