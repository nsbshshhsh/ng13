cmake_minimum_required(VERSION 3.14)
project(CinemaBookingSystem CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Bật cảnh báo compiler
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Thu thập tất cả source files
file(GLOB_RECURSE SOURCES
    "src/models/*.cpp"
    "src/services/*.cpp"
    "src/utils/*.cpp"
    "src/main.cpp"
)

add_executable(CinemaBooking ${SOURCES})

# Include directories
target_include_directories(CinemaBooking PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Tạo thư mục data khi build
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/data)

message(STATUS "=== Cinema Booking System ===")
message(STATUS "C++ Standard : C++17")
message(STATUS "Sources count: ${SOURCES}")
