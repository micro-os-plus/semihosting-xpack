/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2018 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MICRO_OS_PLUS_SEMIHOSTING_H_
#define MICRO_OS_PLUS_SEMIHOSTING_H_

#if defined(__cplusplus)
extern "C"
{
#endif /* defined(__cplusplus) */

  // Semihosting operations.
  // They should be the same on all architectures.
  enum os_semihosting_operation_number
  {
    // Names preserve the original case from the ARM manual.

    // Regular operations.
    SEMIHOSTING_EnterSVC = 0x17,
    SEMIHOSTING_ReportException = 0x18,
    SEMIHOSTING_SYS_CLOSE = 0x02,
    SEMIHOSTING_SYS_CLOCK = 0x10,
    SEMIHOSTING_SYS_ELAPSED = 0x30,
    SEMIHOSTING_SYS_ERRNO = 0x13,
    SEMIHOSTING_SYS_FLEN = 0x0C,
    SEMIHOSTING_SYS_GET_CMDLINE = 0x15,
    SEMIHOSTING_SYS_HEAPINFO = 0x16,
    SEMIHOSTING_SYS_ISERROR = 0x08,
    SEMIHOSTING_SYS_ISTTY = 0x09,
    SEMIHOSTING_SYS_OPEN = 0x01,
    SEMIHOSTING_SYS_READ = 0x06,
    SEMIHOSTING_SYS_READC = 0x07,
    SEMIHOSTING_SYS_REMOVE = 0x0E,
    SEMIHOSTING_SYS_RENAME = 0x0F,
    SEMIHOSTING_SYS_SEEK = 0x0A,
    SEMIHOSTING_SYS_SYSTEM = 0x12,
    SEMIHOSTING_SYS_TICKFREQ = 0x31,
    SEMIHOSTING_SYS_TIME = 0x11,
    SEMIHOSTING_SYS_TMPNAM = 0x0D,
    SEMIHOSTING_SYS_WRITE = 0x05,
    SEMIHOSTING_SYS_WRITEC = 0x03,
    SEMIHOSTING_SYS_WRITE0 = 0x04,

    // Codes returned by SEMIHOSTING_ReportException.
    ADP_Stopped_ApplicationExit = ((2 << 16) + 38),
    ADP_Stopped_RunTimeError = ((2 << 16) + 35),

  };

// ----------------------------------------------------------------------------

  static int
  os_semihosting_call_host (int reason, void* arg);

  // Function used in _exit() to return the status code as Angel exception.
  static void
  os_semihosting_report_exception (int reason);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

// ----------------------------------------------------------------------------

#if defined(__cplusplus)

namespace os
{
  namespace semihosting
  {
    // ------------------------------------------------------------------------
    // Portable semihosting functions in C++.

    int
    call_host (int reason, void* arg);

    void
    report_exception (int reason);

  // --------------------------------------------------------------------------
  } /* namespace semihosting */
// ----------------------------------------------------------------------------
} /* namespace os */

#endif /* defined(__cplusplus) */

// ----------------------------------------------------------------------------

// Include the architecture specific definitions, which
// include `call_host()`.
#include <micro-os-plus/architecture.h>

// Include the portable definitions.
#include <micro-os-plus/semihosting-inlines.h>

// ----------------------------------------------------------------------------

#endif /* MICRO_OS_PLUS_SEMIHOSTING_H_ */
