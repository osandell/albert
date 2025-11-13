cmake_minimum_required(VERSION 3.16)
project(bluetooth VERSION 4.6.0)

find_package(Albert REQUIRED)

albert_plugin(
    SOURCES
        src/*.h
        src/*.cpp
        ${PROJECT_NAME}.qrc
    INCLUDE PRIVATE
        src/
    QT
        Core
)

if(APPLE)

    albert_plugin_compile_options(PRIVATE
        "-fobjc-arc"
    )

    albert_plugin_link(PRIVATE
        "objc"
        "-framework Foundation"
        "-framework IOBluetooth"
    )

    albert_plugin_include_directories(PRIVATE
        src/iobluetooth
    )

    albert_plugin_sources(PRIVATE
        src/iobluetooth/*.h
        src/iobluetooth/*.mm
        src/iobluetooth/*.cpp
    )

elseif(UNIX)

    albert_plugin_link_qt(DBus VERSION 6.0 REQUIRED)

    albert_plugin_dbus_interface(
        XML "src/bluez/bluez.xml"
        INCLUDE "custom_types.h"
    )

    albert_plugin_include_directories(PRIVATE
        src/bluez
    )

    albert_plugin_sources(PRIVATE
        src/bluez/*.h
        src/bluez/*.cpp
    )

endif()
