cmake_minimum_required(VERSION 3.16)
project(widgetsboxmodel VERSION 8.10.1)

find_package(Albert REQUIRED)

find_package(Qt6 QUIET COMPONENTS StateMachine)
if(Qt6StateMachine_FOUND)
    albert_plugin(QT Widgets StateMachine Svg)
else()
    message(STATUS "Qt6StateMachine not found, skipping widgetsboxmodel-qss plugin")
    return()
endif()

install(
    DIRECTORY "themes/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/themes"
    REGEX "themes\\/\\..+" EXCLUDE  # exclude hidden files
)
