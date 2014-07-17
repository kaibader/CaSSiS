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

### Tool: thermodynamics ###

set(thermodynamics_sources
    thermodynamics.cpp
)
add_executable(thermodynamics ${thermodynamics_sources})
target_link_libraries(thermodynamics CaSSiS)
install(TARGETS thermodynamics DESTINATION bin)

### Tool: treetest ###

set(treetest_sources
    "${CMAKE_SOURCE_DIR}/shared/newick.cpp"
    treetest.cpp
)
add_executable(treetest ${treetest_sources})
target_link_libraries(treetest CaSSiS)
install(TARGETS treetest DESTINATION bin)
