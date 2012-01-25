project(bgrttools)

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}"
)

### Tool: bgrtinfo ###

set(bgrtinfo_sources
    bgrtinfo.cpp
)
add_executable(bgrtinfo ${bgrtinfo_sources})
target_link_libraries(bgrtinfo CaSSiS)
install(TARGETS bgrtinfo DESTINATION bin)

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

### Tool: Search Index Server ###

set(index-server_sources
    ../cassis-cli/file/fasta.cpp
    index-server.cpp
)
add_executable(index-server ${index-server_sources})
if(ARB)
    # Build with ARB file support.
    add_definitions(-DARB)
    target_link_libraries(index-server CaSSiS minipt ARB)
else(ARB)
    target_link_libraries(index-server CaSSiS minipt)
endif(ARB)
install(TARGETS index-server DESTINATION bin)
