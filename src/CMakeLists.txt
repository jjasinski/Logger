cmake_minimum_required (VERSION 3.0)

project (logger)
message (STATUS "* ${PROJECT_NAME}")


file (GLOB logger_headers "logger/*.hpp")
file (GLOB logger_sources "logger/*.cpp")

add_executable (${PROJECT_NAME} main.cpp ${logger_headers} ${logger_sources})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

#add_library (${PROJECT_NAME} ${logger_headers} ${logger_sources})
#add_executable (${PROJECT_NAME} main.cpp)