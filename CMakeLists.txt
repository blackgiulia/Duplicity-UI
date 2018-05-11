cmake_minimum_required(VERSION 3.9)

project(duplicity-ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

find_package(Boost COMPONENTS filesystem date_time system REQUIRED)

if(WIN32 OR APPLE)
    add_definitions(-DFD_SETSIZE=1024)
endif()

add_executable(${PROJECT_NAME} "main.cpp" "qml.qrc" "handle.cpp" "handle.hpp")

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Quick ${Boost_LIBRARIES})
