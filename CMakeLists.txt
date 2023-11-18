cmake_minimum_required(VERSION 3.15)
project(CrypotBot)

#boost
find_package(Boost 1.71.0 COMPONENTS log program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-std=c++2a -DBOOST_LOG_DYN_LINK)


file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

if(NOT EXISTS ${CMAKE_BINARY_DIR}/bin)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/tools)