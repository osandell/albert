cmake_minimum_required(VERSION 3.16)
project(vpn VERSION 7.5.1)

find_package(Albert REQUIRED)

file(GLOB_RECURSE I18N_SOURCES src/*)

albert_plugin(
    SOURCES
        src/*.h
        src/*.cpp
        vpn.qrc
    INCLUDE PRIVATE
        src
    I18N_SOURCES
        ${I18N_SOURCES}
    QT
        Widgets
)

if(APPLE)

    albert_plugin_link(PRIVATE "-framework SystemConfiguration")

    albert_plugin_include_directories(PRIVATE src/mac)

    albert_plugin_sources(src/mac/plugin.cpp)

elseif(UNIX)

    albert_plugin_link_qt(DBus VERSION 6.0 REQUIRED)

    albert_plugin_dbus_interface(
        XML "src/xdg/nm.xml"
        INCLUDE "custom_types.h"
    )

    albert_plugin_include_directories(PRIVATE
        src/xdg  # for custom_types.h
        ${PROJECT_BINARY_DIR}  # for the generated files
    )

    albert_plugin_sources(GLOB
        src/xdg/plugin.cpp
        src/xdg/custom_types.h
    )

endif()
