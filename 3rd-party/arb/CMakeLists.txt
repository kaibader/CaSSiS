project(arb)

# We need a defined ARBHOME (path to the ARB source code).
set(ARBHOME "" CACHE PATH "Path to the ARB source code")
if(NOT ARBHOME)
    message(FATAL_ERROR "ARBHOME variable not defined.  "
        "Please invoke cmake with -DARBHOME=/path/to/arb/directory")
    else(NOT ARBHOME)
        if(IS_DIRECTORY "${ARBHOME}")
        message(STATUS "ARB environment found.")
    else(IS_DIRECTORY "${ARBHOME}")
        message(FATAL_ERROR "ARBHOME points to non-existing directory.")
    endif(IS_DIRECTORY "${ARBHOME}")
endif(NOT ARBHOME)

# List of source files for the ARB database and PT-Server functions.
set(arb_sources
    tree.cpp
    ptserver.cpp
    ${ARBHOME}/ARBDB/adali.cxx
    ${ARBHOME}/ARBDB/adcache.cxx
    ${ARBHOME}/ARBDB/adcomm.cxx
    ${ARBHOME}/ARBDB/adcompr.cxx
    ${ARBHOME}/ARBDB/ad_core.cxx
    ${ARBHOME}/ARBDB/adfile.cxx
    ${ARBHOME}/ARBDB/adGene.cxx
    ${ARBHOME}/ARBDB/adhash.cxx
    ${ARBHOME}/ARBDB/adhashtools.cxx
    ${ARBHOME}/ARBDB/adindex.cxx
    ${ARBHOME}/ARBDB/aditem.cxx
    ${ARBHOME}/ARBDB/adlang1.cxx
    ${ARBHOME}/ARBDB/adlink.cxx
    ${ARBHOME}/ARBDB/ad_load.cxx
    ${ARBHOME}/ARBDB/admalloc.cxx
    ${ARBHOME}/ARBDB/admap.cxx
    ${ARBHOME}/ARBDB/admatch.cxx
    ${ARBHOME}/ARBDB/admath.cxx
    ${ARBHOME}/ARBDB/adoptimize.cxx
    ${ARBHOME}/ARBDB/adquery.cxx
    ${ARBHOME}/ARBDB/adRevCompl.cxx
    ${ARBHOME}/ARBDB/ad_save_load.cxx
    ${ARBHOME}/ARBDB/adseqcompr.cxx
    ${ARBHOME}/ARBDB/adsocket.cxx
    ${ARBHOME}/ARBDB/adstring.cxx
    ${ARBHOME}/ARBDB/adsystem.cxx
    ${ARBHOME}/ARBDB/adtables.cxx
    ${ARBHOME}/ARBDB/adtcp.cxx
    ${ARBHOME}/ARBDB/adTest.cxx
    ${ARBHOME}/ARBDB/adtools.cxx
    ${ARBHOME}/ARBDB/adtree.cxx
    ${ARBHOME}/ARBDB/adtune.cxx
    ${ARBHOME}/ARBDB/arbdb.cxx
    ${ARBHOME}/ARBDB/arbdbpp.cxx
    ${ARBHOME}/CORE/arb_cs.cxx
    ${ARBHOME}/CORE/arb_file.cxx
    ${ARBHOME}/CORE/arb_handlers.cxx
    ${ARBHOME}/CORE/arb_msg.cxx
    ${ARBHOME}/CORE/arb_progress.cxx
    ${ARBHOME}/CORE/arb_signal.cxx
    ${ARBHOME}/CORE/arb_sort.cxx
    ${ARBHOME}/CORE/arb_strarray.cxx
    ${ARBHOME}/CORE/arb_strbuf.cxx
    ${ARBHOME}/CORE/arb_string.cxx
    ${ARBHOME}/PROBE/PT_buildtree.cxx
    ${ARBHOME}/PROBE/PT_etc.cxx
    ${ARBHOME}/PROBE/PT_family.cxx
    ${ARBHOME}/PROBE/PT_findEx.cxx
    ${ARBHOME}/PROBE/PT_io.cxx
    ${ARBHOME}/PROBE/PT_match.cxx
    ${ARBHOME}/PROBE/PT_new_design.cxx
    ${ARBHOME}/PROBE/PT_prefixtree.cxx
    ${ARBHOME}/PROBE/PT_rangeCheck.cxx
    ${ARBHOME}/PROBE_COM/PT_extern.c
    ${ARBHOME}/PROBE_COM/C/aisc_extern.c
    ${ARBHOME}/PROBE_COM/C/server.c
    ${ARBHOME}/PROBE_COM/C/struct_man.c
    ${ARBHOME}/PROBE_COM/GENC/aisc_global.c
    ${ARBHOME}/PROBE_COM/GENC/aisc_server.c
    ${ARBHOME}/SL/HELIX/BI_basepos.cxx
)

# Project includes...
include_directories(
    "${ARBHOME}/ARBDB"
    "${ARBHOME}/CORE"
    "${ARBHOME}/INCLUDE"
    "${ARBHOME}/PROBE"
    "${ARBHOME}/PROBE_COM/C"
    "${ARBHOME}/PROBE_COM/GENH"
    "${ARBHOME}/SL/HELIX"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}"
)

# Some ARB (legacy) dependencies.
set(arb_compile_definitions "FAKE_VTAB_PTR=char")
list(APPEND arb_compile_definitions "LINUX")

# Add a flag (needed by ARB) when building in a 64 bit environment.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    list(APPEND arb_compile_definitions "ARB_64")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

# ARB code needs a defined DEBUG/NDEBUG flag.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND arb_compile_definitions "DEBUG")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    list(APPEND arb_compile_definitions "DEBUG")
endif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

# Set the previously defined properties.
set_source_files_properties(${arb_sources}
    PROPERTIES COMPILE_DEFINITIONS "${arb_compile_definitions}")

# ARB uses the c++ compiler for c files.
set(CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

# Build the ARB database library.
add_library(ARB SHARED ${arb_sources})
target_link_libraries(ARB CaSSiS)

# Install.
install(TARGETS ARB DESTINATION lib)
