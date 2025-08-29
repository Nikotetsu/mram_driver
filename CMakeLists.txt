cmake_minimum_required(VERSION 3.5)

#
#   Project details
#

project(
    "MRAM_driver"
    VERSION "0.0.1"
    LANGUAGES CXX
)

message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")

include(cmake/settings.cmake)
#include(cmake/packages.cmake)
include(cmake/sources.cmake)

#
# Testing
#

find_package(GTest REQUIRED)
enable_testing()

message(STATUS "Run testing module\n")
include(cmake/testing_gtest.cmake)
message(STATUS "End testing module\n")

#
# Application
#

add_executable(mram_driver
    ${exe_sources}
)
target_link_libraries(mram_driver
PRIVATE
    
)



