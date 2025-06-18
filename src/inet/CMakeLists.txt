nap_library(inet "core" "${NAPPGUI_SHARED}" NRC_NONE)
if (${NAPPGUI_SHARED})
    nap_link_inet_depends(inet)
endif()
