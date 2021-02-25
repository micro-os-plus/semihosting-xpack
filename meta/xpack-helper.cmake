#
# This file is part of the µOS++ distribution.
#   (https://github.com/micro-os-plus)
# Copyright (c) 2021 Liviu Ionescu
#
# This Source Code Form is subject to the terms of the MIT License.
# If a copy of the license was not distributed with this file, it can
# be obtained from https://opensource.org/licenses/MIT/.
#
# -----------------------------------------------------------------------------

if(micro-os-plus-semihosting-included)
  return()
endif()

set(micro-os-plus-semihosting-included TRUE)

message(STATUS "Including micro-os-plus-semihosting...")

# -----------------------------------------------------------------------------
# Dependencies.

find_package(micro-os-plus-diag-trace REQUIRED)

# -----------------------------------------------------------------------------
# The current folder.

get_filename_component(xpack_current_folder ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

# -----------------------------------------------------------------------------

if(NOT TARGET micro-os-plus-semihosting-static)

  add_library(micro-os-plus-semihosting-static STATIC EXCLUDE_FROM_ALL)

  # ---------------------------------------------------------------------------

  target_sources(
    micro-os-plus-semihosting-static

    PRIVATE
      ${xpack_current_folder}/src/syscalls-semihosting.cpp
      ${xpack_current_folder}/src/trace-semihosting.cpp
  )

  target_include_directories(
    micro-os-plus-semihosting-static

    PUBLIC
      ${xpack_current_folder}/include
  )

  target_link_libraries(
    micro-os-plus-semihosting-static
    
    PUBLIC
      micro-os-plus::diag-trace-static
      micro-os-plus::architecture
  )

  # ---------------------------------------------------------------------------
  # Aliases.

  add_library(micro-os-plus::semihosting-static ALIAS micro-os-plus-semihosting-static)
  message(STATUS "micro-os-plus::semihosting-static")

endif()

# -----------------------------------------------------------------------------
