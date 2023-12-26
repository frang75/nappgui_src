#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------
# SDK version
# This file is part of NAppGUI-SDK project
# See README.txt and LICENSE.txt

file(READ ${NAPPGUI_ROOT_PATH}/prj/version.txt VERSION)
string(REPLACE "." ";" VERSION ${VERSION})
list(GET VERSION 0 NAPPGUI_MAJOR)
list(GET VERSION 1 NAPPGUI_MINOR)
list(GET VERSION 2 NAPPGUI_PATCH)

# Build version
#------------------------------------------------------------------------------
# This is a 'Release' distribution
set (VERSION_FILE ${NAPPGUI_ROOT_PATH}/prj/build.txt)
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
