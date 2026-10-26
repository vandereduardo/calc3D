cmake_minimum_required(VERSION 3.21)

project(calc3D VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

qt_add_executable(calc3D
    src/app/main.cpp
    src/app/main_window.cpp
    src/app/main_window.h
    src/calculation/calculate_cost.cpp
    src/calculation/calculate_cost.h
    src/gcode/gcode_metadata.cpp
    src/gcode/gcode_metadata.h
)

target_link_libraries(calc3D PRIVATE Qt6::Widgets)

if(MSVC)
    target_compile_options(calc3D PRIVATE /utf-8)
endif()

set_target_properties(calc3D PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
