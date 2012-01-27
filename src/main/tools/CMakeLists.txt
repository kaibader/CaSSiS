project(bgrttools)

include_directories(
    "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_BINARY_DIR}/lib"
    "${CMAKE_SOURCE_DIR}/shared"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
)

### Tool: bgrt2graphviz ###

set(bgrt2graphviz_sources
    bgrt2graphviz.cpp
)
add_executable(bgrt2graphviz ${bgrt2graphviz_sources})
target_link_libraries(bgrt2graphviz CaSSiS)
install(TARGETS bgrt2graphviz DESTINATION bin)

### Tool: bgrtmerge ###

set(bgrtmerge_sources
    merge.cpp
    bgrtmerge.cpp
)
add_executable(bgrtmerge ${bgrtmerge_sources})
target_link_libraries(bgrtmerge CaSSiS)
install(TARGETS bgrtmerge DESTINATION bin)

### Tool: Search Index Daemon ###

set(index-daemon_sources
    "${CMAKE_SOURCE_DIR}/shared/fasta.cpp"
    index-daemon.cpp
)
add_executable(index-daemon ${index-daemon_sources})
if(ARB)
    # Build with ARB file support.
    add_definitions(-DARB)
    target_link_libraries(index-daemon CaSSiS minipt ARB)
else(ARB)
    target_link_libraries(index-daemon CaSSiS minipt)
endif(ARB)
install(TARGETS index-daemon DESTINATION bin)
