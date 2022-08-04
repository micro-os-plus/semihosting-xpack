# Change & release log

Releases in reverse chronological order.

Please check
[GitHub](https://github.com/micro-os-plus/semihosting-xpack/issues/)
and close existing issues and pull requests.

## 2022-08-04

* v8.0.0
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

2022-07-24 * bf2f2be xpack.json: add missing *-brk options
2022-07-24 * 61de2db xpack.json: no more camelCase for option names
2022-07-24 * 3b41c6f #6: rename startup cmdline & argv
2022-07-24 * 6dd77bb #5: remove socket functions

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
