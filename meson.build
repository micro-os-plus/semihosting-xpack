# -----------------------------------------------------------------------------
# DO NOT EDIT! Automatically generated from template file:
# xcdl-package.jsonc
#
# This file is part of the µOS++ project (https://micro-os-plus.github.io/).
# Copyright (c) 2022-2026 Liviu Ionescu. All rights reserved.
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
# `subdir('xpacks/@micro-os-plus/semihosting')`
#
# This creates dependencies that can be linked to a target with:
#
# `dependencies: [micro_os_plus_semihosting_startup_dependency],`
# `dependencies: [micro_os_plus_semihosting_syscalls_dependency],`
# `dependencies: [micro_os_plus_diag_trace_semihosting_debug_dependency],`
# `dependencies: [micro_os_plus_diag_trace_semihosting_stdout_dependency],`
# `dependencies: [micro_os_plus_diag_trace_semihosting_dependency],`
# `dependencies: [micro_os_plus_semihosting_dependency],`

# Note: the Meson configuration is provided only as a proof of concept;
# for production it might need some refinements.

# -----------------------------------------------------------------------------

# A source code library with the µOS++ portable semihosting definitions.
message('Processing xpm package @micro-os-plus/semihosting...')

# -----------------------------------------------------------------------------

# Meson's DSL does not provide introspection methods on dependency objects
# to retrieve their properties after creation. The `_local_*` variables
# therefore serve a dual purpose: populating the dependency object and
# iterating over its components for the `message()` calls at the end.

_local_compile_args = [] # Common C/C++ args.
_local_compile_c_args = [] # C only args.
_local_compile_cpp_args = [] # C++ only args.
_local_include_directories = []
_local_sources = []
_local_compile_definitions = []
_local_dependencies = []
_local_link_args = []
_local_link_with = []

_local_sources += [
  'src/semihosting-startup.cpp',
]

# https://mesonbuild.com/Reference-manual_functions.html#declare_dependency
micro_os_plus_semihosting_startup_dependency = declare_dependency(
  include_directories: include_directories(_local_include_directories),
  compile_args: _local_compile_args,
  sources: files(_local_sources),
  dependencies: _local_dependencies,
  link_args: _local_link_args,
  link_with: _local_link_with,
)

# Meson dependencies cannot differentiate between C and C++ compile args;
# they are therefore exposed as separate variables alongside the dependency.
micro_os_plus_semihosting_startup_dependency_compile_c_args = _local_compile_c_args
micro_os_plus_semihosting_startup_dependency_compile_cpp_args = _local_compile_cpp_args

foreach name : _local_include_directories
  message('+ -I ' + name)
endforeach
foreach name : _local_sources + _local_compile_definitions + _local_compile_args + _local_compile_c_args + _local_compile_cpp_args
  message('+ ' + name)
endforeach
message('> micro_os_plus_semihosting_startup_dependency')

# -----------------------------------------------------------------------------

_local_compile_args = [] # Common C/C++ args.
_local_compile_c_args = [] # C only args.
_local_compile_cpp_args = [] # C++ only args.
_local_include_directories = []
_local_sources = []
_local_compile_definitions = []
_local_dependencies = []
_local_link_args = []
_local_link_with = []

_local_sources += [
  'src/semihosting-syscalls.cpp',
]

# https://mesonbuild.com/Reference-manual_functions.html#declare_dependency
micro_os_plus_semihosting_syscalls_dependency = declare_dependency(
  include_directories: include_directories(_local_include_directories),
  compile_args: _local_compile_args,
  sources: files(_local_sources),
  dependencies: _local_dependencies,
  link_args: _local_link_args,
  link_with: _local_link_with,
)

# Meson dependencies cannot differentiate between C and C++ compile args;
# they are therefore exposed as separate variables alongside the dependency.
micro_os_plus_semihosting_syscalls_dependency_compile_c_args = _local_compile_c_args
micro_os_plus_semihosting_syscalls_dependency_compile_cpp_args = _local_compile_cpp_args

foreach name : _local_include_directories
  message('+ -I ' + name)
endforeach
foreach name : _local_sources + _local_compile_definitions + _local_compile_args + _local_compile_c_args + _local_compile_cpp_args
  message('+ ' + name)
endforeach
message('> micro_os_plus_semihosting_syscalls_dependency')

# -----------------------------------------------------------------------------

_local_compile_args = [] # Common C/C++ args.
_local_compile_c_args = [] # C only args.
_local_compile_cpp_args = [] # C++ only args.
_local_include_directories = []
_local_sources = []
_local_compile_definitions = []
_local_dependencies = []
_local_link_args = []
_local_link_with = []

# https://mesonbuild.com/Reference-manual_functions.html#declare_dependency
micro_os_plus_diag_trace_semihosting_debug_dependency = declare_dependency(
  include_directories: include_directories(_local_include_directories),
  compile_args: _local_compile_args,
  sources: files(_local_sources),
  dependencies: _local_dependencies,
  link_args: _local_link_args,
  link_with: _local_link_with,
)

# Meson dependencies cannot differentiate between C and C++ compile args;
# they are therefore exposed as separate variables alongside the dependency.
micro_os_plus_diag_trace_semihosting_debug_dependency_compile_c_args = _local_compile_c_args
micro_os_plus_diag_trace_semihosting_debug_dependency_compile_cpp_args = _local_compile_cpp_args

foreach name : _local_include_directories
  message('+ -I ' + name)
endforeach
foreach name : _local_sources + _local_compile_definitions + _local_compile_args + _local_compile_c_args + _local_compile_cpp_args
  message('+ ' + name)
endforeach
message('> micro_os_plus_diag_trace_semihosting_debug_dependency')

# -----------------------------------------------------------------------------

_local_compile_args = [] # Common C/C++ args.
_local_compile_c_args = [] # C only args.
_local_compile_cpp_args = [] # C++ only args.
_local_include_directories = []
_local_sources = []
_local_compile_definitions = []
_local_dependencies = []
_local_link_args = []
_local_link_with = []

# https://mesonbuild.com/Reference-manual_functions.html#declare_dependency
micro_os_plus_diag_trace_semihosting_stdout_dependency = declare_dependency(
  include_directories: include_directories(_local_include_directories),
  compile_args: _local_compile_args,
  sources: files(_local_sources),
  dependencies: _local_dependencies,
  link_args: _local_link_args,
  link_with: _local_link_with,
)

# Meson dependencies cannot differentiate between C and C++ compile args;
# they are therefore exposed as separate variables alongside the dependency.
micro_os_plus_diag_trace_semihosting_stdout_dependency_compile_c_args = _local_compile_c_args
micro_os_plus_diag_trace_semihosting_stdout_dependency_compile_cpp_args = _local_compile_cpp_args

foreach name : _local_include_directories
  message('+ -I ' + name)
endforeach
foreach name : _local_sources + _local_compile_definitions + _local_compile_args + _local_compile_c_args + _local_compile_cpp_args
  message('+ ' + name)
endforeach
message('> micro_os_plus_diag_trace_semihosting_stdout_dependency')

# -----------------------------------------------------------------------------

_local_compile_args = [] # Common C/C++ args.
_local_compile_c_args = [] # C only args.
_local_compile_cpp_args = [] # C++ only args.
_local_include_directories = []
_local_sources = []
_local_compile_definitions = []
_local_dependencies = []
_local_link_args = []
_local_link_with = []

_local_sources += [
  'src/diag/trace-semihosting.cpp',
]

_local_dependencies += [
  micro_os_plus_diag_trace_dependency,
  micro_os_plus_diag_trace_semihosting_debug_dependency,
  micro_os_plus_diag_trace_semihosting_stdout_dependency,
]

# https://mesonbuild.com/Reference-manual_functions.html#declare_dependency
micro_os_plus_diag_trace_semihosting_dependency = declare_dependency(
  include_directories: include_directories(_local_include_directories),
  compile_args: _local_compile_args,
  sources: files(_local_sources),
  dependencies: _local_dependencies,
  link_args: _local_link_args,
  link_with: _local_link_with,
)

# Meson dependencies cannot differentiate between C and C++ compile args;
# they are therefore exposed as separate variables alongside the dependency.
micro_os_plus_diag_trace_semihosting_dependency_compile_c_args = _local_compile_c_args
micro_os_plus_diag_trace_semihosting_dependency_compile_cpp_args = _local_compile_cpp_args

foreach name : _local_include_directories
  message('+ -I ' + name)
endforeach
foreach name : _local_sources + _local_compile_definitions + _local_compile_args + _local_compile_c_args + _local_compile_cpp_args
  message('+ ' + name)
endforeach
message('> micro_os_plus_diag_trace_semihosting_dependency')

# -----------------------------------------------------------------------------

_local_compile_args = [] # Common C/C++ args.
_local_compile_c_args = [] # C only args.
_local_compile_cpp_args = [] # C++ only args.
_local_include_directories = []
_local_sources = []
_local_compile_definitions = []
_local_dependencies = []
_local_link_args = []
_local_link_with = []

_local_include_directories += [
  'include',
]

_local_dependencies += [
  micro_os_plus_diag_trace_dependency,
  micro_os_plus_diag_trace_semihosting_dependency,
  micro_os_plus_architecture_dependency,
  micro_os_plus_semihosting_startup_dependency,
  micro_os_plus_semihosting_syscalls_dependency,
]

# https://mesonbuild.com/Reference-manual_functions.html#declare_dependency
micro_os_plus_semihosting_dependency = declare_dependency(
  include_directories: include_directories(_local_include_directories),
  compile_args: _local_compile_args,
  sources: files(_local_sources),
  dependencies: _local_dependencies,
  link_args: _local_link_args,
  link_with: _local_link_with,
)

# Meson dependencies cannot differentiate between C and C++ compile args;
# they are therefore exposed as separate variables alongside the dependency.
micro_os_plus_semihosting_dependency_compile_c_args = _local_compile_c_args
micro_os_plus_semihosting_dependency_compile_cpp_args = _local_compile_cpp_args

foreach name : _local_include_directories
  message('+ -I ' + name)
endforeach
foreach name : _local_sources + _local_compile_definitions + _local_compile_args + _local_compile_c_args + _local_compile_cpp_args
  message('+ ' + name)
endforeach
message('> micro_os_plus_semihosting_dependency')

# -----------------------------------------------------------------------------
