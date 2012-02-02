project(dup-index)

include_directories(
    "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_BINARY_DIR}/lib"
    "${CMAKE_SOURCE_DIR}/shared"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
)

set(dup-index-server_sources
    dup-io.cpp
    dup-index-server.cpp
)

# set(dup-index-filter_sources
#     dup-io.cpp
#     dup-index-filter.cpp
# )

add_executable(dup-index-server ${dup-index-server_sources})
# add_executable(dup-index-filter ${dup-index-filter_sources})

if(ARB)
    # Build with ARB file support.
    add_definitions(-DARB)
    target_link_libraries(dup-index-server CaSSiS minipt ARB)
    # target_link_libraries(dup-index-filter CaSSiS minipt ARB)
else(ARB)
    target_link_libraries(dup-index-server CaSSiS minipt)
    # target_link_libraries(dup-index-filter CaSSiS minipt)
endif(ARB)

install(TARGETS dup-index-server DESTINATION bin)
# install(TARGETS dup-index-filter DESTINATION bin)
