project(dup-index)

include_directories(
    "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_BINARY_DIR}/lib"
    "${CMAKE_SOURCE_DIR}/shared"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
)
set(dup-index_sources
    "${CMAKE_SOURCE_DIR}/shared/fasta.cpp"
    dup-index-server.cpp
)
add_executable(dup-index ${dup-index_sources})
if(ARB)
    # Build with ARB file support.
    add_definitions(-DARB)
    target_link_libraries(dup-index CaSSiS minipt ARB)
else(ARB)
    target_link_libraries(dup-index CaSSiS minipt)
endif(ARB)
install(TARGETS dup-index DESTINATION bin)
