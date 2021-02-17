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

function(target_sources_micro_os_plus_semihosting target)

  get_filename_component(xpack_current_folder ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_sources(
    ${target}

    PRIVATE
      ${xpack_current_folder}/src/syscalls-semihosting.cpp
      ${xpack_current_folder}/src/trace-semihosting.cpp
  )

endfunction()

# -----------------------------------------------------------------------------

function(target_include_directories_micro_os_plus_semihosting target)

  get_filename_component(xpack_current_folder ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_include_directories(
    ${target}

    PUBLIC
      ${xpack_current_folder}/include
  )

endfunction()

# -----------------------------------------------------------------------------

function(target_compile_definitions_micro_os_plus_semihosting target)

  # None

endfunction()

# =============================================================================

function(add_libraries_micro_os_plus_semihosting)

  # ---------------------------------------------------------------------------

  find_package(micro-os-plus-diag-trace)

  # ---------------------------------------------------------------------------

  if(NOT TARGET micro-os-plus-semihosting-static)

    add_library(micro-os-plus-semihosting-static STATIC EXCLUDE_FROM_ALL)

    target_sources_micro_os_plus_semihosting(micro-os-plus-semihosting-static)
    target_include_directories_micro_os_plus_semihosting(micro-os-plus-semihosting-static)
    target_compile_definitions_micro_os_plus_semihosting(micro-os-plus-semihosting-static)

    add_library(micro-os-plus::semihosting-static ALIAS micro-os-plus-semihosting-static)
    message(STATUS "micro-os-plus::semihosting-static")

    target_link_libraries(
      micro-os-plus-semihosting-static
      
      PUBLIC
        micro-os-plus::diag-trace-static
        micro-os-plus::architecture
    )

  endif()

  # ---------------------------------------------------------------------------

endfunction()

# -----------------------------------------------------------------------------
