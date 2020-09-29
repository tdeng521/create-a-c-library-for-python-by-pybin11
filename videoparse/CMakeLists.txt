cmake_minimum_required(VERSION 3.4)
message(STATUS "generate videoparse module")
add_compile_options(-g -fpic)
set(PYBIND11_CPP_STANDARD -std=c++11)

find_package(pybind11 REQUIRED)
include_directories(${pybind11_INCLUDE_DIRS} ./)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

link_directories(/usr/local/lib)
add_library(videoparse MODULE videoparse.cpp)
target_link_libraries(videoparse  avutil avcodec avformat pybind11::module)