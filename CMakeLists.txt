cmake_minimum_required (VERSION 3.0)
project (Logger)

#cmake_policy (SET CMP0012 NEW)
cmake_policy (SET CMP0000 NEW) # A minimum required CMake version must be specified.
cmake_policy (SET CMP0017 NEW) # Prefer files from the CMake module directory when including from there.

LIST (INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/CMake-modules)

SET (SANDBOX ${PROJECT_SOURCE_DIR})

message (STATUS "--- configuration ---")
SET (CMAKE_DEBUG_POSTFIX "_d")

if (WIN32)
    if (MSVC)
  endif ()
elseif (UNIX)
    add_definitions ("-DWall")
endif()

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

#
SET (INCLUDE_PROJECT_NAME include)
SET (INCLUDE_DIRECTORY include)
file (GLOB logger_headers "${INCLUDE_DIRECTORY}/logger/*.hpp")
file (GLOB logger_details_headers "${INCLUDE_DIRECTORY}/logger/details/*.hpp")

source_group(logger FILES ${logger_headers})
source_group(logger/details FILES ${logger_details_headers})

#add_library (${PROJECT_NAME}_dummy ${logger_headers} ${logger_details_headers})
#set_target_properties(${PROJECT_NAME}_dummy PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(${PROJECT_NAME}_dummy
#                      PROPERTIES PROJECT_LABEL 
#                      ${PROJECT_NAME})

add_library(${INCLUDE_PROJECT_NAME} ${logger_headers} ${logger_details_headers})
set_target_properties(${INCLUDE_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#add_library(${PROJECT_NAME} INTERFACE)

#target_include_directories(${PROJECT_NAME} INTERFACE
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#  $<INSTALL_INTERFACE:${INCLUDE_DIRECTORY}/logger>

include_directories (${INCLUDE_DIRECTORY})

add_subdirectory (3rdparty/concurrentqueue)
add_subdirectory (src/examples)