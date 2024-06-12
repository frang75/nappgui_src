#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

# CMake Visual Studio additional Warnings
# This file is part of NAppGUI-SDK project
# See README.txt and LICENSE.txt

# Visual Studio Additional Warnings: Extracted from here:
# https://msdn.microsoft.com/en-us/library/23k5d385(v=vs.140).aspx
#
# 'Manual' activation of compiler warning disabled by default in -W4
#  We don't use -Wall for compile problems in VS2017
# https://developercommunity.visualstudio.com/content/problem/174539/objbaseh-error-when-compiling-with-permissive-and.html
#
#------------------------------------------------------------------------------

macro(removeFlag list flag)
    string(REGEX REPLACE "${flag}" "" ${list} ${${list}})
endmacro()

#------------------------------------------------------------------------------

macro(addFlag list flag)
    set("${list}" "${${list}} ${flag}")
endmacro()

#------------------------------------------------------------------------------
macro(nap_msvc_flags runtimeLib)

    # Disable C++ Exceptions
    removeFlag(CMAKE_CXX_FLAGS "/EHsc")

    # C Runtime library (static/dynamic)
    set(RTFLAGS "/MDd;/MTd;/MD;/MT")
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} config)

        # Remove previous C Runtime library configs
        foreach(flag ${RTFLAGS})
            removeFlag(CMAKE_CXX_FLAGS_${config} "${flag}")
            removeFlag(CMAKE_C_FLAGS_${config} "${flag}")
        endforeach()

        if (${runtimeLib} STREQUAL "static")
            if (${config} STREQUAL "DEBUG")
                addFlag(CMAKE_CXX_FLAGS_${config} "/MTd")
                addFlag(CMAKE_C_FLAGS_${config} "/MTd")
            else()
                addFlag(CMAKE_CXX_FLAGS_${config} "/MT")
                addFlag(CMAKE_C_FLAGS_${config} "/MT")
            endif()
        elseif (${runtimeLib} STREQUAL "dynamic")
            if (${config} STREQUAL "DEBUG")
                addFlag(CMAKE_CXX_FLAGS_${config} "/MDd")
                addFlag(CMAKE_C_FLAGS_${config} "/MDd")
            else()
                addFlag(CMAKE_CXX_FLAGS_${config} "/MD")
                addFlag(CMAKE_C_FLAGS_${config} "/MD")
            endif()
        else()
            message(FATAL_ERROR "Unknown runtime library ${runtimeLib}")
        endif()

    endforeach()

endmacro()

#------------------------------------------------------------------------------
macro(nap_msvc_warnings warningList)
set(${warningList}
    # Warning level
    "/W4"
    "/wd4530"   # Disable C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
    "/wd4514"   # Disable 'function' : unreferenced inline function has been removed
    "/wd4625"   # Disable 'derived class' : copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted
    "/wd4626"   # Disable 'derived class' : assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted
    "/wd4098"   # Disable linker mixed (static/dynamic) runtime library warnings
    "/wd4099"   # Disable linker "pdb" warnings
    "/wd4577"   # 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed. Specify /EHsc

    # Additional warning over W4
    # (level 4) enumerator 'identifier' in a switch of enum 'enumeration' is not explicitly handled by a case label.
    "/w44061"

    # (level 4) enumerator 'identifier' in a switch of enum 'enumeration' is not handled
    "/w44062"

    # (level 3) 'operator/operation': unsafe conversion from 'type of expression' to 'type required'
    "/w44191"

    # (level 4) 'identifier': conversion from 'type1' to 'type2', possible loss of data
    "/w44242"

    # (level 4) 'operator': conversion from 'type1' to 'type2', possible loss of data
    "/w44254"

    # (level 4) 'function': no function prototype given: converting '()' to '(void)'
    "/w44255"

    # (level 4) 'function': member function does not override any base class virtual member function
    "/w44263"

    # (level 1) 'virtual_function': no override available for virtual member function from base 'class'; function is hidden
    "/w44264"

    # (level 3) 'class': class has virtual functions, but destructor is not virtual
    "/w44265"

    # (level 4) 'function': no override available for virtual member function from base 'type'; function is hidden
    "/w44266"

    # (level 3) 'operator': unsigned/negative constant mismatch
    "/w44287"

    # (level 4) nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
    "/w44289"

    # (level 4) 'operator': expression is always false
    "/w44296"

    # (level 2) 'conversion': truncation from 'type1' to 'type2'
    "/w44302"

    # (level 1) 'variable' : pointer truncation from 'type' to 'type'
    "/w44311"

    # (level 1) 'operation' : conversion from 'type1' to 'type2' of greater size
    "/w44312"

    # (level 4) 'type' : use of undefined type detected in CLR meta-data - use of this type may lead to a runtime exception
    "/w44339"

    # (level 1) behavior change: 'function' called, but a member operator was called in previous versions
    "/w44342"

    # (level 1) behavior change: 'member1' called instead of 'member2'
    "/w44350"

    # 'this' : used in base member initializer list
    "/w44355"

    # (level 4) 'action': conversion from 'type_1' to 'type_2', signed/unsigned mismatch
    "/w44365"

    # (level 3) layout of class has changed from a previous version of the compiler due to better packing
    "/w44370"

    # (level 3) layout of class may have changed from a previous version of the compiler due to better packing of member 'member'
    "/w44371"

    # (level 4) signed/unsigned mismatch
    "/w44388"

    # (level 2) 'function': function signature contains type 'type'; C++ objects are unsafe to pass between pure code and mixed or native
    "/w44412"

    # (level 4) missing type specifier - int assumed. Note: C no longer supports default-int
    "/w44431"

    # (level 4) 'class1' : Object layout under /vd2 will change due to virtual base 'class2'
    "/w44435"

    # (level 4) dynamic_cast from virtual base 'class1' to 'class2' could fail in some contexts
    "/w44437"

    # (level 3) top level '__unaligned' is not implemented in this context
    "/w44444"

    # (level 4) a forward declaration of an unscoped enumeration must have an underlying type (int assumed)
    "/w44471"

    # (level 1) 'identifier' is a native enum: add an access specifier (private/public) to declare a managed enum
    "/w44472"

    # (level 4) 'function': unreferenced inline function has been removed
    "/w44514"

    # (level 4) 'type name': type-name exceeds meta-data limit of 'limit' characters
    "/w44536"

    # (level 1) expression before comma evaluates to a function which is missing an argument list
    "/w44545"

    # (level 1) function call before comma missing argument list
    "/w44546"

    # (level 1) 'operator': operator before comma has no effect; expected operator with side-effect
    "/w44547"

    # (level 1) expression before comma has no effect; expected expression with side-effect
    "/w44548"

    # (level 1) 'operator': operator before comma has no effect; did you intend 'operator'?
    "/w44549"

    # (level 1) expression has no effect; expected expression with side-effect
    "/w44555"

    # (level 3) '__assume' contains effect 'effect'
    "/w44557"

    # (level 4) informational: catch(�) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
    "/w44571"

    # (level 4) 'identifier' is defined to be '0': did you mean to use '#if identifier'?
    "/w44574"

    # (level 3) 'symbol1' has already been initialized by another union member in the initializer list, 'symbol2'
    "/w44608"

    # (level 3) #pragma warning: there is no warning number 'number'
    "/w44619"

    # (level 4) 'derived class': default constructor could not be generated because a base class default constructor is inaccessible
    "/w44623"

    # (level 4) 'derived class': copy constructor could not be generated because a base class copy constructor is inaccessible
    "/w44625"

    # (level 4) 'derived class': assignment operator could not be generated because a base class assignment operator is inaccessible
    "/w44626"

    # (level 1) digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'
    "/w44628"

    # (level 3) 'instance': construction of local static object is not thread-safe
    "/w44640"

    # (level 4) 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
    "/w44668"

    # (level 4) 'symbol' : no directional parameter attribute specified, defaulting to [in]
    "/w44682"

    # (level 3) 'user-defined type': possible change in behavior, change in UDT return calling convention
    "/w44686"

    # (level 1) 'function': signature of non-private member contains assembly private native type 'native_type'
    "/w44692"

    # (level 4) 'function': function not inlined
    #"/w44710"

    # (level 3) storing 32-bit float result in memory, possible loss of performance
    "/w44738"

    # (level 4) section name 'symbol' is longer than 8 characters and will be truncated by the linker
    "/w44767"

    # (level 3) 'symbol' : object name was truncated to 'number' characters in the debug information
    "/w44786"

    # We consider it as Compiler optimization
    # (level 4) 'bytes' bytes padding added after construct 'member_name'
    # "/w44820"

    # (level 2) conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior
    "/w44826"

    # (level 4) trigraph detected: '??%c' replaced by '%c'
    "/w44837"

    # (level 1) wide string literal cast to 'LPSTR'
    "/w44905"

    # (level 1) string literal cast to 'LPWSTR'
    "/w44906"

    # (level 1) 'declarator': a GUID can only be associated with a class, interface, or namespace
    "/w44917"

    # (level 1) illegal copy-initialization; more than one user-defined conversion has been implicitly applied
    "/w44928"

    # (level 4) we are assuming the type library was built for number-bit pointers
    "/w44931"

    # (level 1) reinterpret_cast used between related classes: 'class1' and 'class2'
    "/w44946"

    # 'function': profile-guided optimizations disabled because optimizations caused profile data to become inconsistent
    "/w44962"

    # (level 4) 'symbol': exception specification does not match previous declaration
    "/w44986"

    # (level 4) nonstandard extension used: 'throw (...)'
    "/w44987"

    # (level 4) 'symbol': variable declared outside class/function scope
    "/w44988"
)
endmacro()

#------------------------------------------------------------------------------

macro(nap_msvc_toolset)

    # DO Nothing
    # CMake 'MSVC_VERSION' is equivalent to '_MSC_VER' macro defined by MSVC compilers

    # MSVC_VERSION (_MSC_VER)
    # 1936 = 17.6 Visual Studio 2022 (v143 toolset)
    # 1935 = 17.5 Visual Studio 2022 (v143 toolset)
    # 1934 = 17.4 Visual Studio 2022 (v143 toolset)
    # 1933 = 17.3 Visual Studio 2022 (v143 toolset)
    # 1932 = 17.2 Visual Studio 2022 (v143 toolset)
    # 1930 = 17.0 Visual Studio 2022 (v143 toolset)

    # 1929 = 16.10/11 Visual Studio 2019 (v142 toolset)
    # 1928 = 16.8/9 Visual Studio 2019 (v142 toolset)
    # 1927 = 16.7 Visual Studio 2019 (v142 toolset)
    # 1926 = 16.6 Visual Studio 2019 (v142 toolset)
    # 1925 = 16.5 Visual Studio 2019 (v142 toolset)
    # 1924 = 16.4 Visual Studio 2019 (v142 toolset)
    # 1923 = 16.3 Visual Studio 2019 (v142 toolset)
    # 1922 = 16.2 Visual Studio 2019 (v142 toolset)
    # 1921 = 16.1 Visual Studio 2019 (v142 toolset)
    # 1920 = 16.0 Visual Studio 2019 (v142 toolset)

    # 1916 = 15.9 Visual Studio 2017 (v141 toolset)
    # 1915 = 15.8 Visual Studio 2017 (v141 toolset)
    # 1914 = 15.7 Visual Studio 2017 (v141 toolset)
    # 1913 = 15.6 Visual Studio 2017 (v141 toolset)
    # 1912 = 15.5 Visual Studio 2017 (v141 toolset)
    # 1911 = 15.3 Visual Studio 2017 (v141 toolset)
    # 1910 = 15.0 Visual Studio 2017 (v141 toolset)

    # 1900 = 14.0 Visual Studio 2015 (v140 toolset)
    # 1800 = 12.0 Visual Studio 2013 (v120 toolset)
    # 1700 = 11.0 Visual Studio 2012 (v110 toolset)
    # 1600 = 10.0 Visual Studio 2010 (v100 toolset)
    # 1500 = 9.0 Visual Studio 2008 (v90 toolset)
    # 1400 = 8.0 Visual Studio 2005 (v80 toolset)
    # 1310 = 7.1 Visual Studio .NET 2003
    # 1300 = 7.0 Visual Studio .NET 2002
    # 1200 = 6.0 Visual Studio 6.0

    #
    # CMake Visual Studio 2015 Problem in Windows 10
    # https://gitlab.kitware.com/cmake/cmake/issues/16713
    # For solving (cmake-gui)
    # 1) Delete CMake Cache
    # 2) Push [Add Entry] Name=CMAKE_SYSTEM_VERSION ; Type=STRING ; Value=8.1
    # 3) [Configure] (Visual Studio 14 2015) Generator
    # 4) [Generate]
    # 5) That's all!
    #
    # For solving in command line
    # cmake -G "Visual Studio 14" -DCMAKE_SYSTEM_VERSION=8.1
    #
    if(MSVC_VERSION GREATER 1929)
        set(CMAKE_VS_PLATFORM_TOOLSET "v143")

    elseif(MSVC_VERSION GREATER  1919)
        set(CMAKE_VS_PLATFORM_TOOLSET "v142")

    elseif(MSVC_VERSION GREATER 1909)
        set(CMAKE_VS_PLATFORM_TOOLSET "v141")

    elseif(MSVC_VERSION GREATER 1899)
        set(CMAKE_VS_PLATFORM_TOOLSET "v140")

    elseif(MSVC_VERSION GREATER 1799)
        set(CMAKE_VS_PLATFORM_TOOLSET "v120")

    elseif(MSVC_VERSION GREATER 1699)
        set(CMAKE_VS_PLATFORM_TOOLSET "v110")

    elseif(MSVC_VERSION GREATER 1599)
        set(CMAKE_VS_PLATFORM_TOOLSET "v100")

    elseif(MSVC_VERSION GREATER 1499)
        set(CMAKE_VS_PLATFORM_TOOLSET "v90")

    elseif(MSVC_VERSION GREATER 1399)
        set(CMAKE_VS_PLATFORM_TOOLSET "v80")

    elseif(MSVC_VERSION EQUAL 1004)
        set(CMAKE_VS_PLATFORM_TOOLSET "v141_xp")

    elseif(MSVC_VERSION EQUAL 1003)
        set(CMAKE_VS_PLATFORM_TOOLSET "v140_xp")

    elseif(MSVC_VERSION EQUAL 1002)
        set(CMAKE_VS_PLATFORM_TOOLSET "v120_xp")

    elseif(MSVC_VERSION EQUAL 1001)
        set(CMAKE_VS_PLATFORM_TOOLSET "v110_xp")

    else()
        message(FATAL_ERROR "Unknows VS Platform Toolset from MSVC_VERSION")

    endif()

endmacro()

