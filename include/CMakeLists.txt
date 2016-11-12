cmake_minimum_required (VERSION 3.0)

project (logger)
message (STATUS "* ${PROJECT_NAME}")


file (GLOB logger_headers "logger/*.hpp")
file (GLOB logger_details_headers "logger/details/*.hpp")

source_group(logger FILES ${logger_headers})
source_group(logger/details FILES ${logger_details_headers})

add_library (${PROJECT_NAME}_dummy ${logger_headers} ${logger_details_headers})
set_target_properties(${PROJECT_NAME}_dummy PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${PROJECT_NAME}_dummy
                      PROPERTIES PROJECT_LABEL 
                      ${PROJECT_NAME})

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/logger>
)
#target_include_directories(${PROJECT_NAME} 
#    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
#)