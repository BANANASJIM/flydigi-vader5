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

# ---- Options ----

option(ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
option(USE_HIDAPI "Use hidapi (libusb backend) instead of Linux hidraw" OFF)

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

# ---- Backend selection ----

if(USE_HIDAPI)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(HIDAPI REQUIRED hidapi-libusb)
    add_compile_definitions(VADER5_USE_HIDAPI)

    set(HIDRAW_SOURCES src/hidapi_device.cpp)
    set(HIDRAW_LIBS ${HIDAPI_LIBRARIES})
    set(HIDRAW_INCLUDE_DIRS ${HIDAPI_INCLUDE_DIRS})
    set(HIDRAW_LIBRARY_DIRS ${HIDAPI_LIBRARY_DIRS})
    set(HIDRAW_CFLAGS ${HIDAPI_CFLAGS_OTHER})

    message(STATUS "Using hidapi (libusb) backend for HID communication")
    message(STATUS "  hidapi include: ${HIDAPI_INCLUDE_DIRS}")
    message(STATUS "  hidapi libs:    ${HIDAPI_LIBRARIES}")
else()
    set(HIDRAW_SOURCES src/hidraw.cpp)
    set(HIDRAW_LIBS "")
    set(HIDRAW_INCLUDE_DIRS "")
    set(HIDRAW_LIBRARY_DIRS "")
    set(HIDRAW_CFLAGS "")

    message(STATUS "Using Linux hidraw backend for HID communication")
endif()

# ---- Targets ----

# vader5d daemon
add_executable(vader5d
    src/daemon/main.cpp
    ${HIDRAW_SOURCES}
    src/uinput.cpp
    src/gamepad.cpp
    src/config.cpp
    src/keycodes.cpp
    src/mouse.cpp
)
target_include_directories(vader5d PRIVATE include ${HIDRAW_INCLUDE_DIRS})
target_link_libraries(vader5d PRIVATE tomlplusplus::tomlplusplus ${HIDRAW_LIBS})
target_link_directories(vader5d PRIVATE ${HIDRAW_LIBRARY_DIRS})
target_compile_options(vader5d PRIVATE ${HIDRAW_CFLAGS})

# vader5-debug TUI tool
add_executable(vader5-debug
    src/tools/debug.cpp
    ${HIDRAW_SOURCES}
)
target_include_directories(vader5-debug PRIVATE include ${HIDRAW_INCLUDE_DIRS})
target_link_libraries(vader5-debug PRIVATE ftxui::screen ftxui::dom ftxui::component ${HIDRAW_LIBS})
target_link_directories(vader5-debug PRIVATE ${HIDRAW_LIBRARY_DIRS})
target_compile_options(vader5-debug PRIVATE ${HIDRAW_CFLAGS})

# ---- Test targets ----

add_executable(test-debug-iface
    src/tools/test_debug_iface.cpp
)
set_target_properties(test-debug-iface PROPERTIES CXX_CLANG_TIDY "")
target_include_directories(test-debug-iface PRIVATE include)

add_executable(test-remap
    src/tools/test_remap.cpp
    src/config.cpp
    src/keycodes.cpp
)
set_target_properties(test-remap PROPERTIES CXX_CLANG_TIDY "")
target_include_directories(test-remap PRIVATE include)
target_link_libraries(test-remap PRIVATE tomlplusplus::tomlplusplus)

add_executable(test-uinput-elite
    src/tools/test_uinput_elite.cpp
    src/config.cpp
    src/keycodes.cpp
)
set_target_properties(test-uinput-elite PROPERTIES CXX_CLANG_TIDY "")
target_include_directories(test-uinput-elite PRIVATE include)
target_link_libraries(test-uinput-elite PRIVATE tomlplusplus::tomlplusplus)
