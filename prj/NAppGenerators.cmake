#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

function(nap_generator_multiconfig _ret)

    if (CMAKE_GENERATOR MATCHES "Visual Studio")
        set(${_ret} True PARENT_SCOPE)

    elseif(CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
        set(${_ret} True PARENT_SCOPE)

    elseif(CMAKE_GENERATOR MATCHES "Ninja")
        set(${_ret} False PARENT_SCOPE)

    elseif(CMAKE_GENERATOR MATCHES "MinGW Makefiles")
        set(${_ret} False PARENT_SCOPE)

    elseif(CMAKE_GENERATOR MATCHES "MSYS Makefiles")
        set(${_ret} False PARENT_SCOPE)

    elseif(CMAKE_GENERATOR MATCHES "Unix Makefiles")
        set(${_ret} False PARENT_SCOPE)

    elseif(CMAKE_GENERATOR STREQUAL "Xcode")
        set(${_ret} True PARENT_SCOPE)

    else()
        message(FATAL_ERROR "Unsuported generator: ${CMAKE_GENERATOR}")

    endif()

endfunction()
