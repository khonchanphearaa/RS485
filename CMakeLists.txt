cmake_minimum_required(VERSION 3.16)

project(RS485
    VERSION 1.0.0
    DESCRIPTION "RS485 meter reading daemon for water/electric defines meters"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Find MySQL Connector/C++ directly (no .pc file shipped on Debian bookworm)
find_library(MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn REQUIRED)
find_path(MYSQLCPPCONN_INCLUDE_DIR NAMES cppconn/driver.h REQUIRED)

if(NOT MYSQLCPPCONN_LIBRARY)
    message(FATAL_ERROR "MySQL Connector/C++ library not found. Install libmysqlcppconn-dev.")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${MYSQLCPPCONN_INCLUDE_DIR}
)

# Make library available to subdirectory targets
set(MYSQLCPPCONN_LIBRARIES ${MYSQLCPPCONN_LIBRARY})

add_subdirectory(build)

install(TARGETS meter_reader DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)