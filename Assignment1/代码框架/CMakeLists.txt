cmake_minimum_required(VERSION 3.20)
project(Rasterizer LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV CONFIG REQUIRED)

add_executable(Rasterizer
    main.cpp
    rasterizer.cpp
    rasterizer.hpp
    Triangle.cpp
    Triangle.hpp
)

target_compile_features(Rasterizer PRIVATE cxx_std_23)
target_include_directories(Rasterizer PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(Rasterizer PRIVATE ${OpenCV_LIBRARIES})
