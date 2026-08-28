# Copyright (c) 2025 Realtek Semiconductor Corporation
# SPDX-License-Identifier: Apache-2.0
#
# CMake fragment for rtl87x2j_platform_lib.
#
# Usage — include from an application or MCUboot CMakeLists.txt that already
# has find_package(Zephyr ...) in scope:
#
#   add_subdirectory(${CMAKE_SOURCE_DIR}/../lib/rtl87x2j_platform_lib
#                    ${CMAKE_CURRENT_BINARY_DIR}/rtl87x2j_platform_lib)
#   target_link_libraries(app PUBLIC rtl87x2j_platform_lib)
#
# The build invokes the companion Makefile via ExternalProject_Add, forwarding
# Zephyr's toolchain flags so the library is compiled for the same target as
# the Zephyr image.
#
# Override RTK_BASE on the CMake command line if the bee5-zephyr tree is at a
# different location:
#   cmake -DRTK_BASE=/path/to/bee5-zephyr/realtek ...

cmake_minimum_required(VERSION 3.20.0)

# ---------------------------------------------------------------------------
# Export Zephyr toolchain flags for the external Makefile
# ---------------------------------------------------------------------------
zephyr_get_include_directories_for_lang_as_string(       C zephyr_includes)
zephyr_get_system_include_directories_for_lang_as_string(C zephyr_system_includes)
zephyr_get_compile_definitions_for_lang_as_string(       C zephyr_definitions)
zephyr_get_compile_options_for_lang_as_string(           C zephyr_options)

if(DEFINED CMAKE_C_COMPILER_TARGET)
  set(target_flag "--target=${CMAKE_C_COMPILER_TARGET}")
endif()

set(rtl87x2j_platform_lib_cflags
    "${target_flag} ${zephyr_includes} ${zephyr_definitions} ${zephyr_options} ${zephyr_system_includes}"
)

# ---------------------------------------------------------------------------
# RTK_BASE: root of the Realtek bee5-zephyr source tree
# ---------------------------------------------------------------------------
if(NOT DEFINED RTK_BASE)
  set(RTK_BASE "/mnt/d/zcode/bee5-zephyr/realtek")
endif()

# ---------------------------------------------------------------------------
# ExternalProject: build the library via the companion Makefile
# ---------------------------------------------------------------------------
set(rtl87x2j_platform_lib_src_dir   ${CMAKE_CURRENT_SOURCE_DIR})
set(rtl87x2j_platform_lib_build_dir ${CMAKE_CURRENT_BINARY_DIR})

set(RTL87X2J_PLATFORM_LIB_FILE
    ${rtl87x2j_platform_lib_build_dir}/lib/librtl87x2j_platform_lib.a)
set(RTL87X2J_PLATFORM_INC_DIR
    ${rtl87x2j_platform_lib_src_dir}/include)

if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
  set(submake "$(MAKE)")
else()
  set(submake "make")
endif()

include(ExternalProject)

ExternalProject_Add(
  rtl87x2j_platform_lib_project
  PREFIX     ${rtl87x2j_platform_lib_build_dir}
  SOURCE_DIR ${rtl87x2j_platform_lib_src_dir}
  BINARY_DIR ${rtl87x2j_platform_lib_src_dir}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND
    ${submake}
    PREFIX=${rtl87x2j_platform_lib_build_dir}
    CC=${CMAKE_C_COMPILER}
    AR=${CMAKE_AR}
    CFLAGS=${rtl87x2j_platform_lib_cflags}
    RTK_BASE=${RTK_BASE}
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS ${RTL87X2J_PLATFORM_LIB_FILE}
)

# ---------------------------------------------------------------------------
# Imported CMake target
# ---------------------------------------------------------------------------
add_library(rtl87x2j_platform_lib STATIC IMPORTED GLOBAL)
add_dependencies(rtl87x2j_platform_lib rtl87x2j_platform_lib_project)

set_target_properties(rtl87x2j_platform_lib PROPERTIES
    IMPORTED_LOCATION             ${RTL87X2J_PLATFORM_LIB_FILE}
    INTERFACE_INCLUDE_DIRECTORIES ${RTL87X2J_PLATFORM_INC_DIR}
)
