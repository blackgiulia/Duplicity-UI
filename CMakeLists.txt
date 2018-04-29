cmake_minimum_required(VERSION 3.9)

project(duplicity-ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set (CMAKE_PREFIX_PATH "/Users/hn/Qt5.10.1//5.10.1/clang_64")

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

find_package(Boost REQUIRED COMPONENTS filesystem date_time system)

add_executable(${PROJECT_NAME} "main.cpp" "qml.qrc" "handle.cpp" "handle.hpp")

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Quick ${Boost_LIBRARIES})
