nap_library(osgui "draw2d" "${NAPPGUI_SHARED}" NRC_NONE)

# Avoid '#include <webkit2/webkit2.h>' C++ comments warnings
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set_source_files_properties("gtk/osweb.c" PROPERTIES COMPILE_FLAGS "-std=gnu99")
endif()
