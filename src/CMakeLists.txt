cmake_minimum_required (VERSION 3.0)

project (logger)
message (STATUS "* ${PROJECT_NAME}")


file (GLOB logger_headers "logger/*.hpp")
file (GLOB logger_details_headers "logger/details/*.hpp")

source_group(logger FILES ${logger_headers})
source_group(logger/details FILES ${logger_details_headers})

add_executable (${PROJECT_NAME} main.cpp ${logger_headers} ${logger_details_headers})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

#add_library (${PROJECT_NAME} ${logger_headers} ${logger_sources})
#add_executable (${PROJECT_NAME} main.cpp)