cmake_minimum_required(VERSION 3.16)
project(CollectionArchive LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Diretórios de código ---
include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB APP_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(CollectionArchive
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
    ${CMAKE_SOURCE_DIR}/vendor/sqlite/sqlite3.c
    ${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c
)

target_include_directories(CollectionArchive PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
    ${CMAKE_SOURCE_DIR}/vendor/sqlite
    ${CMAKE_SOURCE_DIR}/vendor/stb
    ${CMAKE_SOURCE_DIR}/vendor/nlohmann
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
)

if (WIN32)
    target_link_directories(CollectionArchive PRIVATE
        ${CMAKE_SOURCE_DIR}/vendor/glfw/lib
    )
    target_link_libraries(CollectionArchive PRIVATE glfw3 opengl32 gdi32 user32 shell32 imm32)
endif()

if (MINGW)
    target_compile_options(CollectionArchive PRIVATE -Wall -O2)
    target_link_options(CollectionArchive PRIVATE -static-libgcc -static-libstdc++)
    target_compile_definitions(CollectionArchive PRIVATE SQLITE_OMIT_LOAD_EXTENSION)
endif()

# Copia dados para o diretório de execução
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
