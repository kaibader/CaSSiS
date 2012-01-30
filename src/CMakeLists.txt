cmake_minimum_required(VERSION 2.6)

SET(CMAKE_INSTALL_RPATH ".")

# Prevent in-source builds. Always build in a separate folder.
if(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Sorry, but in-source builds are disabled."
        "Please use a separate build directory.")
endif(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_BINARY_DIR}")

# The build type (debug, release, ...) has to be defined.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
        "Choose the type of build. Options: Debug Release RelWithDebInfo MinSizeRel"
    FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# The ARB database and search index library.
# Disabled by default. Always disabled in Windows builds.
# Caution: Should only be enabled if an ARBHOME variable is defined as well.
set(ARB "off" CACHE BOOL "Build with ARB support")
if(WIN32)
    set(ARB "off")
endif(WIN32)
if(ARB)
    add_subdirectory(lib/arb)
endif(ARB)

# The version number. Will be stored in "config.h".
# All tools share a version number with the CaSSiS library.
set(CASSIS_VERSION_MAJOR  0)
set(CASSIS_VERSION_MINOR  4)
set(CASSIS_VERSION_PATCH  1)
set(CASSIS_VERSION_SUFFIX "_alpha")
message(STATUS "Setting CaSSiS version number to: "
    "${CASSIS_VERSION_MAJOR}.${CASSIS_VERSION_MINOR}.${CASSIS_VERSION_PATCH}${CASSIS_VERSION_SUFFIX}")

# Build date macro (thanks @ Benoit_RAT).
macro(TODAY RESULT)
    if(WIN32)
        execute_process(COMMAND cmd /c date /t OUTPUT_VARIABLE DATE)
        string(REGEX REPLACE "(..).(..)...(..).*" "\\3\\2\\1" ${RESULT} "${DATE}")
    elseif(UNIX)
        execute_process(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE DATE)
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1" ${RESULT} "${DATE}")
    else(WIN32)
        message(SEND_ERROR "date not implemented")
        set(${RESULT} 000000)
    endif(WIN32)
endmacro(TODAY)

# Sets the build date string in "config.h".
TODAY(CASSIS_BUILD_DATE)
message(STATUS "Setting CaSSiS build date to: ${CASSIS_BUILD_DATE}")

# Compiler definitions (for debugging purposes only).
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-g -O0 -Wall -Wextra -Wshadow)
    #add_definitions(-g -O0 -Wall -Weffc++ -pedantic -pedantic-errors -Wextra
    #    -Wall -Wcast-align -Wcast-qual -Wchar-subscripts -Wcomment
    #    -Wdisabled-optimization -Werror -Wfloat-equal -Wformat -Wformat=2
    #    -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimplicit -Wimport
    #    -Winit-self -Winline -Winvalid-pch -Wunsafe-loop-optimizations
    #    -Wlong-long -Wmissing-braces -Wmissing-field-initializers
    #    -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn
    #    -Wpacked -Wparentheses -Wpointer-arith -Wredundant-decls -Wreturn-type
    #    -Wsequence-point -Wshadow -Wsign-compare -Wstack-protector
    #    -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch -Wswitch-default
    #    -Wswitch-enum -Wtrigraphs -Wuninitialized -Wunknown-pragmas
    #    -Wunreachable-code -Wunused -Wunused-function -Wunused-label
    #    -Wunused-parameter -Wunused-value -Wunused-variable -Wvariadic-macros
    #    -Wvolatile-register-var -Wwrite-strings)
    #    # -Wpadded -Wconversion
    message(STATUS "Enabled pedantic gcc warnings")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

# Disable various warnings in MSVC.
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif(WIN32)

# The MiniPT search index library.
add_subdirectory(lib/minipt)

# The CaSSiS library.
add_subdirectory(lib/cassis)

# The CaSSiS command line interface tool.
add_subdirectory(main/cassis)

# The graphical user interface version of CaSSiS.
add_subdirectory(main/cassis-gui)

# Various tools (useful for debugging and testing).
add_subdirectory(main/tools)

# Build with DUP multi-pipeline support.
# Disabled by default. Always disabled in Windows builds.
set(DUP "off" CACHE BOOL "Build with DUP search index support")
if(WIN32)
    set(DUP "off")
endif(WIN32)
if(DUP)
    #add_subdirectory(main/dup-index)
endif(DUP)

# Add a 'create package' functionality.
include(package.cmake)
