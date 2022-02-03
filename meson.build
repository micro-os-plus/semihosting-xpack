#
# This file is part of the µOS++ distribution.
#   (https://github.com/micro-os-plus)
# Copyright (c) 2022 Liviu Ionescu
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
# `subdir('xpacks/micro-os-plus-semihosting')`
#
# The result is a dependency that can be refered as:
#
# `dependencies: [micro_os_plus_semihosting_dependency],`

# Note: the meson configuration is provided only as a proof of concept,
# for production it might need some refinements.

# -----------------------------------------------------------------------------

message('Processing xPack @micro-os-plus/semihosting...')

# https://mesonbuild.com/Reference-manual_functions.html#declare_dependency
micro_os_plus_semihosting_dependency = declare_dependency(
  include_directories: include_directories(
    'include',
  ),
  sources: files(
    'src/syscalls-semihosting.cpp',
    'src/trace-semihosting.cpp'
  ),
  dependencies: [
    micro_os_plus_architecture_dependency,
    micro_os_plus_diag_trace_dependency,
  ],
)

message('+ -I include')
message('+ src/syscalls-semihosting.cpp')
message('+ src/trace-semihosting.cpp')
message('> micro_os_plus_semihosting_dependency')

# -----------------------------------------------------------------------------
