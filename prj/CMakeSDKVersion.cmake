# SDK version
# This file is part of NAppGUI-SDK project
# See README.txt and LICENSE.txt

set(NAPPGUI_MAJOR 1)
set(NAPPGUI_MINOR 3)
set(NAPPGUI_REVISION 0)

# Build version
#------------------------------------------------------------------------------
# This is a 'Release' distribution
set (VERSION_FILE ${CMAKE_PRJ_PATH}/build.txt)
if (EXISTS "${VERSION_FILE}")
    set(RELEASE_DISTRIBUTION YES)
    file(READ ${VERSION_FILE} NAPPGUI_BUILD)
    string(REPLACE "\r\n" "" NAPPGUI_BUILD ${NAPPGUI_BUILD})
    string(REPLACE "\n" "" NAPPGUI_BUILD ${NAPPGUI_BUILD})

# This a 'Development' version
else()

    find_package(Subversion)
    if (SUBVERSION_FOUND)
        Subversion_WC_INFO(${CMAKE_CURRENT_LIST_DIR} SVN_NAPPGUI)
        set(NAPPGUI_BUILD ${SVN_NAPPGUI_WC_REVISION})
    endif()

endif()

if (NOT NAPPGUI_BUILD)
    message("- Unknown NAppGUI Revision")
    set(NAPPGUI_BUILD "XXXX")
endif()
