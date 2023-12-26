#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

function(nap_build_opt optName opt)
    if(NOT NAPPGUI_IS_PACKAGE)
        message(STATUS "* ${optName}: ${opt}")
    endif()
endfunction()

