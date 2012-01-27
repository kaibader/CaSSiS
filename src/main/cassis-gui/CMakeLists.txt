project(CaSSiS-GUI)

# Search for Qt4 includes and libraries...
find_package(Qt4)
if(NOT QT4_FOUND)
    message(STATUS "Qt4-dev files not found. Skipping the CaSSiS GUI build process...")
else(NOT QT4_FOUND)

set(QT_USE_OPENGL TRUE)
include(${QT_USE_FILE})

set(cdcassis_sources
    "${CMAKE_SOURCE_DIR}/shared/newick.cpp"
    linechart.cpp
    main.cpp
    mainwindow.cpp
    resulttab.cpp
    tree.cpp
    treeview.cpp
)

set(resources
    resources/cdcassis.qrc
)
qt4_add_resources(resources_cpp ${resources})

set(moc_files
    linechart.h
    mainwindow.h
    resulttab.h
    tree.h
    treeview.h
)
qt4_wrap_cpp(moc_cpp_files ${moc_files})

include_directories(
    "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_BINARY_DIR}/lib"
    "${CMAKE_SOURCE_DIR}/shared"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
    "${QT_INCLUDE_DIR}"
    "${QT_QTGUI_INCLUDE_DIR}"
    "${QT_QTOPENGL_INCLUDE_DIR}"
)

add_executable(cassis-gui ${cdcassis_sources} ${moc_cpp_files} ${resources_cpp})
target_link_libraries(cassis-gui CaSSiS ${QT_LIBRARIES} ${QT_QTGUI_LIBRARIES} ${QT_QTOPENGL_LIBRARIES})

# Install the CaSSiS GUI.
install(TARGETS cassis-gui DESTINATION bin)

endif(NOT QT4_FOUND)
