cmake_minimum_required(VERSION 3.2)

project(Typo)

set (CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE src_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp" "src/*.h")
add_executable(Typo
    ${src_files}
)

include_directories(
    src/
    # C:/MinGW/lib
    C:/MinGW/include
)
link_directories(
    C:/MinGW/lib
)

target_link_libraries(Typo PUBLIC libpdcurses)

