cmake_minimum_required(VERSION 3.16)
project(calculator_qalculate VERSION 7.7.1)

find_package(Albert REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBQALCULATE libqalculate)
if(NOT LIBQALCULATE_FOUND)
    message(STATUS "libqalculate not found, skipping calculator-qalculate plugin")
    return()
endif()

albert_plugin(
    INCLUDE SYSTEM PRIVATE
        ${LIBQALCULATE_INCLUDE_DIRS}
    LINK PRIVATE
        ${LIBQALCULATE_LIBRARIES}
    QT Widgets
)

target_link_directories(${PROJECT_NAME} PRIVATE ${LIBQALCULATE_LIBRARY_DIRS})
