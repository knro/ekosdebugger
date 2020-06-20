cmake_minimum_required(VERSION 3.5)

project(ekosdebugger LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets Sql REQUIRED)

add_executable(ekosdebugger
    main.cpp
    debuggerview.cpp
    debuggerview.ui
    userdb.cpp
    profileinfo.cpp
)

target_link_libraries(ekosdebugger PRIVATE
    Qt5::Widgets
    Qt5::Sql
    )
