cmake_minimum_required(VERSION 3.12)

project(ekosdebugger LANGUAGES CXX VERSION 1.1)
set(APP_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
add_definitions(-DAPP_VERSION="${APP_VERSION}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_WITH_QT6 "Build with Qt6" OFF)

if(BUILD_WITH_QT6)
    find_package(Qt6 COMPONENTS Widgets Sql Xml DBus Core5Compat REQUIRED)
else()
    find_package(Qt5 COMPONENTS Widgets Sql Xml DBus REQUIRED)
endif()

set(SOURCES
    main.cpp
    debuggerview.cpp
    debuggerview.ui
    userdb.cpp
    profileinfo.cpp
    handler.cpp
    xmldriverslistreader.cpp)

if(BUILD_WITH_QT6)
    qt6_add_resources(SOURCES resources/resources.qrc)
else()
    qt5_add_resources(SOURCES resources/resources.qrc)
endif()

add_executable(ekosdebugger ${SOURCES})

if(BUILD_WITH_QT6)
    target_link_libraries(ekosdebugger PRIVATE
        Qt6::Widgets
        Qt6::Sql
        Qt6::Xml
        Qt6::DBus
        Qt6::Core5Compat
    )
else()
    target_link_libraries(ekosdebugger PRIVATE
        Qt5::Widgets
        Qt5::Sql
        Qt5::Xml
        Qt5::DBus
    )
endif()

install(TARGETS ekosdebugger RUNTIME DESTINATION bin )
