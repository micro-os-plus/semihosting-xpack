# -----------------------------------------------------------------------------
# DO NOT EDIT! Automatically generated from xcdl-package.jsonc.
#
# This file is part of the µOS++ project (https://micro-os-plus.github.io/).
# Copyright (c) 2021-2026 Liviu Ionescu. All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose is hereby granted, under the terms of the MIT license.
#
# If a copy of the license was not distributed with this file, it can be
# obtained from https://opensource.org/licenses/mit.
#
# -----------------------------------------------------------------------------

# This file is intended to be used by applications via:
#
# `add_subdirectory("xpacks/@micro-os-plus/semihosting")`
#
# This creates libraries that can be linked to a target with:
#
# ~~~
# target_link_libraries(your-target PUBLIC micro-os-plus::semihosting-startup)
# target_link_libraries(your-target PUBLIC micro-os-plus::semihosting-syscalls)
# target_link_libraries(your-target PUBLIC micro-os-plus::semihosting-trace-debug)
# target_link_libraries(your-target PUBLIC micro-os-plus::semihosting-trace-stdout)
# target_link_libraries(your-target PUBLIC micro-os-plus::semihosting-trace)
# target_link_libraries(your-target PUBLIC micro-os-plus::semihosting)
# ~~~

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/
cmake_minimum_required (VERSION 3.20)
project (
  micro-os-plus-semihosting
  DESCRIPTION
    "A source code library with the µOS++ portable semihosting definitions"
  LANGUAGES CXX
)

# To access the optional functions, include the
# `cmake/micro-os-plus-build-helper.cmake` file from the
# `@micro-os-plus/build-helper` package.

if (COMMAND xpack_get_package_name_and_version)
  xpack_get_package_name_and_version (
    "${CMAKE_CURRENT_SOURCE_DIR}/package.json"
  )
  message (VERBOSE
           "Processing xPack ${PACKAGE_JSON_NAME}@${PACKAGE_JSON_VERSION}..."
  )
endif ()

# -----------------------------------------------------------------------------

# Interface libraries are the main targets, which consumers (like embedded
# applications) link against. It defines the public API and properties of the
# library; sources are propagated to and compiled by each consumer.

# They are aliased using namespaced syntax (Project::Target). This is the
# canonical pattern for consumption via target_link_libraries(), whether the
# dependency is added via add_subdirectory(), FetchContent, or an installed
# package via find_package(). Using "::" lets CMake detect an invalid or
# misspelled target name at configure time, rather than silently passing it
# through to the linker as a plain string or flag. Note: alias targets are
# read-only; properties must be set on the underlying target, not on the alias
# itself.

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (
  micro-os-plus-semihosting-startup-interface INTERFACE EXCLUDE_FROM_ALL
)

target_sources (
  micro-os-plus-semihosting-startup-interface
  INTERFACE "src/semihosting-startup.cpp"
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (micro-os-plus-semihosting-startup-interface)
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::semihosting-startup ALIAS
  micro-os-plus-semihosting-startup-interface
)
message (
  VERBOSE
  "> micro-os-plus::semihosting-startup -> micro-os-plus-semihosting-startup-interface"
)

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (
  micro-os-plus-semihosting-syscalls-interface INTERFACE EXCLUDE_FROM_ALL
)

target_sources (
  micro-os-plus-semihosting-syscalls-interface
  INTERFACE "src/semihosting-syscalls.cpp"
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (micro-os-plus-semihosting-syscalls-interface)
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::semihosting-syscalls ALIAS
  micro-os-plus-semihosting-syscalls-interface
)
message (
  VERBOSE
  "> micro-os-plus::semihosting-syscalls -> micro-os-plus-semihosting-syscalls-interface"
)

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (
  micro-os-plus-semihosting-trace-debug-interface INTERFACE EXCLUDE_FROM_ALL
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (micro-os-plus-semihosting-trace-debug-interface)
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::semihosting-trace-debug ALIAS
  micro-os-plus-semihosting-trace-debug-interface
)
message (
  VERBOSE
  "> micro-os-plus::semihosting-trace-debug -> micro-os-plus-semihosting-trace-debug-interface"
)

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (
  micro-os-plus-semihosting-trace-stdout-interface INTERFACE EXCLUDE_FROM_ALL
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (micro-os-plus-semihosting-trace-stdout-interface)
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::semihosting-trace-stdout ALIAS
  micro-os-plus-semihosting-trace-stdout-interface
)
message (
  VERBOSE
  "> micro-os-plus::semihosting-trace-stdout -> micro-os-plus-semihosting-trace-stdout-interface"
)

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (
  micro-os-plus-semihosting-trace-interface INTERFACE EXCLUDE_FROM_ALL
)

target_sources (
  micro-os-plus-semihosting-trace-interface
  INTERFACE "src/diag/trace-semihosting.cpp"
)

target_link_libraries (
  micro-os-plus-semihosting-trace-interface
  INTERFACE "micro-os-plus::diag-trace"
            "micro-os-plus::semihosting-trace-debug"
            "micro-os-plus::semihosting-trace-stdout"
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (micro-os-plus-semihosting-trace-interface)
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::semihosting-trace ALIAS
  micro-os-plus-semihosting-trace-interface
)
message (
  VERBOSE
  "> micro-os-plus::semihosting-trace -> micro-os-plus-semihosting-trace-interface"
)

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (micro-os-plus-semihosting-interface INTERFACE EXCLUDE_FROM_ALL)

target_include_directories (
  micro-os-plus-semihosting-interface INTERFACE "include"
)

target_link_libraries (
  micro-os-plus-semihosting-interface
  INTERFACE "micro-os-plus::diag-trace"
            "micro-os-plus::architecture"
            "micro-os-plus::semihosting-startup"
            "micro-os-plus::semihosting-syscalls"
            "micro-os-plus::semihosting-trace"
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (micro-os-plus-semihosting-interface)
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::semihosting ALIAS micro-os-plus-semihosting-interface
)
message (VERBOSE
         "> micro-os-plus::semihosting -> micro-os-plus-semihosting-interface"
)

# -----------------------------------------------------------------------------
