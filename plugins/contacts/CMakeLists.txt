if(APPLE)

cmake_minimum_required(VERSION 3.16)
project(contacts VERSION 6.8.1)

find_package(Albert REQUIRED)

albert_plugin(
    LINK PRIVATE "-framework Contacts"
    QT Widgets Concurrent
)

target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")

endif()
