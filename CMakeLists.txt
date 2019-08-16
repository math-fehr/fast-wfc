cmake_minimum_required(VERSION 3.9)
project(fastwfc VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(DEFAULT_BUILD_TYPE "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
endif()

include(GNUInstallDirs)

set(SOURCE_FILES src/lib/wave.cpp src/lib/propagator.cpp src/lib/wfc.cpp)

add_library(${PROJECT_NAME}_static STATIC ${SOURCE_FILES})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set(LIBRARY_OUTPUT_PATH lib CACHE PATH "Build directory" FORCE)

target_include_directories(${PROJECT_NAME}_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>)

set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1)

install(TARGETS ${PROJECT_NAME} EXPORT FastWFCConfig
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})

install(TARGETS ${PROJECT_NAME}_static EXPORT FastWFCStaticConfig
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY src/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(EXPORT FastWFCConfig DESTINATION share/fastwfc/cmake)
install(EXPORT FastWFCStaticConfig DESTINATION share/fastwfc/cmake)

export(TARGETS ${PROJECT_NAME} FILE FastWFCConfig.cmake)
export(TARGETS ${PROJECT_NAME}_static FILE FastWFCStaticConfig.cmake)
