cmake_minimum_required(VERSION 3.16)
project(mediaremote VERSION 7.0.1)

find_package(Albert REQUIRED)

albert_plugin(
    SOURCES
        src/*.h src/*.cpp src/*.ui include/albert/plugin/*.h
    INCLUDE
        INTERFACE include
        PRIVATE src/ include/albert/plugin
    QT Widgets
)

if(APPLE)

    albert_plugin_compile_options(PRIVATE "-fobjc-arc")

    albert_plugin_link(PRIVATE
        "objc"
        "-framework Foundation"
        "-F/System/Library/PrivateFrameworks"
        "-framework MediaRemote"
    )

    albert_plugin_sources(
        src/mediaremote/*.h
        src/mediaremote/*.mm
    )

    albert_plugin_include_directories(PRIVATE src/mediaremote)

elseif(UNIX)

    albert_plugin_include_directories(PRIVATE src/mpris)

    albert_plugin_link_qt(DBus VERSION 6.0 REQUIRED)

    albert_plugin_dbus_interface(XML "src/mpris/mpris.xml")

    albert_plugin_sources(
        src/mpris/*.cpp
        src/mpris/*.h
    )

endif()
