#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

#
# From https://www.iso-9899.info/wiki/WarningFlags
#
macro(nap_gcc_warns)
    set(FLAGS "")
    set(CFLAGS "")
    set(CXXFLAGS "")

    # Warn about non-ISO looking code, stricter checking for conformance testing
    # with one of the ANSI/ISO std options above.  Issue all warnings demanded by
    # strict ISO C for the appropriate standard.  Turns off some gcc extensions
    # that are left on without it.
    set(FLAGS "${FLAGS} -pedantic")

    # Misnamed, enables a list of warning options, but not all of them.
    set(FLAGS "${FLAGS} -Wall")

    # Warn if an array subscript has type "char" (which can be signed)
    # set(FLAGS "${FLAGS} -Wchar-subscripts")   # included in -Wall

    # Warn if a comment sequence /* appears in another /* comment or a
    # backslash-newline appears in a // comment.
    # set(FLAGS "${FLAGS} -Wcomment")           # included in -Wall

    # Check calls to printf, scanf, etc to make sure args have proper types.
    # set(FLAGS "${FLAGS} -Wformat")            # included in -Wall

    # set(FLAGS "${FLAGS} -Wnonnull")           # included in -Wall
    # set(FLAGS "${FLAGS} -Wimplicit-int")      # included in -Wall
    # set(FLAGS "${FLAGS} -Wimplicit-function-declaration") # included in -Wall
    # set(FLAGS "${FLAGS} -Wimplicit")          # included in -Wall
    # set(FLAGS "${FLAGS} -Wmain")              # included in -Wall
    # set(FLAGS "${FLAGS} -Wmissing-braces")    # included in -Wall
    # set(FLAGS "${FLAGS} -Wparentheses")       # included in -Wall
    # set(FLAGS "${FLAGS} -Wsequence-point")    # included in -Wall
    # set(FLAGS "${FLAGS} -Wreturn-type")       # included in -Wall
    # set(FLAGS "${FLAGS} -Wswitch")            # included in -Wall
    # set(FLAGS "${FLAGS} -Wtrigraphs")         # included in -Wall
    # set(FLAGS "${FLAGS} -Wunknown-pragmas")   # included in -Wall
    set(FLAGS "${FLAGS} -Wunused-function")     # included in -Wall
    # set(FLAGS "${FLAGS} -Wunused-label")      # included in -Wall
    set(FLAGS "${FLAGS} -Wunused-variable")     # included in -Wall
    set(FLAGS "${FLAGS} -Wunused-value")        # included in -Wall
    set(FLAGS "${FLAGS} -Wuninitialized")       # included in -Wall
    # set(FLAGS "${FLAGS} -Wstrict-aliasing")   # included in -Wall if -fstrict-aliasing

    # Warn if a user-supplied include directory does not exist.
    set(FLAGS "${FLAGS} -Wmissing-include-dirs")

    # Warn whenever a switch statement does not have a default case.
    # Unfortunately this includes switching on an enum where all cases
    # are covered.
    set(FLAGS "${FLAGS} -Wswitch-default")

    # Warn when a switch statement has an index of enumerate type and lacks a case
    # for one or more of the named codes in the enumeration, or case labels outside
    # the enumeration range.
    set(FLAGS "${FLAGS} -Wswitch-enum")

    # Warn whenever a function parameter is unused aside from its declaration,
    # unless the "unused" attribute is used.
    set(FLAGS "${FLAGS} -Wunused-parameter")

    # Print extra warning information.  This is equivalent to -W, may need to use
    #    -W with older versions of gcc
    set(FLAGS "${FLAGS} -Wextra")

    # set(FLAGS "${FLAGS} -Wmissing-field-initializers")  # included in -Wextra

    # Warn about comparison between signed and unsigned values that can produce
    # an incorrect result when the signed value is converted to unsigned.
    # set(FLAGS "${FLAGS} -Wsign-compare")                # in -Wextra, without -Wno-sign-compare

    # warn about constructs that behave differently in traditional and ISO C, or
    # those with no traditional C equivalent, or problematic ones to be avoided.
    # set(FLAGS "${FLAGS} -Wtraditional")

    # Warn for missing prototypes.
    set(CFLAGS "${CFLAGS} -Wmissing-prototypes")

    # Warn if a function is declared or defined w/o specifying the argument types.
    set(CFLAGS "${CFLAGS} -Wstrict-prototypes")

    # Warn for a global function defined without a previous prototype.  Issued even
    # if the definition itself provides a prototype.
    set(CFLAGS "${CFLAGS} -Wmissing-declarations")

    # Warn when a variable is shadowed, or a built-in function is shadowed.
    set(FLAGS "${FLAGS} -Wshadow")

    # Warn when an object larger than *len* bytes is defined.
    # set(FLAGS "${FLAGS} -Wlarger-than-1024")

    # Allocate even uninitialized global variables in the data section of the
    # object file, rather than generating them as common blocks.  If the same
    # variable is declared without "extern" in two different compilations, you
    # will get an error when linked.
    # set(FLAGS "${FLAGS} -fno-common")

    # Do not store floating point variables in registers and inhibit other options
    # that might change whether a floating point value is taken from a register or
    # memory.  Useful primarily for programs that rely in a precise definition of
    # IEEE floating point.
    # set(FLAGS "${FLAGS} -ffloat-store")

    # Turns on -fno-math-errno, -funsafe-math-optimizations, -fno-trapping-math,
    # -ffinite-math-only, -fno-rounding-math, -fno-signaling-nans and
    # fcx-limited-range.  Should never be turned on with optimizations, since it
    # can result in incorrect output for programs that depend upon an exact
    # implementation of IEEE or ISO math function behavior.
    # set(FLAGS "${FLAGS} -ffast-math")

    # Warn if any function that return structures or unions are defined or called.
    set(FLAGS "${FLAGS} -Waggregate-return")

    # Targetted primarily for interfacing with code that doesn't use prototypes;
    # otherwise pretty noisy.  Also warn about "x = -1;" is x is unsigned.
    set(FLAGS "${FLAGS} -Wconversion")

    # Warn for floating point equality expressions.
    set(FLAGS "${FLAGS} -Wfloat-equal")

    # Warn for long long usage.  (only if -pedantic is used)
    set(FLAGS "${FLAGS} -Wlong-long")

    # Warn for multiple declarations for the same thing in the same scope.
    set(FLAGS "${FLAGS} -Wredundant-decls")

    # Warn for function call cast to non-matching type.
    set(CFLAGS "${CFLAGS} -Wbad-function-cast")

    # Warn when a pointer is cast to remove a type qualifier, ex: warn if
    # "const char *" is cast to an ordinary "char *".
    set(FLAGS "${FLAGS} -Wcast-qual")

    # Warn for casts which increase alignment constraints, such as "char *" cast
    # to "int *" on machines where unaligned accesses are not alowed.
    set(FLAGS "${FLAGS} -Wcast-align")

    # Make string literals const.
    set(FLAGS "${FLAGS} -Wwrite-strings")

    # Warn for undefined identifiers in #if directives.
    set(FLAGS "${FLAGS} -Wundef")

    # Warn for text at end of preprocessor directive.
    set(FLAGS "${FLAGS} -Wendif-labels")

    # Warn about anything that depends on the size of a function type or a "void".
    set(FLAGS "${FLAGS} -Wpointer-arith")

    # Warn if function declared as inline can not be inlined.
    set(FLAGS "${FLAGS} -Winline")

    # Warn if a requested optimization pass is disabled, meaning the gcc optimizer
    # is not able to handle the code.  The code may be too big or complex.
    set(FLAGS "${FLAGS} -Wdisabled-optimization")

    # after a statement in a block, i.e C99
    set(CFLAGS "${CFLAGS} -Wdeclaration-after-statement")

    # warn if a 64-bit value is implicitly converted from a 64- to a 32-bit type.
    # Apparently only on Apple
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set(FLAGS "${FLAGS} -Wshorten-64-to-32")
    endif()

    # Warn if an old-style function definition is used, even with a valid prototype.
    set(CFLAGS "${CFLAGS} -Wold-style-definition")

    # Warn about functions which might be candidates for attribute "noreturn".
    set(FLAGS "${FLAGS} -Wmissing-noreturn")

    # warn of user-supplied include dirs that are missing
    set(FLAGS "${FLAGS} -Wmissing-include-dirs")

    # warn about strftime formats which yield a 2-digit year
    set(FLAGS "${FLAGS} -Wformat-y2k")    # if -Wformat is also specified only

    # if -Wformat is enabled, warn if the format string is not a string literal and
    # cannot be checked, unless it takes its format arguments as a "va_list"
    set(FLAGS "${FLAGS} -Wformat-nonliteral") # format string can not be checked

    # format security problems get flagged, like printf(foo);
    set(FLAGS "${FLAGS} -Wformat-security")

    # Warn the first time #import is used.
    set(FLAGS "${FLAGS} -Wimport")

    # Warn about uninitialized variables that are initialized with themselves.
    # Examples: int i = i;
    # Requires -Wuninitialized and -O1 or above.
    set(FLAGS "${FLAGS} -Winit-self")

    # warn if precompiled header is found but can't be used
    set(FLAGS "${FLAGS} -Winvalid-pch")

    set(FLAGS "${FLAGS} -Wpacked") # warn if packed attibute has no effect

    set(FLAGS "${FLAGS} -Wpadded")  # warn if padding is included in a structure

    # This will warn about unreachable code.
    set(FLAGS "${FLAGS} -Wunreachable-code")

    # Disable "asm", "inline" or "typeof" as keywords.  -ansi implies this option.
    # With C99, "inline" is still allowed.
    # CFLAGS += -fno-asm

    # Don't use special code for certain "built in" functions.  For use of some
    # LD_PRELOAD tricks this might be necessary.
    # CFLAGS += -fno-builtin

    # Warn if an "extern" declaration is encountered within a function.
    set(CFLAGS "${CFLAGS} -Wnested-externs")

    # Warn about four character constants, 'APPL'.
    # Apparently only on Apple
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set(FLAGS "${FLAGS} -Wfour-char-constants")
    endif()
    # CFLAGS += -Wno-multichar   # do not warn if a multichar constant 'FOO' is used

    # ISO C identifier normalization options.  See man page.
    # CFLAGS += -Wnormalized=<none|id|nfc|nfkc>

    # Generate code to verify you do not go beyond the stack boundary.  Useful
    # primarily for threaded (PTHREAD) code.  This does /not/ cause stack checking
    # to be done; the operating system must do that.  Generates code to ensure that
    # the OS sees the stack being extended.
    # This causes the compiler to write a byte to each page of the local variable
    # area on entry to each function, thus removing any chance that the overflow will
    # miss the guard page.  Has been reported buggy in some implementations,
    # particularly with stack usage inside main().
    # See: http://www.kegel.com/stackcheck/demo.c
    # CFLAGS += -fstack-check
    # See also -fstack-limit-symbol, -fstack-limit-register

    # CFLAGS += -fstack-protector  # required for the -Wstack-protector
    # Warn about functions that will not be protected against stack smashing
    set(FLAGS "${FLAGS} -Wstack-protector")

    # Disable flags
    # In future try to adapt NAppGUI to remove these flags, making the code more robust
    set(CFLAGS "${CFLAGS} -Wno-bad-function-cast -Wno-missing-prototypes -Wno-missing-declarations")
    set(FLAGS "${FLAGS} -Wno-long-long -Wno-overlength-strings -Wno-aggregate-return -Wno-cast-qual -Wno-padded -Wno-switch-default -Wno-conversion -Wno-float-equal -Wno-format-nonliteral -Wno-switch-enum -Wno-redundant-decls -Wno-shadow -Wno-undef -Wno-missing-noreturn -Wno-stack-protector -Wno-missing-include-dirs -Wno-cast-align")
    set(CXXFLAGS "${CXXFLAGS} -Wnon-virtual-dtor -Woverloaded-virtual")

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set(FLAGS "${FLAGS} -Wno-unreachable-code")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS} ${CFLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS} ${CXXFLAGS}")

endmacro()

macro(nap_gcc_flags arch)

    # Warnings nightmare..
    nap_gcc_warns()

    # Force ANSI-C for old GCC / old CMake
    if(${CMAKE_VERSION} VERSION_LESS "3.1.0" OR ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "5.0.0")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu89")
    endif()

    # Architecture
    if (arch)
        if (arch STREQUAL "x86")
            set(ARCH_FLAG "-m32")

        elseif (arch STREQUAL "x64")
            set(ARCH_FLAG "-m64")

        elseif (arch STREQUAL "arm64")
            # Ok!

        elseif (arch STREQUAL "arm")
            set(ARCH_FLAG "-march=armv7-a")

        endif()
    endif()

    if (ARCH_FLAG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH_FLAG}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAG}")
    endif()

endmacro()
