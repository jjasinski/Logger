cmake_minimum_required (VERSION 3.0)
project (Logger)

option (UseExternalConcurrentQueue "Use an external ConcurrentQueue implementation from https://github.com/cameron314/concurrentqueue" 
        ON
        )


cmake_policy (SET CMP0000 NEW) # A minimum required CMake version must be specified.
cmake_policy (SET CMP0017 NEW) # Prefer files from the CMake module directory when including from there.

LIST (INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/CMake-modules)

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

add_library(${INCLUDE_PROJECT_NAME} ${logger_headers} ${logger_details_headers})
set_target_properties(${INCLUDE_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

include_directories (${INCLUDE_DIRECTORY})

if (UseExternalConcurrentQueue)
  add_definitions(-DLOGGER_USE_MOODYCAMEL_CONCURRENT_QUEUE)
  add_subdirectory (3rdparty/concurrentqueue)

  target_link_libraries (${INCLUDE_PROJECT_NAME} ConcurrentQueue)
endif (UseExternalConcurrentQueue)

add_subdirectory (src/examples)
add_subdirectory (src/benchmark)