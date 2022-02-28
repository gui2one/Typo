cmake_minimum_required(VERSION 3.2)

project(Typo)

set (CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE src_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp" "src/*.h")
add_executable(Typo
    ${src_files}
)
target_compile_options(Typo PRIVATE )
include_directories(
    src/
    # C:/MinGW/lib
    C:/msys64/usr/include
    C:/msys64/mingw64/include
)
link_directories(
    C:/msys64/usr/lib
    C:/msys64/mingw64/lib
)

target_link_libraries(Typo PUBLIC 
    -lncurses
       
)
add_custom_command(
    TARGET Typo 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}\\resources
        $<TARGET_FILE_DIR:Typo>\\resources
)
