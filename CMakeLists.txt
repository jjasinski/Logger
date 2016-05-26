cmake_minimum_required (VERSION 3.0)
project (LoggerSolution)

#cmake_policy (SET CMP0012 NEW)
cmake_policy (SET CMP0000 NEW) # A minimum required CMake version must be specified.
cmake_policy (SET CMP0017 NEW) # Prefer files from the CMake module directory when including from there.

SET (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/BuildPlatform/CMake-modules ${PROJECT_SOURCE_DIR}/CMake-modules)

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

add_subdirectory (src)