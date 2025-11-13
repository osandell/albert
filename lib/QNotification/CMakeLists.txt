# SPDX-FileCopyrightText: 2024 Manuel Schneider
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

project(qnotifications VERSION 1.0)

set(REQUIRED_QT_VERSION 6.0.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} COMPONENTS Core REQUIRED)

set(PUBLIC_HEADER
    include/qnotification.h
    ${PROJECT_BINARY_DIR}/qnotifications_export.h
)

add_library(${PROJECT_NAME} STATIC
    ${PUBLIC_HEADER}
    src/qnotification.cpp
    src/qnotificationmanager.h
)
add_library(QNotifications::QNotifications ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        src/
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})

if(WIN32)

    target_sources(${PROJECT_NAME} PRIVATE
        src/qnotification_win.cpp
        src/qnotificationmanager_win.cpp
    )

elseif(APPLE)

    target_sources(${PROJECT_NAME} PRIVATE
        src/qnotificationmanager_mac.mm
    )

    set_source_files_properties(
        src/qnotificationmanager_mac.mm
        PROPERTIES COMPILE_FLAGS "-fobjc-arc"
    )

    target_link_libraries(${PROJECT_NAME}
        # PRIVATE "-framework Cocoa"
    )

elseif(UNIX)  # assume xdg

    target_sources(${PROJECT_NAME} PRIVATE
        src/qnotificationmanager_xdg.cpp
    )

    find_package(Qt6 COMPONENTS DBus REQUIRED)

    qt_add_dbus_interface(GENERATED_DBUS org.freedesktop.Notifications.xml xdgnotificationsinterface)

    target_sources(${PROJECT_NAME} PRIVATE
        ${GENERATED_DBUS}
        org.freedesktop.Notifications.xml
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::DBus)

else()

    message(FATAL_ERROR "Unsupported platform")

endif()


set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    AUTOMOC ON
)
