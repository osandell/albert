if (APPLE)

cmake_minimum_required(VERSION 3.16)
project(menubar VERSION 2.5.1)

find_package(Albert REQUIRED)

albert_plugin(
    LINK PRIVATE
        ${CORESERVICES_LIBRARY}
         objc
         "-framework CoreServices"
         "-framework ApplicationServices"
         "-framework Cocoa"
    QT
        Widgets
)

target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")

endif()
