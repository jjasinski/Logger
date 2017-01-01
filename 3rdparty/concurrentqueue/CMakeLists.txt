cmake_minimum_required (VERSION 3.3)

include(ExternalProject)

project(ConcurrentQueue)

include(ExternalProject)
ExternalProject_Add(ConcurrentQueue-download
  GIT_REPOSITORY "https://github.com/cameron314/concurrentqueue.git"
  GIT_TAG "master"
  SOURCE_DIR        "${PROJECT_BINARY_DIR}/src"
  CONFIGURE_COMMAND "" 
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

add_library (${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE 
  ${PROJECT_BINARY_DIR}/src
  )
target_sources(${PROJECT_NAME} INTERFACE 
  ${PROJECT_BINARY_DIR}/src/concurrentqueue.h
)

#add_dependencies (${PROJECT_NAME} ConcurrentQueue-download)

