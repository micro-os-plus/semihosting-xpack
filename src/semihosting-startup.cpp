/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus/)
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

#if defined(MICRO_OS_PLUS_INCLUDE_CONFIG_H)
#include <micro-os-plus/config.h>
#endif // MICRO_OS_PLUS_INCLUDE_CONFIG_H

#if defined(MICRO_OS_PLUS_INCLUDE_SEMIHOSTING_STARTUP)

#include <micro-os-plus/semihosting.h>
#include <micro-os-plus/architecture.h>
#include <micro-os-plus/diag/trace.h>

#include <ctype.h>

// ----------------------------------------------------------------------------

#if !defined(MICRO_OS_PLUS_INTEGER_SEMIHOSTING_ARGS_BUF_ARRAY_SIZE)
#define MICRO_OS_PLUS_INTEGER_SEMIHOSTING_ARGS_BUF_ARRAY_SIZE 80
#endif

#if !defined(MICRO_OS_PLUS_INTEGER_SEMIHOSTING_ARGV_BUF_ARRAY_SIZE)
#define MICRO_OS_PLUS_INTEGER_SEMIHOSTING_ARGV_BUF_ARRAY_SIZE 10
#endif

// ----------------------------------------------------------------------------

using namespace micro_os_plus;

// ----------------------------------------------------------------------------

extern "C"
{
  void
  micro_os_plus_startup_initialize_args (int* p_argc, char*** p_argv);

  void
  micro_os_plus_terminate (int code);
}

// ----------------------------------------------------------------------------

// This is the semihosting implementation for the routine to
// process arguments.
// The entire command line is received from the host
// and parsed into strings.

void
micro_os_plus_startup_initialize_args (int* p_argc, char*** p_argv)
{
  // Array of chars to receive the command line from the host.
  static char args_buf[MICRO_OS_PLUS_INTEGER_SEMIHOSTING_ARGS_BUF_ARRAY_SIZE];

  // Array of pointers to store the final argv pointers (pointing
  // in the above array).
  static char* argv_buf[MICRO_OS_PLUS_INTEGER_SEMIHOSTING_ARGV_BUF_ARRAY_SIZE];

  int argc = 0;
  bool is_in_argument = false;

  semihosting::param_block_t fields[2];
  fields[0] = reinterpret_cast<semihosting::param_block_t> (args_buf);
  fields[1] = sizeof (args_buf) - 1;
  int ret = static_cast<int> (
      semihosting::call_host (SEMIHOSTING_SYS_GETCMDLINE, fields));
  if (ret == 0)
    {
      // In case the host send more than we can chew, limit the
      // string to our buffer.
      args_buf[sizeof (args_buf) - 1] = '\0';

      // The returned command line is a null terminated string.
      char* p = reinterpret_cast<char*> (fields[0]);

      int delim = '\0';
      int ch;

      while ((ch = *p) != '\0')
        {
          if (!is_in_argument)
            {
              if (!isblank (ch))
                {
                  if (argc >= static_cast<int> (
                          (sizeof (argv_buf) / sizeof (argv_buf[0])) - 1))
                    break;

                  if (ch == '"' || ch == '\'')
                    {
                      // Remember the delimiter to search for the
                      // corresponding terminator.
                      delim = ch;
                      ++p; // skip the delimiter.
                      ch = *p;
                    }
                  // Remember the arg beginning address.
                  argv_buf[argc++] = p;
                  is_in_argument = true;
                }
            }
          else if (delim != '\0')
            {
              if ((ch == delim))
                {
                  delim = '\0';
                  *p = '\0';
                  is_in_argument = false;
                }
            }
          else if (isblank (ch))
            {
              delim = '\0';
              *p = '\0';
              is_in_argument = false;
            }
          ++p;
        }
    }

  if (argc == 0)
    {
      // No arguments found in string, return a single empty name.
      args_buf[0] = '\0';
      argv_buf[0] = &args_buf[0];
      ++argc;
    }

  // Must end the array with a null pointer.
  argv_buf[argc] = nullptr;

  *p_argc = argc;
  *p_argv = &argv_buf[0];

  return;
}

// ----------------------------------------------------------------------------

void __attribute__ ((noreturn, weak)) micro_os_plus_terminate (int code)
{
#if (__SIZEOF_POINTER__ == 4)
  semihosting::call_host (SEMIHOSTING_SYS_EXIT,
                          reinterpret_cast<semihosting::param_block_t*> (
                              code == 0 ? ADP_STOPPED_APPLICATION_EXIT
                                        : ADP_STOPPED_RUN_TIME_ERROR));
#elif (__SIZEOF_POINTER__ == 8)
  semihosting::param_block_t fields[2];
  fields[0] = ADP_STOPPED_APPLICATION_EXIT;
  fields[1] = static_cast<semihosting::param_block_t> (code);
  semihosting::call_host (SEMIHOSTING_SYS_EXIT, fields);
#endif

#if defined(MICRO_OS_PLUS_DEBUG)
  architecture::brk ();
#endif

  while (true)
    {
      architecture::wfi ();
    }
  /* NOTREACHED */
}

// ----------------------------------------------------------------------------

#endif // defined(MICRO_OS_PLUS_INCLUDE_SEMIHOSTING_STARTUP)

// ----------------------------------------------------------------------------

#endif // !Unix

// ----------------------------------------------------------------------------
