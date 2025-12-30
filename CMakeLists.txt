cmake_minimum_required(VERSION 3.20)
project(vader5ctl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v5.0.0
)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tomlplusplus ftxui)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

option(ENABLE_CLANG_TIDY "Enable clang-tidy" ON)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY clang-tidy)
    if(CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY}
            --warnings-as-errors=*
        )
    endif()
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(vader5d
    src/main.cpp
    src/hidraw.cpp
    src/transport.cpp
    src/uinput.cpp
    src/gamepad.cpp
    src/config.cpp
)
target_include_directories(vader5d PRIVATE include ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(vader5d PRIVATE tomlplusplus::tomlplusplus ${LIBUSB_LIBRARIES})

add_executable(vader5-debug
    src/debug.cpp
    src/hidraw.cpp
    src/transport.cpp
)
target_include_directories(vader5-debug PRIVATE include ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(vader5-debug PRIVATE ${LIBUSB_LIBRARIES} ftxui::screen ftxui::dom ftxui::component)
