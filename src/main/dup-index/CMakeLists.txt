project(dup-test)

include_directories(
    "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_BINARY_DIR}/lib"
    "${CMAKE_SOURCE_DIR}/shared"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
)

# Program: dup-index-server

set(dup-index-server_sources
    "${CMAKE_SOURCE_DIR}/shared/fasta.cpp"
    dup-io.cpp
    dup-index-server.cpp
)

add_executable(dup-index-server ${dup-index-server_sources})

if(ARB)
    # Build with ARB file support.
    add_definitions(-DARB)
    target_link_libraries(dup-index-server CaSSiS minipt ARB)
else(ARB)
    target_link_libraries(dup-index-server CaSSiS minipt)
endif(ARB)

install(TARGETS dup-index-server DESTINATION bin)

# Program: dup-test

set(dup-test_sources
    "${CMAKE_SOURCE_DIR}/shared/fasta.cpp"
    dup-io.cpp
    dup-test.cpp
)

add_executable(dup-test ${dup-test_sources})

if(ARB)
    # Build with ARB file support.
    add_definitions(-DARB)
    target_link_libraries(dup-test CaSSiS minipt ARB)
else(ARB)
    target_link_libraries(dup-test CaSSiS minipt)
endif(ARB)

install(TARGETS dup-test DESTINATION bin)
