# The CaSSiS command line interface tool project.
project(CaSSiS)

# Define a 'dump statistical information' flag.
# Disabled by default.
set(DUMP_STATS "off" CACHE BOOL "Dump statistical information")
if(DUMP_STATS)
    add_definitions(-DDUMP_STATS)
endif(DUMP_STATS)

# List of source files for the CaSSiS tool.
set(cassis_sources
    "${CMAKE_SOURCE_DIR}/shared/csv.cpp"
    "${CMAKE_SOURCE_DIR}/shared/fasta.cpp"
    "${CMAKE_SOURCE_DIR}/shared/newick.cpp"
    "${CMAKE_SOURCE_DIR}/shared/sigfile.cpp"
    gen-signatures.cpp
    main.cpp
    parameters.cpp
)

# Various includes.
include_directories(
    "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_BINARY_DIR}/lib"
    "${CMAKE_SOURCE_DIR}/shared"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
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
