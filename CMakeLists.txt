cmake_minimum_required(VERSION 3.10)
project(vidm C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_executable(vidm
    core/pause.c
    core/audio.c
    core/media_decoder.c
    plugin/plugin.c

    ui/main.cpp
    ui/mainwindow.cpp
    ui/playlistwidget.cpp
    ui/playliststudio.cpp
)

target_include_directories(vidm PRIVATE
    core
    plugin
    ui
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(vidm
    Qt6::Widgets
    ${SDL2_LIBRARIES}
    avformat avcodec avutil swscale
)