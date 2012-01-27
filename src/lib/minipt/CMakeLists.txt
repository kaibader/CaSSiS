project(libminipt)

# List of source files for the minipt library.
set(minipt_sources
    buildtree.cpp
    findex.cpp
    io.cpp
    match.cpp
    minipt.cpp
    prefixtree.cpp
    strbuf.cpp
)

include_directories(
    "${CMAKE_SOURCE_DIR}/lib"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
)

# Legacy!
# DEBUG/NDEBUG needs to be defined.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND minipt_compile_definitions "DEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    list(APPEND minipt_compile_definitions "DEBUG")
else()
    list(APPEND minipt_compile_definitions "NDEBUG")
endif()

# Legacy!
# Add a flag (needed by ARB) when building in a 64 bit environment.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    list(APPEND minipt_compile_definitions "ARB_64")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

set_source_files_properties(${minipt_sources}
    PROPERTIES COMPILE_DEFINITIONS "${minipt_compile_definitions}")

# Build the minipt library.
if(WIN32)
	add_library(minipt STATIC ${minipt_sources})
	target_link_libraries(minipt CaSSiS)
else(WIN32)
	add_library(minipt SHARED ${minipt_sources})
	target_link_libraries(minipt CaSSiS)
endif(WIN32)

# Install.
install(TARGETS minipt DESTINATION lib)
