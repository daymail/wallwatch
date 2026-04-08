cmake_minimum_required(VERSION 3.16..3.28)
project("Wallwatch" VERSION 1.0 LANGUAGES CXX)
include(fn.cmake)
include(libs.cmake)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs videoio)
qt_standard_project_setup(REQUIRES 6.9)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(wallwatch
    wallwatch.cpp
    filewatcher.cpp
    filewatcher.h
    themer.cpp
)

set_target_properties(wallwatch PROPERTIES AUTOMOC ON)

target_link_libraries(wallwatch PRIVATE
    quantize_lib
    scheme_lib
    m3_lib
    variants_lib
    ${OpenCV_LIBRARIES}
    Qt::Core
    Qt::Concurrent
)

target_include_directories(wallwatch PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    colorspace
    scheme
    quantize
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(quantize_lib PRIVATE ${OpenCV_LIBRARIES})
target_include_directories(quantize_lib PRIVATE ${OpenCV_INCLUDE_DIRS})
install(TARGETS wallwatch RUNTIME DESTINATION /usr/local/bin)
