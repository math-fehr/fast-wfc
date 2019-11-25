cmake_minimum_required(VERSION 3.9)
project(wfc_demo LANGUAGES CXX)

make_directory(results)

set(CMAKE_CXX_STANDARD 17)
set(DEFAULT_BUILD_TYPE "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
endif()

find_library( FASTWFC_LIB
  NAMES
    fastwfc_static
    )

add_executable(${PROJECT_NAME} src/lib/main.cpp)
target_link_libraries(wfc_demo ${FASTWFC_LIB})

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>)
