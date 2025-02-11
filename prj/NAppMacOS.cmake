#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

# Get macOS name from SDK
# https://ss64.com/osx/sw_vers.html

#
# Values for 'CMAKE_OSX_DEPLOYMENT_TARGET'
# 15.0      # Sequoia
# 14.0      # Sonoma
# 13.0      # Ventura
# 12.0      # Monterey
# 11.0      # Big Sur
# 10.15     # Catalina
# 10.14     # Mojave
# 10.13     # High Sierra
# 10.12     # Sierra
# 10.11     # El Capitan
# 10.10     # Yosemite
# 10.9      # Mavericks
# 10.8      # Mountain Lion
# 10.7      # Lion
# 10.6      # Snow Leopard
#

#------------------------------------------------------------------------------

function(nap_macos_sdk_name sdkVersion _ret)
    # 15 = "Sequoia"
    if (sdkVersion VERSION_GREATER "14.9999")
        set(${_ret} "Sequoia" PARENT_SCOPE)

    # 14 = "Sonoma"
    elseif (sdkVersion VERSION_GREATER "13.9999")
        set(${_ret} "Sonoma" PARENT_SCOPE)

    # 13 = "Ventura"
    elseif (sdkVersion VERSION_GREATER "12.9999")
        set(${_ret} "Ventura" PARENT_SCOPE)

    # 12 = "Monterey"
    elseif (sdkVersion VERSION_GREATER "11.9999")
        set(${_ret} "Monterey" PARENT_SCOPE)

    # 11 = "Big Sur"
    elseif (sdkVersion VERSION_GREATER "10.9999")
        set(${_ret} "Big Sur" PARENT_SCOPE)

    # 10.15 = "Catalina"
    elseif (sdkVersion VERSION_GREATER "10.14.9999")
        set(${_ret} "Catalina" PARENT_SCOPE)

    # 10.14 = "Mojave"
    elseif (${sdkVersion} VERSION_GREATER "10.13.9999")
        set(${_ret} "Mojave" PARENT_SCOPE)

    # 10.13 = "High Sierra"
    elseif (${sdkVersion} VERSION_GREATER "10.12.9999")
        set(${_ret} "High Sierra" PARENT_SCOPE)

    # 10.12 = "Sierra"
    elseif (${sdkVersion} VERSION_GREATER "10.11.9999")
        set(${_ret} "Sierra" PARENT_SCOPE)

    # 10.11 = "El Capitan"
    elseif (${sdkVersion} VERSION_GREATER "10.10.9999")
        set(${_ret} "El Capitan" PARENT_SCOPE)

    # 10.10 = "Yosemite"
    elseif (${sdkVersion} VERSION_GREATER "10.9.9999")
        set(${_ret} "Yosemite" PARENT_SCOPE)

    # 10.9 = "Mavericks"
    elseif (${sdkVersion} VERSION_GREATER "10.8.9999")
        set(${_ret} "Mavericks" PARENT_SCOPE)

    # 10.8 = "Mountian Lion"
    elseif (${sdkVersion} VERSION_GREATER "10.7.9999")
        set(${_ret} "Mountian Lion" PARENT_SCOPE)

    # 10.7 = "Lion"
    elseif (${sdkVersion} VERSION_GREATER "10.6.9999")
        set(${_ret} "Lion" PARENT_SCOPE)

    # 10.6 = "Snow Leopard"
    elseif (${sdkVersion} VERSION_GREATER "10.5.9999")
        set(${_ret} "Snow Leopard" PARENT_SCOPE)

    else()
        message(FATAL_ERROR "Unsupported macOS version: ${sdkVersion}")

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_osx_arch arch _ret)

    if (${arch} STREQUAL "i386")
        set(${_ret} "x86" PARENT_SCOPE)
    elseif (${arch} STREQUAL "x86_64")
        set(${_ret} "x64" PARENT_SCOPE)
    elseif (${arch} STREQUAL "arm64")
        set(${_ret} "arm64" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown macOS arch: ${arch}")
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_osx_build_arch arch _ret)

    if (${arch} STREQUAL "x86")
        set(${_ret} "i386" PARENT_SCOPE)
    elseif (${arch} STREQUAL "x64")
        set(${_ret} "x86_64" PARENT_SCOPE)
    elseif (${arch} STREQUAL "arm64")
        set(${_ret} "arm64" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown macOS build arch: ${arch}")
    endif()

endfunction()
