[![license](https://img.shields.io/github/license/micro-os-plus/semihosting-xpack)](https://github.com/micro-os-plus/semihosting-xpack/blob/xpack/LICENSE)
[![CI on Push](https://github.com/micro-os-plus/semihosting-xpack/workflows/CI%20on%20Push/badge.svg)](https://github.com/micro-os-plus/semihosting-xpack/actions?query=workflow%3A%22CI+on+Push%22)

# A source library xPack with the µOS++ semihosting support

This project provides support for semihosting.

It complements the newlib
implementation (arm/librdimon) with an extensive set of POSIX functions,
which also include debugging support.

The project is hosted on GitHub as
[micro-os-plus/semihosting-xpack](https://github.com/micro-os-plus/semihosting-xpack).

## Maintainer info

This page is addressed to developers who plan to include this source
library into their own projects.

For maintainer info, please see the
[README-MAINTAINER](README-MAINTAINER.md) file.

## Install

As a source library xPacks, the easiest way to add it to a project is via
**xpm**, but it can also be used as any Git project, for example as a submodule.

### Prerequisites

A recent [xpm](https://xpack.github.io/xpm/),
which is a portable [Node.js](https://nodejs.org/) command line application.

For details please follow the instructions in the
[xPack install](https://xpack.github.io/install/) page.

### xpm

Note: the package will be available from npmjs.com at a later date.

For now, it can be installed from GitHub:

```sh
cd my-project
xpm init # Unless a package.json is already present

xpm install github:micro-os-plus/semihosting-xpack
```

When ready, this package will be available as
[`@micro-os-plus/semihosting`](https://www.npmjs.com/package/@micro-os-plus/semihosting)
from the `npmjs.com` registry:

```sh
cd my-project
xpm init # Unless a package.json is already present

xpm install @micro-os-plus/semihosting@latest

ls -l xpacks/micro-os-plus-semihosting
```

### Git submodule

If, for any reason, **xpm** is not available, the next recommended
solution is to link it as a Git submodule below an `xpacks` folder.

```sh
cd my-project
git init # Unless already a Git project
mkdir -p xpacks

git submodule add https://github.com/micro-os-plus/semihosting-xpack.git \
  xpacks/micro-os-plus-semihosting
```

## Branches

Apart from the unused `master` branch, there are two active branches:

- `xpack`, with the latest stable version (default)
- `xpack-develop`, with the current development version

All development is done in the `xpack-develop` branch, and contributions via
Pull Requests should be directed to this branch.

When new releases are published, the `xpack-develop` branch is merged
into `xpack`.

## Developer info

This project provides several components, which can be enabled separately:

- the declarations for the basic semihosting calls
- implementations for the system calls
- implementations for the initialisations required during the startup
- implementations for the debug trace channels via semihosting

### Status

The semihosting implementation is fully functional.

### C++ API

The following functions are declared:

```c++
namespace micro_os_plus::semihosting
{
  int
  call_host (int reason, void* arg);
}
```

### C API

All C++ functions have C equivalents:

```c
  static int
  micro_os_plus_semihosting_call_host (int reason, void* arg);

```

### Implementation

The architecture should provide a definition of the following function:

```c
  static inline __attribute__ ((always_inline)) int
  micro_os_plus_semihosting_call_host (int reason, void* arg);
```

### Build & integration info

To include this package in a project, consider the following details.

#### Include folders

The following folders should be passed to the compiler during the build:

- `include`

The header files to be included in user projects are:

```c++
#include <micro-os-plus/semihosting.h>
```

#### Source files

The source files to be added to the build are:

- `src/semihosting-startup.cpp`
- `src/semihosting-syscalls.cpp`
- `src/semihosting-trace.cpp`

#### Preprocessor definitions

- `MICRO_OS_PLUS_INCLUDE_SEMIHOSTING_SYSCALLS`
- `MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES` (20)
- `MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_CHDIR_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_CHMOD_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_CHOWN_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_CLOSEDIR_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_EXECVE_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_FCNTL_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_FORK_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_FSYNC_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_FTRUNCATE_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_IOCTL_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_KILL_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_LINK_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_OPENDIR_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_RAISE_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_READDIR_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_READLINK_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_REWINDDIR_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_RMDIR_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_SELECT_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_SYMLINK_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_SYNC_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_TRUNCATE_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_UTIME_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_WAIT_BRK`
- `MICRO_OS_PLUS_DEBUG_SYSCALL_WRITEV_BRK`
- `MICRO_OS_PLUS_INCLUDE_SEMIHOSTING_STARTUP`
- `MICRO_OS_PLUS_INTEGER_SEMIHOSTING_CMDLINE_ARRAY_SIZE` (80)
- `MICRO_OS_PLUS_INTEGER_SEMIHOSTING_ARGV_ARRAY_SIZE` (10)

#### Compiler options

- `-std=c++20` or higher for C++ sources
- `-std=c11` for C sources

#### C++ Namespaces

- `micro_os_plus::semihosting`

#### C++ Classes

- none

#### Dependencies

- the `@micro-os-plus/diag-trace` package, for the declarations of the
  debug trace channels

#### CMake

To integrate the semihosting source library into a CMake application, add this
folder to the build:

```cmake
add_subdirectory("xpacks/micro-os-plus-semihosting")`
```

The result is an interface library that can be added as an application
dependency with:

```cmake
target_link_libraries(your-target PRIVATE

  micro-os-plus::semihosting
)
```

#### meson

To integrate the µTest++ source library into a meson application, add this
folder to the build:

```meson
subdir('xpacks/micro-os-plus-semihosting')
```

The result is a dependency object that can be added
to an application with:

```meson
exe = executable(
  your-target,
  link_with: [
    # Nothing, not static.
  ],
  dependencies: [
    micro_os_plus_semihosting_dependency,
  ]
)
```

### Examples

TBD

### Known problems

- none

### Tests

TBD

## Change log - incompatible changes

According to [semver](https://semver.org) rules:

> Major version X (X.y.z | X > 0) MUST be incremented if any
backwards incompatible changes are introduced to the public API.

The incompatible changes, in reverse chronological order, are:

- v7.x: get rid of the C++ posix:: namespace
- v6.x: rename source files
- v5.x: rename SYS_GETCMDLINE; use param_block_t
- v4.x: rename MICRO_OS_PLUS_DEBUG, MICRO_OS_PLUS_TRACE
- v3.x: rename the micro_os_plus namespace
- v2.x: update for new OpenOCD implementation; use the posix:: namespace
- v1.x: the code was extracted from the mono-repo µOS++ project

## License

The original content is released under the
[MIT License](https://opensource.org/licenses/MIT/),
with all rights reserved to
[Liviu Ionescu](https://github.com/ilg-ul/).
