# The CaSSiS command line interface tool project.
project(cassis-cli)

# Define a 'dump statistical information' flag.
# Disabled by default.
set(DUMP_STATS "off" CACHE BOOL "Dump statistical information")
if(DUMP_STATS)
    add_definitions(-DDUMP_STATS)
endif(DUMP_STATS)

# List of source files for the CaSSiS tool.
set(cassis_sources
    file/csv.cpp
    file/fasta.cpp
    file/newick.cpp
    file/sigfile.cpp
    gen-signatures.cpp
    main.cpp
    parameters.cpp
)

# Various includes.
include_directories(
    "${PROJECT_SOURCE_DIR}/file"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}"
)

# Build the CaSSiS binary.
add_executable(cassis ${cassis_sources})
if(ARB)
    # Build CaSSiS with ARB file support. 
    add_definitions(-DARB)
    target_link_libraries(cassis CaSSiS minipt ARB)
else(ARB)
    target_link_libraries(cassis CaSSiS minipt)
endif(ARB)

# Install the CaSSiS binary.
install(TARGETS cassis DESTINATION bin)
