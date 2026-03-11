cmake_minimum_required(VERSION 3.15)

project(EDF LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(cpr REQUIRED)
find_package(OpenSSL REQUIRED)

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(EDF ${sources} ${headers})
target_link_libraries(EDF PRIVATE Qt5::Core cpr::cpr)

list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_library(EDFLib ${sources} ${headers})
target_include_directories(EDFLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(EDFLib PRIVATE Qt5::Core cpr::cpr)

