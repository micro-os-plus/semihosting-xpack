#
# This file is part of the µOS++ distribution.
#   (https://github.com/micro-os-plus)
# Copyright (c) 2021 Liviu Ionescu
#
# Permission to use, copy, modify, and/or distribute this software
# for any purpose is hereby granted, under the terms of the MIT license.
#
# If a copy of the license was not distributed with this file, it can
# be obtained from https://opensource.org/licenses/MIT/.
#
# -----------------------------------------------------------------------------

# This file is intended to be consumed by applications with:
#
# `add_subdirectory("xpacks/micro-os-plus-semihosting")`
#
# The result is an interface library that can be added to the linker with:
#
# `target_link_libraries(your-target PUBLIC micro-os-plus::semihosting)`

# -----------------------------------------------------------------------------
## Preamble ##

# https://cmake.org/cmake/help/v3.20/
cmake_minimum_required(VERSION 3.20)

project(
  micro-os-plus-semihosting
  DESCRIPTION "µOS++ portable semihosting"
)

if(COMMAND xpack_get_package_name_and_version)
  xpack_get_package_name_and_version("${CMAKE_CURRENT_SOURCE_DIR}/package.json")
  message(VERBOSE "Processing xPack ${PACKAGE_JSON_NAME}@${PACKAGE_JSON_VERSION}...")
endif()

# -----------------------------------------------------------------------------
## The project library definitions ##

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#normal-libraries
# PRIVATE: build definitions, used internally
# INTERFACE: usage definitions, passed up to targets linking to it
# PUBLIC: both

add_library(micro-os-plus-semihosting-interface INTERFACE EXCLUDE_FROM_ALL)

# -----------------------------------------------------------------------------
# Target settings.

target_include_directories(
  micro-os-plus-semihosting-interface

  INTERFACE
    "include"
)

message(VERBOSE "+ -I include")

target_sources(
  micro-os-plus-semihosting-interface

  INTERFACE
    "src/syscalls-semihosting.cpp"
    "src/trace-semihosting.cpp"
)

message(VERBOSE "+ src/syscalls-semihosting.cpp")
message(VERBOSE "+ src/trace-semihosting.cpp")

target_compile_definitions(
  micro-os-plus-semihosting-interface

  INTERFACE
    # ... add preprocessor macro definitions.
)

target_compile_options(
  micro-os-plus-semihosting-interface

  INTERFACE
    # ... perhaps ignore some warnings.
)

target_link_libraries(
  micro-os-plus-semihosting-interface

  INTERFACE
    # Dependencies
    micro-os-plus::diag-trace
    micro-os-plus::architecture
)

# -----------------------------------------------------------------------------
# Aliases.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library(micro-os-plus::semihosting ALIAS micro-os-plus-semihosting-interface)
message(VERBOSE "> micro-os-plus::semihosting -> micro-os-plus-semihosting-interface")

# -----------------------------------------------------------------------------
