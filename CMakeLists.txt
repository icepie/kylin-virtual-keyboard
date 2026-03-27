cmake_minimum_required(VERSION 3.10.0)

project(kylin-virtual-keyboard LANGUAGES CXX)

OPTION(ENABLE_TEST "Build Test" OFF)
OPTION(ENABLE_QT6 "Build with Qt6 " OFF)
OPTION(ENABLE_LAYER_SHELL "Build with layer-shell support for Sway/wlroots compositors" ON)

# set something
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find packages
find_package(spdlog REQUIRED)
if (NOT spdlog_FOUND)
    message(FATAL_ERROR "Spdlog not found!")
endif()

if (ENABLE_QT6)
    find_package(QT NAMES Qt6 REQUIRED)
    find_package(Qt6 COMPONENTS Core Gui Widgets Concurrent DBus Quick QuickControls2 Core5Compat REQUIRED)
else()
    find_package(QT NAMES Qt5 REQUIRED)
    find_package(Qt5 COMPONENTS Core Gui Widgets Concurrent DBus Quick QuickControls2 REQUIRED)
endif()

if (NOT Qt${QT_VERSION_MAJOR}_FOUND)
    message(FATAL_ERROR "Qt${QT_VERSION_MAJOR} not found!")
    return()
endif()

message(STATUS "Current Qt version: ${Qt${QT_VERSION_MAJOR}_VERSION}")

find_package(KF${QT_VERSION_MAJOR}WindowSystem REQUIRED)
if (NOT KF${QT_VERSION_MAJOR}WindowSystem_FOUND)
   message(FATAL_ERROR "KF${QT_VERSION_MAJOR}WindowSystem not found!")
endif()

find_package(Fcitx5Core REQUIRED)
if (NOT Fcitx5Core_FOUND)
    message(FATAL_ERROR "Fcitx5Core not found!")
endif()

find_package(PkgConfig REQUIRED)
if (QT_VERSION_MAJOR EQUAL 6)
    # Qt6: 先尝试 gsettings-qt6，再回退到 gsettings-qt（可能为 Qt5 版本）
    pkg_check_modules(GSettings_QT QUIET IMPORTED_TARGET gsettings-qt6)
    if (NOT GSettings_QT_FOUND)
        pkg_check_modules(GSettings_QT REQUIRED IMPORTED_TARGET gsettings-qt)
    endif()
else()
    pkg_check_modules(GSettings_QT REQUIRED IMPORTED_TARGET gsettings-qt)
endif()

if (ENABLE_LAYER_SHELL)
    # LayerShellQt 仅支持 Qt6；Qt5 构建时跳过
    if (QT_VERSION_MAJOR EQUAL 6)
        find_package(LayerShellQt QUIET)
        if (LayerShellQt_FOUND)
            message(STATUS "LayerShellQt found, enabling layer-shell support for Sway/wlroots.")
            add_compile_definitions(HAVE_LAYER_SHELL)
        else()
            message(WARNING "LayerShellQt not found. layer-shell support disabled.\n"
                            "Run: sudo pacman -S layer-shell-qt")
        endif()
    else()
        message(STATUS "layer-shell support requires Qt6. Build with -DENABLE_QT6=ON to enable.")
    endif()
endif()

include(GNUInstallDirs)
include_directories(${GSettings_QT_INCLUDE_DIRS})
include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

set(RESOURCES
    ${CMAKE_SOURCE_DIR}/resources.qrc
)

add_subdirectory(third-party)
add_subdirectory(data)
add_subdirectory(src)
if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()
