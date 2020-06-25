cmake_minimum_required(VERSION 3.5)

project(ekosdebugger LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets Sql Xml REQUIRED)

set(SOURCES
    main.cpp
    debuggerview.cpp
    debuggerview.ui
    userdb.cpp
    profileinfo.cpp
    handler.cpp
    xmldriverslistreader.cpp)

qt5_add_resources(SOURCES resources/resources.qrc)

add_executable(ekosdebugger ${SOURCES})

target_link_libraries(ekosdebugger PRIVATE
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    )

install(TARGETS ekosdebugger RUNTIME DESTINATION bin )

