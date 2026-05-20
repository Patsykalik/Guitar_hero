cmake_minimum_required(VERSION 3.16)

project(GuitarHeroClone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
    Sql
)

add_executable(GuitarHeroClone
    game.cpp
    gameplay.cpp
    gameplay.h
    interface.cpp
    interface.h
)

target_link_libraries(GuitarHeroClone PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Sql
)

target_compile_options(GuitarHeroClone PRIVATE
    -Wall
    -Wextra
)

set(ASSETS
    Zhit_v_kayf.mp3
    Freedom.mp3
    Don_Omar.mp3
    3ae8bceb83287ebf54c46b101eafd361.jpg
)

foreach(FILE ${ASSETS})

    add_custom_command(
        TARGET GuitarHeroClone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/${FILE}
        $<TARGET_FILE_DIR:GuitarHeroClone>/${FILE}
    )

endforeach()
