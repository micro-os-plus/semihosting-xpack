/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus/)
 * Copyright (c) 2018 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

#ifndef MICRO_OS_PLUS_SEMIHOSTING_H_
#define MICRO_OS_PLUS_SEMIHOSTING_H_

// ----------------------------------------------------------------------------

#include <micro-os-plus/architecture.h>

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

  enum semihosting_operation_numbers
  {
    /*
     * ARM semihosting operations, in lexicographic order.
     */
    SEMIHOSTING_ENTER_SVC = 0x17, /* DEPRECATED */

    SEMIHOSTING_SYS_CLOSE = 0x02,
    SEMIHOSTING_SYS_CLOCK = 0x10,
    SEMIHOSTING_SYS_ELAPSED = 0x30,
    SEMIHOSTING_SYS_ERRNO = 0x13,
    SEMIHOSTING_SYS_EXIT = 0x18,
    SEMIHOSTING_SYS_EXIT_EXTENDED = 0x20,
    SEMIHOSTING_SYS_FLEN = 0x0C,
    SEMIHOSTING_SYS_GETCMDLINE = 0x15,
    SEMIHOSTING_SYS_HEAPINFO = 0x16,
    SEMIHOSTING_SYS_ISERROR = 0x08,
    SEMIHOSTING_SYS_ISTTY = 0x09,
    SEMIHOSTING_SYS_OPEN = 0x01,
    SEMIHOSTING_SYS_READ = 0x06,
    SEMIHOSTING_SYS_READC = 0x07,
    SEMIHOSTING_SYS_REMOVE = 0x0E,
    SEMIHOSTING_SYS_RENAME = 0x0F,
    SEMIHOSTING_SYS_REPORTEXCEPTION = 0x18, // TODO
    SEMIHOSTING_SYS_REPORTEXCEPTIONEXTENDED = 0x20, // TODO
    SEMIHOSTING_SYS_SEEK = 0x0A,
    SEMIHOSTING_SYS_SYSTEM = 0x12,
    SEMIHOSTING_SYS_SYNCCACHERANGE = 0x19, // TODO
    SEMIHOSTING_SYS_TICKFREQ = 0x31,
    SEMIHOSTING_SYS_TIME = 0x11,
    SEMIHOSTING_SYS_TMPNAM = 0x0D,
    SEMIHOSTING_SYS_WRITE = 0x05,
    SEMIHOSTING_SYS_WRITEC = 0x03,
    SEMIHOSTING_SYS_WRITE0 = 0x04,
  };

  /*
   * Codes used by SEMIHOSTING_SYS_EXIT (formerly
   * SEMIHOSTING_REPORT_EXCEPTION).
   * On 64-bits, the exit code is passed explicitly.
   */
  enum semihosting_reported_exceptions
  {
    /* On 32 bits, use it for exit(0) */
    ADP_STOPPED_APPLICATION_EXIT = ((2 << 16) + 38),
    /* On 32 bits, use it for exit(1) */
    ADP_STOPPED_RUN_TIME_ERROR = ((2 << 16) + 35),
  };

  //
  enum semihosting_feature_magic_numbers
  {
    NUM_SHFB_MAGIC = 4,
    SHFB_MAGIC_0 = 0x53,
    SHFB_MAGIC_1 = 0x48,
    SHFB_MAGIC_2 = 0x46,
    SHFB_MAGIC_3 = 0x42
  };

  enum semihosting_extensions
  {
    SH_EXT_EXIT_EXTENDED_BITNUM = 0x0,
    SH_EXT_STDOUT_STDERR_BITNUM = 0x1
  };

  // ----------------------------------------------------------------------------

  // To allow for static inline optimizations, this definition is actually not
  // in this package, but in the architecture semihosting-inlines.h file.

  // static micro_os_plus_semihosting_response_t
  // micro_os_plus_semihosting_call_host (
  //    int reason,
  //    micro_os_plus_semihosting_param_block_t* arg);

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

#if defined(__cplusplus)

// ----------------------------------------------------------------------------

namespace micro_os_plus::semihosting
{
  // --------------------------------------------------------------------------
  // Portable semihosting functions in C++.

  typedef micro_os_plus::architecture::register_t param_block_t;
  typedef micro_os_plus::architecture::signed_register_t response_t;

  response_t
  call_host (int reason, param_block_t* arg);

  // --------------------------------------------------------------------------
} // namespace micro_os_plus::semihosting

#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

// Include the portable definitions.
#include <micro-os-plus/semihosting-inlines.h>

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_SEMIHOSTING_H_

// ----------------------------------------------------------------------------
