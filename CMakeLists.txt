cmake_minimum_required(VERSION 2.6)

SET(CMAKE_INSTALL_RPATH ".")

# Prevent in-source builds. Always build in a separate folder.
if(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_BINARY_DIR}")

# The build type (debug, release, ...) has to be defined.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
        "Choose the type of build. Options: Debug Release RelWithDebInfo MinSizeRel"
    FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Define an ARB flag.
# Disabled by default; Always disabled in Windows builds.
# Caution: Should only be enabled if an ARBHOME is defined.
set(ARB "off" CACHE BOOL "Build with ARB support")
if(WIN32)
set(ARB "off")
endif(WIN32)
if(ARB)
    add_subdirectory(3rd-party/arb)
endif(ARB)

# The version number. Will be written to "bgrt/config.h".
# All tools share a version number with the BGRT library.
set(CASSIS_VERSION_MAJOR  0)
set(CASSIS_VERSION_MINOR  4)
set(CASSIS_VERSION_PATCH  0)
set(CASSIS_VERSION_SUFFIX "")
message(STATUS "Setting CaSSiS version number to: "
    "${CASSIS_VERSION_MAJOR}.${CASSIS_VERSION_MINOR}.${CASSIS_VERSION_PATCH}${CASSIS_VERSION_SUFFIX}")

# Build date macro (thanks @ Benoit RAT).
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

# Set the date string. See: library/config.h
TODAY(CASSIS_BUILD_DATE)
message(STATUS "Setting CaSSiS build date to: ${CASSIS_BUILD_DATE}")

# For debugging purposes only...
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

# Disable various warnings in MSVC...
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif(WIN32)

# The MiniPT search index library
add_subdirectory(minipt)

# The CaSSiS library
add_subdirectory(cassis)

# The CaSSiS command line interface tool
add_subdirectory(cassis-cli)

# The graphical user interface version of CaSSiS
add_subdirectory(cassis-gui)

# Various tools (useful for debugging and testing)
add_subdirectory(tools)

# Add 'create package' functionality
include(package.cmake)
