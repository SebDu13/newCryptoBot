cmake_minimum_required(VERSION 3.15)
project(CrypotBot)

#add_definitions(-DBOOST_LOG_DYN_LINK)

#boost
find_package(Boost 1.71.0 COMPONENTS log_setup log program_options REQUIRED)
#include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
add_definitions(-std=c++2a -Wno-deprecated-declarations)
#set (CMAKE_CXX_FLAGS "-DBOOST_LOG_DYN_LINK")
#set (CMAKE_CXX_FLAGS "-DCMAKE_TOOLCHAIN_FILE=../conan/conan_toolchain.cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES conan_provider.cmake)


file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

if(NOT EXISTS ${CMAKE_BINARY_DIR}/bin)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries (
	${PROJECT_NAME}
	${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    OpenSSL::Crypto)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/tools)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/exchanges)
include_directories(${CURL_INCLUDE_DIR})
include_directories(${OpenSSL_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

include(CMakePrintHelpers)
cmake_print_variables(Boost_LIBRARIES)
