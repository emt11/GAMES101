cmake_minimum_required(VERSION 3.20)

project(Transformation LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Eigen3 CONFIG REQUIRED)

add_executable(Transformation main.cpp)

target_link_libraries(Transformation PRIVATE Eigen3::Eigen)
target_compile_features(Transformation PRIVATE cxx_std_23)
