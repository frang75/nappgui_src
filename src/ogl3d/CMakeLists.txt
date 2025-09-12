nap_library(ogl3d "sewer" "${NAPPGUI_SHARED}" NRC_NONE)

# Avoid GCC warnings in glew.c
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set_source_files_properties("glew.c" PROPERTIES COMPILE_FLAGS "-std=gnu99 -Wno-strict-prototypes -Wno-old-style-definition -Wno-address -w")
endif()

if (${NAPPGUI_SHARED})
    nap_link_opengl_depends(ogl3d)
endif()

