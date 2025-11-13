cmake_minimum_required(VERSION 3.16)
project(applications VERSION 14.12.0)

find_package(Albert REQUIRED)

file(GLOB_RECURSE I18N_SOURCES src/*)

albert_plugin(
    INCLUDE
        INTERFACE include
        PRIVATE include/albert/plugin src
    SOURCES
        src/applicationbase.cpp
        src/applicationbase.h
        src/pluginbase.cpp
        src/pluginbase.h
        include/albert/plugin/${PROJECT_NAME}.h
    I18N_SOURCES
        ${I18N_SOURCES}
    QT
        Widgets
        Concurrent
)

if(WIN32)
    # Override the hidden visibility preset from albert_plugin macro for Windows
    # The base classes from albert library are already exported, we just need default visibility
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
    )
    target_include_directories(${PROJECT_NAME} PRIVATE src/win)
    target_sources(${PROJECT_NAME} PRIVATE
        src/win/application.cpp
        src/win/application.h
        src/win/configwidget.ui
        src/win/plugin.cpp
        src/win/plugin.h
        src/win/terminal.cpp
        src/win/terminal.h
    )
    # Link Windows libraries for COM and Shell API
    target_link_libraries(${PROJECT_NAME} PRIVATE shell32 shlwapi ole32)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE objc "-framework Cocoa")
    target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")
    target_include_directories(${PROJECT_NAME} PRIVATE src/mac)
    target_sources(${PROJECT_NAME} PRIVATE
        src/mac/configwidget.ui
        src/mac/application.mm
        src/mac/application.h
        src/mac/plugin.cpp
        src/mac/plugin.h
        src/mac/terminal.cpp
        src/mac/terminal.h
    )
elseif(UNIX)
    target_include_directories(${PROJECT_NAME} PRIVATE src/xdg)
    target_sources(${PROJECT_NAME} PRIVATE
        src/xdg/application.cpp
        src/xdg/application.h
        src/xdg/configwidget.ui
        src/xdg/plugin.cpp
        src/xdg/plugin.h
        src/xdg/terminal.cpp
        src/xdg/terminal.h
    )
endif()

