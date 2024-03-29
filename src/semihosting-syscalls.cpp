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

#if defined(MICRO_OS_PLUS_INCLUDE_SEMIHOSTING_SYSCALLS)

#include <micro-os-plus/semihosting.h>
#include <micro-os-plus/architecture.h>
#include <micro-os-plus/diag/trace.h>

#include <cstring>

#include <cstdint>
#include <cstdarg>
#include <cerrno>

#include <sys/fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>

// DO NOT include it, not supported.
// #include <dirent.h>

// ----------------------------------------------------------------------------

#if !defined(MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES)
#define MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES (20)
#endif

// ----------------------------------------------------------------------------

using namespace micro_os_plus;

// ----------------------------------------------------------------------------

/**
 * This file implements all POSIX functions over the Arm
 * semihosting API.
 *
 * Compliance:
 * "Semihosting for AArch32 and AArch64, Release 2.0"
 * https://github.com/ARM-software/abi-aa/releases/download/2022Q1/semihosting.pdf
 *
 * Notes: Reentrancy and 'errno'.
 * The standard headers define errno as '*(__errno())';
 * In a multi-threaded environment, __errno() must return a
 * thread specific pointer.
 *
 * ENOSYS
 * Not implemented functions print a trace message and return ENOSYS.
 * To help identify the caller, optionally they can break to the
 * debugger.
 */

// ----------------------------------------------------------------------------

extern "C"
{
  // This name is used by newlib; for compatibility reasons, better preserve
  // it.
  void
  initialise_monitor_handles (void);
}

// ----------------------------------------------------------------------------

// Local support functions in the anonymous namespace.
namespace
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"

  // Struct used to keep track of the file position, just so we
  // can implement fseek(fh,x,SEEK_CUR).
  struct file
  {
    int handle;
    off_t pos;
  };

#pragma GCC diagnostic pop

  /*
   *  User file descriptors (fd) are integer indexes into
   * the opened_files[] array. Error checking is done by using
   * find_slot().
   *
   * This opened_files array is manipulated directly by only
   * these 5 functions:
   *
   * find_slot() - Translate entry.
   * new_slot() - Find empty entry.
   * initialise_monitor_handles() - Initialize entries.
   * open()
   * close()
   *
   * Every other function must use find_slot().
   */
  file opened_files[MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES];

  file*
  find_slot (int fd);

  int
  new_slot (void);

  int
  get_host_errno (void);

  int
  with_set_errno (int result);

  int
  check_error (int result);

  int
  stat_impl (int fd, struct stat* st);
} // namespace

// ----------------------------------------------------------------------------

// This should be called before static constructors.
void
initialise_monitor_handles (void)
{
  // Open the standard file descriptors by opening the special
  // teletype device, ":tt", read-only to obtain a descriptor for
  // standard input and write-only to obtain a descriptor for standard
  // output. Finally, open ":tt" in append mode to obtain a descriptor
  // for standard error. Since this is a write mode, most kernels will
  // probably return the same value as for standard output, but the
  // kernel can differentiate the two using the mode flag and return a
  // different descriptor for standard error.

  semihosting::param_block_t fields[3];

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

  fields[0] = reinterpret_cast<semihosting::param_block_t> (
      const_cast<char*> (":tt"));
  fields[2] = 3; // length of filename
  fields[1] = 0; // mode "r"
  int monitor_stdin = static_cast<int> (semihosting::call_host (
      SEMIHOSTING_SYS_OPEN,
      static_cast<semihosting::param_block_t*> (fields)));

  fields[0] = reinterpret_cast<semihosting::param_block_t> (
      const_cast<char*> (":tt"));
  fields[2] = 3; // length of filename
  fields[1] = 4; // mode "w"
  int monitor_stdout = static_cast<int> (semihosting::call_host (
      SEMIHOSTING_SYS_OPEN,
      static_cast<semihosting::param_block_t*> (fields)));

#pragma GCC diagnostic pop

  fields[0] = reinterpret_cast<semihosting::param_block_t> (
      const_cast<char*> (":tt"));
  fields[2] = 3; // length of filename
  fields[1] = 8; // mode "a"
  int monitor_stderr = static_cast<int> (semihosting::call_host (
      SEMIHOSTING_SYS_OPEN,
      static_cast<semihosting::param_block_t*> (fields)));

  // If we failed to open stderr, redirect to stdout.
  if (monitor_stderr == -1)
    {
      monitor_stderr = monitor_stdout;
    }

  for (int i = 0; i < MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES; i++)
    {
      opened_files[i].handle = -1;
    }

  opened_files[0].handle = monitor_stdin;
  opened_files[0].pos = 0;
  opened_files[1].handle = monitor_stdout;
  opened_files[1].pos = 0;
  opened_files[2].handle = monitor_stderr;
  opened_files[2].pos = 0;
}

// ----------------------------------------------------------------------------

namespace
{
  // --------------------------------------------------------------------------

  /**
   * Return a pointer to the structure associated with
   * the user file descriptor fd.
   */
  file*
  find_slot (int fd)
  {
    if (static_cast<size_t> (fd)
        >= MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES)
      {
        // File descriptor is out of range.
        return nullptr;
      }

    if (opened_files[fd].handle == -1)
      {
        // File descriptor not in use.
        return nullptr;
      }

    // Valid, return host file descriptor.
    return &opened_files[fd];
  }

  /**
   * Return the next lowest numbered free file
   * structure, or -1 if we can't find one.
   */
  int
  new_slot (void)
  {
    size_t i;
    for (i = 0; i < MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES; i++)
      {
        if (opened_files[i].handle == -1)
          {
            break;
          }
      }

    if (i == MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES)
      {
        return -1;
      }

    return static_cast<int> (i);
  }

  int
  get_host_errno (void)
  {
    return static_cast<int> (
        semihosting::call_host (SEMIHOSTING_SYS_ERRNO, nullptr));
  }

  /**
   * Set errno and return result.
   */
  int
  with_set_errno (int result)
  {
    errno = get_host_errno ();
    return result;
  }

  /*
   * Check the return and set errno appropriately.
   */
  int
  check_error (int result)
  {
    if (result == -1)
      {
        return with_set_errno (-1);
      }

    return result;
  }

  int
  stat_impl (int fd, struct stat* st)
  {
    file* pfd;
    pfd = find_slot (fd);
    if (pfd == nullptr)
      {
        errno = EBADF;
        return -1;
      }

    // Always assume a character device, with 1024 byte blocks.
    st->st_mode |= S_IFCHR;
    st->st_blksize = 1024;

    semihosting::param_block_t fields[1];
    fields[0] = static_cast<semihosting::param_block_t> (pfd->handle);

    int res;
    res = check_error (static_cast<int> (
        semihosting::call_host (SEMIHOSTING_SYS_FLEN, fields)));
    if (res == -1)
      {
        return -1;
      }

    // Return the file size.
    st->st_size = res;
    return 0;
  }

} // namespace

// ----------------------------------------------------------------------------
// ---- Newlib libgloss functions ---------------------------------------------

extern "C"
{
  int
  _open (const char* path, int oflag, ...);

  int
  _close (int fildes);

  ssize_t
  _read (int fildes, void* buf, size_t nbyte);

  ssize_t
  _write (int fildes, const void* buf, size_t nbyte);

  off_t
  _lseek (int fildes, off_t offset, int whence);

  int
  _isatty (int fildes);

  int
  _fstat (int fildes, struct stat* buf);

  int
  _stat (const char* path, struct stat* buf);

  int
  _rename (const char* existing, const char* _new);

  int
  _unlink (const char* path);

  int
  _system (const char* command);

  int
  _gettimeofday (timeval* ptimeval, void* ptimezone);

  int
  _ftime (timeb* tp);

  clock_t
  _times (tms* buf);

  clock_t
  _clock (void);

  pid_t
  _getpid (void);

  int
  _execve (const char* path, char* const argv[], char* const envp[]);

  pid_t
  _fork (void);

  int
  _kill (pid_t pid, int sig);

  // In `include/sys/wait.h`
  // pid_t
  // _wait (int* stat_loc);

  int
  _link (const char* existing, const char* _new);
}

/**
 * @details
 *
 * The `open()` function shall establish the connection between a file and a
 * file descriptor. It shall create an open file description that refers
 * to a file and a file descriptor that refers to that open file
 * description. The file descriptor is used by other I/O functions to
 * refer to that file. The _path_ argument points to a pathname naming
 * the file.
 */
int
_open (const char* path, int oflag, ...)
{

  int fd = new_slot ();
  if (fd == -1)
    {
      trace::printf ("%s() EMFILE\n", __FUNCTION__);

      errno = EMFILE;
      return -1;
    }

  // It is an error to open a file that already exists.
  if ((oflag & O_CREAT) && (oflag & O_EXCL))
    {
      struct stat st;
      int res;
      res = stat (path, &st);
      if (res != -1)
        {
          trace::printf ("%s() EEXIST\n", __FUNCTION__);

          errno = EEXIST;
          return -1;
        }
    }

  int aflags = 0;
  // The flags are Unix-style, so we need to convert them.
#ifdef O_BINARY
  if (oflag & O_BINARY)
    {
      aflags |= 1;
    }
#endif

  // In O_RDONLY we expect aflags == 0.

  if (oflag & O_RDWR)
    {
      aflags |= 2;
    }

  if ((oflag & O_CREAT) || (oflag & O_TRUNC) || (oflag & O_WRONLY))
    {
      aflags |= 4;
    }

  if (oflag & O_APPEND)
    {
      // Can't ask for w AND a; means just 'a'.
      aflags &= ~4;
      aflags |= 8;
    }

  semihosting::param_block_t fields[3];
  fields[0] = reinterpret_cast<semihosting::param_block_t> (
      const_cast<char*> (path));
  fields[1] = static_cast<semihosting::param_block_t> (aflags);
  fields[2] = std::strlen (path);

  int fh = static_cast<int> (
      semihosting::call_host (SEMIHOSTING_SYS_OPEN, fields));

  // Return a user file descriptor or an error.
  if (fh >= 0)
    {
      opened_files[fd].handle = fh;
      opened_files[fd].pos = 0;
      return fd;
    }
  else
    {
      return with_set_errno (fh);
    }
}

int
_close (int fildes)
{
  file* pfd;
  pfd = find_slot (fildes);
  if (pfd == nullptr)
    {
      trace::printf ("%s() EBADF\n", __FUNCTION__);

      errno = EBADF;
      return -1;
    }

  // Handle stderr == stdout.
  if ((fildes == 1 || fildes == 2)
      && (opened_files[1].handle == opened_files[2].handle))
    {
      pfd->handle = -1;
      return 0;
    }

  semihosting::param_block_t fields[1];
  fields[0] = static_cast<semihosting::param_block_t> (pfd->handle);

  // Attempt to close the handle.
  int res;
  res = check_error (static_cast<int> (
      semihosting::call_host (SEMIHOSTING_SYS_CLOSE, fields)));

  // Reclaim handle?
  if (res == 0)
    {
      pfd->handle = -1;
    }

  return res;
}

// ----------------------------------------------------------------------------

// fd, is a valid user file handle.
// Translates the return of SYS_READ into
// bytes read.

ssize_t
_read (int fildes, void* buf, size_t nbyte)
{
  file* pfd;
  pfd = find_slot (fildes);
  if (pfd == nullptr)
    {
      trace::printf ("%s() EBADF\n", __FUNCTION__);

      errno = EBADF;
      return -1;
    }

  semihosting::param_block_t fields[3];
  fields[0] = static_cast<semihosting::param_block_t> (pfd->handle);
  fields[1] = reinterpret_cast<semihosting::param_block_t> (buf);
  fields[2] = nbyte;

  int res;
  // Returns the number of bytes *not* written.
  res = check_error (static_cast<int> (
      semihosting::call_host (SEMIHOSTING_SYS_READ, fields)));
  if (res == -1)
    {
      return -1;
    }

  pfd->pos += static_cast<off_t> (nbyte - static_cast<size_t> (res));

  // res == nbyte is not an error,
  // at least if we want feof() to work.
  return static_cast<ssize_t> (nbyte - static_cast<size_t> (res));
}

ssize_t
_write (int fildes, const void* buf, size_t nbyte)
{
  file* pfd;
  pfd = find_slot (fildes);
  if (pfd == nullptr)
    {
      trace::printf ("%s() EBADF\n", __FUNCTION__);

      errno = EBADF;
      return -1;
    }

  semihosting::param_block_t fields[3];

  fields[0] = static_cast<semihosting::param_block_t> (pfd->handle);
  fields[1] = reinterpret_cast<semihosting::param_block_t> (buf);
  fields[2] = nbyte;

  // Returns the number of bytes *not* written.
  int res;
  res = check_error (static_cast<int> (
      semihosting::call_host (SEMIHOSTING_SYS_WRITE, fields)));
  /* Clearly an error. */
  if (res < 0)
    {
      return -1;
    }

  pfd->pos += static_cast<off_t> (nbyte - static_cast<size_t> (res));

  // Did we write 0 bytes?
  // Retrieve errno for just in case.
  if ((nbyte - static_cast<size_t> (res)) == 0)
    {
      return with_set_errno (0);
    }

  return static_cast<ssize_t> (nbyte - static_cast<size_t> (res));
}

off_t
_lseek (int fildes, off_t offset, int whence)
{
  file* pfd;

  // Valid file descriptor?
  pfd = find_slot (fildes);
  if (pfd == nullptr)
    {
      errno = EBADF;
      return -1;
    }

  // Valid whence?
  if ((whence != SEEK_CUR) && (whence != SEEK_SET) && (whence != SEEK_END))
    {
      errno = EINVAL;
      return -1;
    }

  // Convert SEEK_CUR to SEEK_SET.
  if (whence == SEEK_CUR)
    {
      offset += pfd->pos;
      // The resulting file offset would be negative.
      if (offset < 0)
        {
          errno = EINVAL;
          if ((pfd->pos > 0) && (offset > 0))
            {
              errno = EOVERFLOW;
            }
          return -1;
        }
      whence = SEEK_SET;
    }

  semihosting::param_block_t fields[2];
  int res;

  if (whence == SEEK_END)
    {
      fields[0] = static_cast<semihosting::param_block_t> (pfd->handle);
      res = check_error (static_cast<int> (
          semihosting::call_host (SEMIHOSTING_SYS_FLEN, fields)));
      if (res == -1)
        {
          return -1;
        }
      offset += res;
    }

  // This code only does absolute seeks.
  fields[0] = static_cast<semihosting::param_block_t> (pfd->handle);
  fields[1] = static_cast<semihosting::param_block_t> (offset);
  res = check_error (static_cast<int> (
      semihosting::call_host (SEMIHOSTING_SYS_SEEK, fields)));

  // At this point ptr is the current file position.
  if (res >= 0)
    {
      pfd->pos = offset;
      return offset;
    }
  else
    {
      return -1;
    }
}

/**
 * @details
 *
 * This function shall test whether _fildes_, an open file descriptor,
 * is associated with a terminal device.
 */
int
_isatty (int fildes)
{
  file* pfd;
  pfd = find_slot (fildes);
  if (pfd == nullptr)
    {
      trace::printf ("%s() EBADF\n", __FUNCTION__);

      errno = EBADF;
      return 0;
    }

  semihosting::param_block_t fields[1];
  fields[0] = static_cast<semihosting::param_block_t> (pfd->handle);

  int tty;
  tty = static_cast<int> (
      semihosting::call_host (SEMIHOSTING_SYS_ISTTY, fields));

  if (tty == 1)
    {
      return 1;
    }

  errno = get_host_errno ();
  return 0;
}

int
_fstat (int fildes, struct stat* buf)
{
  memset (buf, 0, sizeof (*buf));
  return stat_impl (fildes, buf);
}

// ----------------------------------------------------------------------------
// ----- POSIX file functions -----

int
_stat (const char* path, struct stat* buf)
{
  int fd;
  memset (buf, 0, sizeof (*buf));
  // The best we can do is try to open the file read only.
  // If it exists, then we can guess a few things about it.
  if ((fd = open (path, O_RDONLY)) == -1)
    {
      return -1;
    }
  buf->st_mode |= S_IFREG;
#if __BSD_VISIBLE
  buf->st_mode |= S_IREAD;
#endif
  int res = stat_impl (fd, buf);
  // Not interested in the error.
  close (fd);
  return res;
}

int
_rename (const char* existing, const char* _new)
{
  semihosting::param_block_t fields[4];
  fields[0] = reinterpret_cast<semihosting::param_block_t> (
      const_cast<char*> (existing));
  fields[1] = std::strlen (existing);
  fields[2] = reinterpret_cast<semihosting::param_block_t> (
      const_cast<char*> (_new));
  fields[3] = std::strlen (_new);

  return check_error (static_cast<int> (
             semihosting::call_host (SEMIHOSTING_SYS_RENAME, fields)))
             ? -1
             : 0;
}

int
_unlink (const char* path)
{
  semihosting::param_block_t fields[2];
  fields[0] = reinterpret_cast<semihosting::param_block_t> (
      const_cast<char*> (path));
  fields[1] = (strlen (path));

  int res;
  res = static_cast<int> (
      semihosting::call_host (SEMIHOSTING_SYS_REMOVE, fields));
  if (res == -1)
    {
      return with_set_errno (res);
    }
  return 0;
}

int
_system (const char* command)
{
  // Hmmm.  The ARM debug interface specification doesn't say whether
  // SYS_SYSTEM does the right thing with a null argument, or assign any
  // meaning to its return value.  Try to do something reasonable....
  if (command == nullptr)
    {
      return 1; // maybe there is a shell available? we can hope. :-P
    }

  semihosting::param_block_t fields[2];
  fields[0] = reinterpret_cast<semihosting::param_block_t> (
      const_cast<char*> (command));
  fields[1] = strlen (command);
  int err = check_error (static_cast<int> (
      semihosting::call_host (SEMIHOSTING_SYS_SYSTEM, fields)));
  if ((err >= 0) && (err < 256))
    {
      // We have to convert e, an exit status to the encoded status of
      // the command.  To avoid hard coding the exit status, we simply
      // loop until we find the right position.
      int exit_code;

      for (exit_code = err; (err != 0) && (WEXITSTATUS (err) != exit_code);
           err <<= 1)
        {
          continue;
        }
    }
  return err;
}

int
_gettimeofday (timeval* ptimeval, void* ptimezone)
{
  timezone* tzp = static_cast<timezone*> (ptimezone);
  if (ptimeval)
    {
      // Ask the host for the seconds since the Unix epoch.
      ptimeval->tv_sec
          = semihosting::call_host (SEMIHOSTING_SYS_TIME, nullptr);
      ptimeval->tv_usec = 0;
    }

  // Return fixed data for the time zone.
  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }

  return 0;
}

int
_ftime (timeb* tp)
{
  // Ask the host for the seconds since the Unix epoch.
  tp->time = semihosting::call_host (SEMIHOSTING_SYS_TIME, nullptr);
  tp->millitm = 0;

  return 0;
}

// Return a clock that ticks at 100Hz.
clock_t
_clock (void)
{
  clock_t timeval;
  timeval = static_cast<clock_t> (
      semihosting::call_host (SEMIHOSTING_SYS_CLOCK, nullptr));

  return timeval;
}

// RISC-V provides a more elaborate definition in newlib.
clock_t
_times (tms* buf)
{
  clock_t timeval = clock ();
  if (buf)
    {
      buf->tms_utime = timeval; // user time
      buf->tms_stime = 0; // system time
      buf->tms_cutime = 0; // user time, children
      buf->tms_cstime = 0; // system time, children
    }

  return timeval;
}

pid_t
_getpid (void)
{
  return 1;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int
_execve (const char* path, char* const argv[], char* const envp[])
{
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_EXECVE_BRK))
  architecture::brk ();
#endif

  trace::printf ("%s() ENOSYS\n", __FUNCTION__);

  errno = ENOSYS; // Not implemented
  return -1;
}

pid_t
_fork (void)
{
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_FORK_BRK))
  architecture::brk ();
#endif

  trace::printf ("%s() ENOSYS\n", __FUNCTION__);

  errno = ENOSYS; // Not implemented
  return -1;
}

int
_kill (pid_t pid, int sig)
{
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_KILL_BRK))
  architecture::brk ();
#endif

  trace::printf ("%s() ENOSYS\n", __FUNCTION__);

  errno = ENOSYS; // Not implemented
  return -1;
}

pid_t
_wait (int* stat_loc)
{
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_WAIT_BRK))
  architecture::brk ();
#endif

  trace::printf ("%s() ENOSYS\n", __FUNCTION__);

  errno = ENOSYS; // Not implemented
  return -1;
}

int
_link (const char* existing, const char* _new)
{
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_LINK_BRK))
  architecture::brk ();
#endif

  trace::printf ("%s() ENOSYS\n", __FUNCTION__);

  errno = ENOSYS; // Not implemented
  return -1;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

#if 0

char*
getcwd (char* buf, size_t size)
{
  // no cwd available via semihosting, so we use the temporary folder
  strncpy (buf, "/tmp", size);
  return buf;
}

#endif

// ----------------------------------------------------------------------------

/*

Other RISC-V calls:

int
_access(const char *file, int mode);

int
_fstatat(int dirfd, const char *file, struct stat *st, int flags);

int
_openat(int dirfd, const char *name, int flags, int mode);

*/

// ----------------------------------------------------------------------------

#endif // defined(MICRO_OS_PLUS_INCLUDE_SEMIHOSTING_SYSCALLS)

// ----------------------------------------------------------------------------

#endif // !Unix

// ----------------------------------------------------------------------------
