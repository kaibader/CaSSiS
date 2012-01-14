project(CaSSiS)

# Enable this option to build a pThreads version of the BGRT library.
option(PTHREADS "Enable parallel bgrt processing using pThreads" OFF)

# List of source files for the BGRT library.
set(bgrt_sources
    bgrt.cpp
    io.cpp
    namemap.cpp
    search.cpp
    thermodynamics.cpp
    tree.cpp
)

# Some configuration stuff necessary for the pThreads version of the BGRT library.
if(PTHREADS)
    message("Parallel bgrt processing using pThreads enabled.")
    list(APPEND bgrt_sources pool.cpp)
    set(CMAKE_THREAD_PREFER_PTHREADS 1)
    find_package(Threads REQUIRED)
    if(NOT CMAKE_USE_PTHREADS_INIT)
        message(STATUS_ERROR "pThread support not found.")
    endif(NOT CMAKE_USE_PTHREADS_INIT)
    set(bgrt_LINK_FLAGS "${bgrt_LINK_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
endif(PTHREADS)

# Create a config.h file from the config.h.in template.
configure_file("${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h" ESCAPE_QUOTES)
include_directories("${PROJECT_BINARY_DIR}")

# Build the BGRT library as a shared library.
if(WIN32)
add_library(CaSSiS STATIC ${bgrt_sources})
else(WIN32)
add_library(CaSSiS SHARED ${bgrt_sources})
endif(WIN32)
target_link_libraries(CaSSiS ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(
    CaSSiS PROPERTIES
    VERSION "${CASSIS_VERSION_MAJOR}.${CASSIS_VERSION_MINOR}.${CASSIS_VERSION_PATCH}${CASSIS_VERSION_SUFFIX}"
    SOVERSION "${CASSIS_VERSION_MAJOR}"
)

# Install libCaSSiS.
# file(GLOB header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
# install(FILES ${header} DESTINATION include/bgrt)
install(TARGETS CaSSiS DESTINATION lib)
