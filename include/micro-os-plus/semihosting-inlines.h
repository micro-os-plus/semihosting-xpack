/*
 * This file is part of the µOS++ project (https://micro-os-plus.github.io/).
 * Copyright (c) 2018-2026 Liviu Ionescu. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/mit.
 */

#ifndef MICRO_OS_PLUS_SEMIHOSTING_INLINES_H_
#define MICRO_OS_PLUS_SEMIHOSTING_INLINES_H_

// ----------------------------------------------------------------------------

#if defined(__cplusplus)

// ----------------------------------------------------------------------------

// Include the architecture specific definitions, which
// include `call_host()`.
#include <micro-os-plus/architecture.h>

// ----------------------------------------------------------------------------

namespace micro_os_plus::semihosting
{
  // --------------------------------------------------------------------------
  // Portable semihosting functions in C++.

  inline __attribute__ ((always_inline)) response_t
  call_host (int reason, param_block_t* arg)
  {
    return micro_os_plus_semihosting_call_host (reason, arg);
  }

  // --------------------------------------------------------------------------
} // namespace micro_os_plus::semihosting

// ----------------------------------------------------------------------------

#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_SEMIHOSTING_INLINES_H_

// ----------------------------------------------------------------------------
