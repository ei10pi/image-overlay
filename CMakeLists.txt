cmake_minimum_required(VERSION 3.24)

project(image_overlay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS FALSE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3")

set(SOURCES
    main.cpp
    shared.cpp
    bitmap.cpp
    convert.cpp
    overlay.cpp
    video.cpp
    yuv.cpp
)

add_executable(image_overlay ${SOURCES})

set_target_properties(image_overlay PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build)

