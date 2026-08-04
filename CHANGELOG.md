# Change & release log

Releases in reverse chronological order.

Please check
[GitHub](https://github.com/micro-os-plus/semihosting-xpack/issues/)
and close existing issues and pull requests.

## 2026-08-04

* v9.0.0
* 41bdbaa re-generate top commons

## 2026-07-31

* 8a96255 CHANGELOG update
* 1b59bb8 xcdl-package.jsonc update
* d176686 re-generate top xcdl
* 2c59b57 re-format clang
* 090665b re-generate top commons
* 27d7fde cosmetise guards & includes
* 104da62 copyright notices update

## 2026-07-20

* 91a8d7c prepare 9.0.0
* 546cb3b xcdl-package.jsonc cosmetics
* 970c31b src cosmetics

## 2026-07-19

* 3eda256 xcdl-package.jsonc update
* d37fec6 re-grnerate top commons
* 56d34ac xcdl-package.jsonc update

## 2026-07-18

* 988cfdf README update
* f4ac427 xcdl-package.jsonc rework & xcdl export
* c4ade51 update for xcdl defines

## 2026-07-17

* bd3f06d re-generate top commons
* adf3bff re-generate top commons & export xcdl
* f323470 package*.json update
* 569ea45 csrc: cleaups in includes, rely on semihosting.h
* e58d6a9 semihosting.h: add semihosting-defines.h

## 2026-07-16

* 027473f re-generate top commons

## 2026-06-11

* 4f7b083 prefer project-config.h

## 2026-06-06

* e027777 use __has_include(<micro-os-plus/config.h>)
* ce89d69 copyright update 2026

## 2025-11-20

* f28a25c rename npm-pack

## 2025-10-07

* d7c19a5 update copyright notices

## 2023-11-28

* b01e83f README updates
* 52e2a3d package.json: cosmetise scripts

## 2023-10-19

* cd4d48a package.json: cosmetise description
* 851fc04 README updates

## 2023-07-14

* 2110bfc README updates
* 7dd61b1 package.json: cosmetise urls
* 9039269 package.json: minXpm 0.16.2

## 2023-06-04

* 2cc2d5b update for @scope/name

## 2023-06-03

* 52c2e29 lower case ci.yml
* 269f010 package.json min 0.16.0

## 2023-05-08

* 53878da .npmignore update
* 5699fe8 README updates
* 9ccabf6 README updates
* 17c2d71 meson.build cosmetics

## 2022-08-16

* 5ce09b1 package.json min 0.14.0 & defaults

## 2022-08-04

* b7921ae 8.0.0
* 339a29e prepare v8.0.0
* 4425dc3 rename MICRO_OS_PLUS_INTEGER_TRACE_SEMIHOSTING_BUFFER_ARRAY_SIZE
* d11f17e .vscode/settings.json: cmake.ignoreCMakeListsMissing
* 6942748 .vscode/settings.json: makefile.configureOnOpen

## 2022-07-28

* v7.1.0
* 38d818b #7: rework, implement the newlib underscore functions
* 94c139c semihosting.h: add feature magic numbers
* 56799d5 update response to signed_register_t

## 2022-07-25

* be0bea9 semihosting.h: explain C declaration not here
* 22c9f3d add preliminary xpack.json

## 2022-07-24

## 2022-07-24

* bf2f2be xpack.json: add missing *-brk options
* 61de2db xpack.json: no more camelCase for option names
* 3b41c6f #6: rename startup cmdline & argv
* 6dd77bb #5: remove socket functions

## 2022-07-23

* v7.0.0 (not published on npmjs)
* 80c09e8 add experimental xpack.json
* 430bbbb #4: get rid of the C++ posix namespace

## 2022-07-22

* v6.0.0 (not published on npmjs)
* 209fe7c reformat namespaces
* 0e96151 split semihosting-startup.cpp
* d0c8d25 rename semihosting-trace.cpp

## 2022-05-24

* v5.0.0
* 25123f9 use param_block_t; rename SYS_GETCMDLINE
* 2a7e329 semihosting.h: add extra numbers

## 2022-02-15

* v4.0.0
* f815aee trace-semihosting.cpp: add flush()

## 2022-02-10

* 0eb2ebb .c* include <micro-os-plus/config.h>

## 2022-02-09

* 7a079af rename MICRO_OS_PLUS_DEBUG
* 0a59d26 rename MICRO_OS_PLUS_TRACE

## 2022-02-05

* v3.2.1

## 2022-02-04

* v3.2.0
* 2a3d08b cmake: add xpack_display_target_lists

## 2022-02-02

* edadab4 add meson support
* b9bbe9b trace-semihosting.cpp: fix casts

## 2022-01-28

* 2072ae4 rework CMake, top config, no sources

## 2022-01-27

* 99fd5b7 syscalls: move initialise_monitor_handles to startup

## 2022-01-26

* v3.1.0
* rework CMake

## 2022-01-02

* v3.0.3
* avoid select alias, it fails with arm-none-eabi-gcc 10.3 on Ubuntu

## 2022-01-02

* v3.0.2

## 2021-03-09

* v3.0.1
* fix/silence warnings

## 2021-03-01

* v3.0.0

## 2021-02-28

* rename micro_os_plus

## 2021-02-04

* v2.1.0
* first restructured release

## 2018-04-16

* v2.0.2
* cleanup deps
* update README developer info

##  2018-01-25

* v2.0.1
* [#3] use void* field_t

## 2018-01-24

* v2.0.0

## 2018-01-23

* Use the posix:: namespace
* add debug breaks for unimplemented calls

## 2018-01-21

* [#1] Update semihosting after OpenOCD new definitions; remove `report_exception()`
* [#2] Add syscall functions

## 2018-01-15

* v1.0.0
* initial version, copied from µOS++.
