/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2015 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

#if (!(defined(__APPLE__) || defined(__linux__) || defined(__unix__))) \
    || defined(__DOXYGEN__)

// ----------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_TRACE)

#if defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_DEBUG) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT)

#include <micro-os-plus/diag/trace.h>

// ----------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS)
#error \
    "Cannot debug semihosting using semihosting trace; use MICRO_OS_PLUS_USE_TRACE_ITM"
#endif

#include <micro-os-plus/semihosting.h>

// ----------------------------------------------------------------------------

using namespace micro_os_plus;

// ----------------------------------------------------------------------------

namespace micro_os_plus
{
  namespace trace
  {
    // ------------------------------------------------------------------------

    void
    initialize (void)
    {
      // For semihosting, no inits are required.
    }

    // ------------------------------------------------------------------------

    // Semihosting is another output channel that can be used for the trace
    // messages. It comes in two flavours: STDOUT and DEBUG. The STDOUT channel
    // is the equivalent of the stdout file in POSIX and in most cases it is
    // forwarded to the GDB server stdout stream. The debug channel is a
    // separate channel. STDOUT is buffered, so nothing is displayed until
    // a \n; DEBUG is not buffered, but can be slow.
    //
    // Choosing between semihosting stdout and debug depends on the
    // capabilities of your GDB server, and also on specific needs. It is
    // recommended to test DEBUG first, and if too slow, try STDOUT.
    //
    // The JLink GDB server fully support semihosting, and both configurations
    // are available; to activate it, use "monitor semihosting enable" or check
    // the corresponding button in the JLink Debugging plug-in.
    // In OpenOCD, support for semihosting can be enabled using
    // "monitor arm semihosting enable".
    //
    // Note: Applications built with semihosting output active normally cannot
    // be executed without the debugger connected and active, since they use
    // BKPT to communicate with the host. However, with a carefully written
    // HardFault_Handler, the semihosting BKPT calls can be processed, making
    // possible to run semihosting applications as standalone, without being
    // terminated with hardware faults.

    // ------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_DEBUG)

#if !defined(MICRO_OS_PLUS_INTEGER_TRACE_SEMIHOSTING_BUFF_ARRAY_SIZE)
#define MICRO_OS_PLUS_INTEGER_TRACE_SEMIHOSTING_BUFF_ARRAY_SIZE (16)
#endif

    ssize_t
    write (const void* buf, std::size_t nbyte)
    {
      if (buf == nullptr || nbyte == 0)
        {
          return 0;
        }

      const char* cbuf = static_cast<const char*> (buf);

      // Since the single character debug channel is quite slow, try to
      // optimize and send a null terminated string, if possible.
      if (cbuf[nbyte] == '\0')
        {
          // send string
          semihosting::call_host (
              SEMIHOSTING_SYS_WRITE0,
              static_cast<void*> (const_cast<char*> (cbuf)));
        }
      else
        {
          // If not, use a local buffer to speed things up.
          // For re-entrance, this bugger must be allocated on the stack,
          // so be cautious with the size.
          char tmp[MICRO_OS_PLUS_INTEGER_TRACE_SEMIHOSTING_BUFF_ARRAY_SIZE];
          size_t togo = nbyte;
          while (togo > 0)
            {
              std::size_t n
                  = ((togo < sizeof (tmp)) ? togo : sizeof (tmp) - 1);
              std::size_t i = 0;
              for (; i < n; ++i, ++cbuf)
                {
                  tmp[i] = *cbuf;
                }
              tmp[i] = '\0';

              semihosting::call_host (SEMIHOSTING_SYS_WRITE0,
                                      static_cast<void*> (tmp));

              togo -= n;
            }
        }

      // All bytes written.
      return static_cast<ssize_t> (nbyte);
    }

#elif defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT)

    ssize_t
    write (const void* buf, std::size_t nbyte)
    {
      if (buf == nullptr || nbyte == 0)
        {
          return 0;
        }

      static int handle; // STATIC!

      // All fields should be large enough to hold a pointer.
      using field_t = void*;
      field_t fields[3];
      int ret;

      if (handle == 0)
        {
          // On the very first call get the file handle from the host.

          // Special filename for stdin/out/err.
          fields[0] = const_cast<char*> (":tt");
          fields[1] = reinterpret_cast<field_t> (4); // mode "w"
          // Length of ":tt", except null terminator.
          fields[2] = reinterpret_cast<field_t> (sizeof (":tt") - 1);

          ret = semihosting::call_host (SEMIHOSTING_SYS_OPEN,
                                        static_cast<void*> (fields));
          if (ret == -1)
            {
              return -1;
            }

          handle = ret;
        }

      fields[0] = reinterpret_cast<field_t> (handle);
      fields[1] = const_cast<field_t> (buf);
      fields[2] = reinterpret_cast<field_t> (nbyte);
      // Send character array to host file/device.
      ret = semihosting::call_host (SEMIHOSTING_SYS_WRITE,
                                    static_cast<void*> (fields));
      // This call returns the number of bytes NOT written (0 if all ok).

      // -1 is not a legal value, but SEGGER seems to return it
      if (ret == -1)
        {
          return -1;
        }

      // The compliant way of returning errors.
      if (ret == static_cast<int> (nbyte))
        {
          return -1;
        }

      // Return the number of bytes written.
      return static_cast<ssize_t> ((nbyte)) - ret;
    }

#endif // defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT)

  } // namespace trace
} // namespace micro_os_plus

#endif /* defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_DEBUG) || \
          defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT) */
#endif // defined(MICRO_OS_PLUS_TRACE)

// ----------------------------------------------------------------------------

#endif // !Unix

// ----------------------------------------------------------------------------
