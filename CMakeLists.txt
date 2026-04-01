cmake_minimum_required(VERSION 3.16..3.28)
project("Wallwatch" VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/colorspace)

add_library(m3_lib STATIC
    m3/hct/hct.cc
    m3/hct/cam.cc
    m3/hct/hct_solver.cc
    m3/hct/viewing_conditions.cc
    m3/utils/utils.cc
    m3/contrast/contrast.cc
    m3/score/score.cpp
)

add_library(scheme_lib STATIC
    colorspace/colorspace.cpp
    colorspace/blend.cpp
    colorspace/lab.cpp
    colorspace/temperature/temperature.cpp
    scheme/tone.cpp
    scheme/dislike/dislike.cpp
    scheme/dynamic/dynamiccolor.cpp
    scheme/dynamic/dynamicscheme.cpp
    scheme/dynamic/m3_dynamic.cpp
)

add_library(quantize_lib STATIC
    quantize/wu.cpp
    quantize/wsmeans.cpp
    quantize/celebi.cpp
)

add_executable(wallwatch wallwatch.cpp)
target_link_libraries(wallwatch PRIVATE
    m3_lib
    scheme_lib
    quantize_lib
)

target_include_directories(wallwatch PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/colorspace
    ${CMAKE_CURRENT_SOURCE_DIR}/scheme
    ${CMAKE_CURRENT_SOURCE_DIR}/m3/hct
    ${CMAKE_CURRENT_SOURCE_DIR}/m3/score
    ${CMAKE_CURRENT_SOURCE_DIR}m3/utils
    ${CMAKE_CURRENT_SOURCE_DIR}m3/contrast
    ${CMAKE_CURRENT_SOURCE_DIR}/scheme/variants
    ${CMAKE_CURRENT_SOURCE_DIR}/scheme/dislike
    ${CMAKE_CURRENT_SOURCE_DIR}/scheme/dynamic
    ${CMAKE_CURRENT_SOURCE_DIR}/quantize
)
