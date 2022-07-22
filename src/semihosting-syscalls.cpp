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

#include "posix-functions.h"

#include <cstring>

#include <cstdint>
#include <cstdarg>
#include <cerrno>

#include <sys/fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>

// ----------------------------------------------------------------------------

using namespace micro_os_plus;

// ----------------------------------------------------------------------------

/**
 * This file implements all POSIX functions over the ARM
 * semihosting API.
 *
 * Compliance:
 * "Semihosting for AArch32 and AArch64, Release 2.0"
 * https://static.docs.arm.com/100863/0200/semihosting.pdf
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

#if !defined(MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES)
#define MICRO_OS_PLUS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES (20)
#endif

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

  off_t
  lseek_impl (int fd, off_t offset, int whence);

  int
  stat_impl (int fd, struct stat* st);

  // -------------------------------------------------------------------

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

  /*
   *  fd, is a user file descriptor.
   */
  off_t
  lseek_impl (int fd, off_t offset, int whence)
  {
    file* pfd;

    // Valid file descriptor?
    pfd = find_slot (fd);
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

    int res;
    res = check_error (static_cast<int> (semihosting::call_host (
        SEMIHOSTING_SYS_FLEN,
        reinterpret_cast<semihosting::param_block_t*> (&pfd->handle))));
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
// ---- Standard POSIX IO functions -------------------------------------------

namespace posix
{
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
  open (const char* path, int oflag, ...)
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
        res = posix::stat (path, &st);
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
  close (int fildes)
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

  // --------------------------------------------------------------------------

  // fd, is a valid user file handle.
  // Translates the return of SYS_READ into
  // bytes read.

  ssize_t
  read (int fildes, void* buf, size_t nbyte)
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
        return res;
      }

    pfd->pos += static_cast<off_t> (nbyte - static_cast<size_t> (res));

    // res == nbyte is not an error,
    // at least if we want feof() to work.
    return static_cast<ssize_t> (nbyte - static_cast<size_t> (res));
  }

  ssize_t
  write (int fildes, const void* buf, size_t nbyte)
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
  lseek (int fildes, off_t offset, int whence)
  {
    return lseek_impl (fildes, offset, whence);
  }

  /**
   * @details
   *
   * This function shall test whether _fildes_, an open file descriptor,
   * is associated with a terminal device.
   */
  int
  isatty (int fildes)
  {
    file* pfd;
    pfd = find_slot (fildes);
    if (pfd == nullptr)
      {
        trace::printf ("%s() EBADF\n", __FUNCTION__);

        errno = EBADF;
        return 0;
      }

    int tty;
    tty = static_cast<int> (semihosting::call_host (
        SEMIHOSTING_SYS_ISTTY,
        reinterpret_cast<semihosting::param_block_t*> (&pfd->handle)));

    if (tty == 1)
      {
        return 1;
      }

    errno = get_host_errno ();
    return 0;
  }

  int
  fstat (int fildes, struct stat* buf)
  {
    memset (buf, 0, sizeof (*buf));
    return stat_impl (fildes, buf);
  }

  // --------------------------------------------------------------------------
  // ----- POSIX file functions -----

  int
  stat (const char* path, struct stat* buf)
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
  rename (const char* existing, const char* _new)
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
  unlink (const char* path)
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
  system (const char* command)
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
  gettimeofday (timeval* ptimeval, void* ptimezone)
  {
    timezone* tzp = static_cast<timezone*> (ptimezone);
    if (ptimeval)
      {
        // Ask the host for the seconds since the Unix epoch.
        ptimeval->tv_sec = static_cast<time_t> (
            semihosting::call_host (SEMIHOSTING_SYS_TIME, nullptr));
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

  // Return a clock that ticks at 100Hz.
  clock_t
  clock (void)
  {
    clock_t timeval;
    timeval = semihosting::call_host (SEMIHOSTING_SYS_CLOCK, nullptr);

    return timeval;
  }

  clock_t
  times (tms* buf)
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

  char*
  getcwd (char* buf, size_t size)
  {
    // no cwd available via semihosting, so we use the temporary folder
    strncpy (buf, "/tmp", size);
    return buf;
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

  // --------------------------------------------------------------------------
  // ----- POSIX file_system functions -----

  // Required by Google Tests
  int
  mkdir (const char* path, mode_t mode)
  {
#if 0
    // always return true
    return 0;
#else
    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS;
    return -1;
#endif
  }

  int
  rmdir (const char* path)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_RMDIR_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  void
  sync (void)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SYNC_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
  }

  // --------------------------------------------------------------------------
  // ----- Directories functions -----

  DIR*
  opendir (const char* dirpath)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_OPENDIR_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return nullptr;
  }

  dirent*
  readdir (DIR* dirp)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_READDIR_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return nullptr;
  }

  int
  readdir_r (DIR* dirp, dirent* entry, dirent** result)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_READDIR_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  void
  rewinddir (DIR* dirp)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_REWINDDIR_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
  }

  int
  closedir (DIR* dirp)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_CLOSEDIR_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

#pragma GCC diagnostic pop

  // --------------------------------------------------------------------------
  // Socket functions

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

  // socket() and socketpair() are the functions creating sockets.
  // The other are socket specific functions.

  // In addition, the following IO functions should work on sockets:
  // close(), read(), write(), writev(), ioctl(), fcntl(), select().

  int
  socket (int domain, int type, int protocol)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SOCKET_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  socketpair (int domain, int type, int protocol, int socket_vector[2])
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SOCKETPAIR_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  accept (int socket, sockaddr* address, socklen_t* address_len)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_ACCEPT_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  bind (int socket, const sockaddr* address, socklen_t address_len)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_BIND_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  connect (int socket, const sockaddr* address, socklen_t address_len)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_CONNECT_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  getpeername (int socket, sockaddr* address, socklen_t* address_len)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_GETPEERNAME_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  getsockname (int socket, sockaddr* address, socklen_t* address_len)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_GETSOCKNAME_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  getsockopt (int socket, int level, int option_name, void* option_value,
              socklen_t* option_len)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_GETSOCKOPT_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  listen (int socket, int backlog)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_LISTEN_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  ssize_t
  recv (int socket, void* buffer, size_t length, int flags)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_RECV_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  ssize_t
  recvfrom (int socket, void* buffer, size_t length, int flags,
            sockaddr* address, socklen_t* address_len)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_RECVFROM_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  ssize_t
  recvmsg (int socket, msghdr* message, int flags)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_RECVMSG_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  ssize_t
  send (int socket, const void* buffer, size_t length, int flags)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SEND_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  ssize_t
  sendmsg (int socket, const msghdr* message, int flags)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SENDMSG_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  ssize_t
  sendto (int socket, const void* message, size_t length, int flags,
          const sockaddr* dest_addr, socklen_t dest_len)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SENDTO_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  setsockopt (int socket, int level, int option_name, const void* option_value,
              socklen_t option_len)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SETSOCKOPT_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  shutdown (int socket, int how)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SHUTDOWN_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  sockatmark (int socket)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SOCKATMARK_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

#pragma GCC diagnostic pop

  // --------------------------------------------------------------------------

  // These functions are defined here to avoid linker errors in free
  // standing applications. They might be called in some error cases
  // from library code.
  //
  // If you detect other functions to be needed, just let us know
  // and we'll add them.

  // --------------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

  // --------------------------------------------------------------------------
  // Not yet implemented.

  int
  select (int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds,
          timeval* timeout)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SELECT_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  chdir (const char* path)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_CHDIR_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  // --------------------------------------------------------------------------
  // Not available via semihosting.

  ssize_t
  writev (int fildes, const iovec* iov, int iovcnt)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_WRITEV_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  ioctl (int fildes, int request, ...)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_IOCTL_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  fcntl (int fildes, int cmd, ...)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_FCNTL_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  ftruncate (int fildes, off_t length)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_FTRUNCATE_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  fsync (int fildes)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_FSYNC_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  chmod (const char* path, mode_t mode)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_CHMOD_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  truncate (const char* path, off_t length)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_TRUNCATE_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  utime (const char* path, const utimbuf* times)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_UTIME_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  // --------------------------------------------------------------------------
  // Unavailable in non-Unix embedded environments.

  int
  execve (const char* path, char* const argv[], char* const envp[])
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
  fork (void)
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

  pid_t
  getpid (void)
  {
    return 1;
  }

  int
  kill (pid_t pid, int sig)
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

  int
  raise (int sig)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_RAISE_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  pid_t
  wait (int* stat_loc)
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
  chown (const char* path, uid_t owner, gid_t group)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_CHOWN_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  int
  link (const char* existing, const char* _new)
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

  int
  symlink (const char* existing, const char* _new)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_SYMLINK_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

  ssize_t
  readlink (const char* path, char* buf, size_t bufsize)
  {
#if defined(MICRO_OS_PLUS_DEBUG) \
    && (defined(MICRO_OS_PLUS_DEBUG_SYSCALLS_BRK) \
        || defined(MICRO_OS_PLUS_DEBUG_SYSCALL_READLINK_BRK))
    architecture::brk ();
#endif

    trace::printf ("%s() ENOSYS\n", __FUNCTION__);

    errno = ENOSYS; // Not implemented
    return -1;
  }

#pragma GCC diagnostic pop

} // namespace posix

// ----------------------------------------------------------------------------

#if (__STDC_HOSTED__ != 0)

extern "C"
{

  /**
   * In hosted environments only, alias all functions to C;
   * in freestanding environments these are intentionally omitted.
   *
   * The main trick here is that some of the functions are prefixed
   * with `_`; these are newlib specific functions.
   *
   * Aliases might not be very portable, but are very efficient.
   *
   * The aliases must be in the same compilation unit as the names
   * they alias.
   */

#if (__SIZEOF_POINTER__ == 4)
  int __attribute__ ((weak, alias ("_ZN5posix6acceptEiP8sockaddrPm")))
  accept (int socket, sockaddr* address, socklen_t* address_len);

  int __attribute__ ((weak, alias ("_ZN5posix4bindEiPK8sockaddrm")))
  bind (int socket, const sockaddr* address, socklen_t address_len);
#elif (__SIZEOF_POINTER__ == 8)
  int __attribute__ ((weak, alias ("_ZN5posix6acceptEiP8sockaddrPj")))
  accept (int socket, sockaddr* address, socklen_t* address_len);

  int __attribute__ ((weak, alias ("_ZN5posix4bindEiPK8sockaddrj")))
  bind (int socket, const sockaddr* address, socklen_t address_len);
#endif

  int __attribute__ ((weak, alias ("_ZN5posix5chdirEPKc")))
  chdir (const char* path);

#if (__SIZEOF_POINTER__ == 4)
  int __attribute__ ((weak, alias ("_ZN5posix5chmodEPKcm")))
  chmod (const char* path, mode_t mode);
#elif (__SIZEOF_POINTER__ == 8)
  int __attribute__ ((weak, alias ("_ZN5posix5chmodEPKcj")))
  chmod (const char* path, mode_t mode);
#endif

  int __attribute__ ((weak, alias ("_ZN5posix5chownEPKctt")))
  _chown (const char* path, uid_t owner, gid_t group);

  clock_t __attribute__ ((weak, alias ("_ZN5posix5clockEv"))) _clock (void);

  int __attribute__ ((weak, alias ("_ZN5posix5closeEi"))) _close (int fildes);

  int __attribute__ ((weak, alias ("_ZN5posix8closedirEP3DIR")))
  closedir (DIR* dirp);

#if (__SIZEOF_POINTER__ == 4)
  int __attribute__ ((weak, alias ("_ZN5posix7connectEiPK8sockaddrm")))
  connect (int socket, const sockaddr* address, socklen_t address_len);
#elif (__SIZEOF_POINTER__ == 8)
  int __attribute__ ((weak, alias ("_ZN5posix7connectEiPK8sockaddrj")))
  connect (int socket, const sockaddr* address, socklen_t address_len);
#endif

  int __attribute__ ((weak, alias ("_ZN5posix6execveEPKcPKPcS4_")))
  _execve (const char* path, char* const argv[], char* const envp[]);

  int __attribute__ ((weak, alias ("_ZN5posix5fcntlEiiz")))
  fcntl (int fildes, int cmd, ...);

  pid_t __attribute__ ((weak, alias ("_ZN5posix4forkEv"))) _fork (void);

  int __attribute__ ((weak, alias ("_ZN5posix5fstatEiP4stat")))
  _fstat (int fildes, struct stat* buf);

  int __attribute__ ((weak, alias ("_ZN5posix9ftruncateEil")))
  ftruncate (int fildes, off_t length);

  int __attribute__ ((weak, alias ("_ZN5posix5fsyncEi"))) fsync (int fildes);

#if (__SIZEOF_POINTER__ == 4)
  char* __attribute__ ((weak, alias ("_ZN5posix6getcwdEPcj")))
  getcwd (char* buf, size_t size);

  int __attribute__ ((weak, alias ("_ZN5posix11getpeernameEiP8sockaddrPm")))
  getpeername (int socket, sockaddr* address, socklen_t* address_len);
#elif (__SIZEOF_POINTER__ == 8)
  char* __attribute__ ((weak, alias ("_ZN5posix6getcwdEPcm")))
  getcwd (char* buf, size_t size);

  int __attribute__ ((weak, alias ("_ZN5posix11getpeernameEiP8sockaddrPj")))
  getpeername (int socket, sockaddr* address, socklen_t* address_len);
#endif

  pid_t __attribute__ ((weak, alias ("_ZN5posix6getpidEv"))) _getpid (void);

#if (__SIZEOF_POINTER__ == 4)
  int __attribute__ ((weak, alias ("_ZN5posix11getsocknameEiP8sockaddrPm")))
  getsockname (int socket, sockaddr* address, socklen_t* address_len);

  int __attribute__ ((weak, alias ("_ZN5posix10getsockoptEiiiPvPm")))
  getsockopt (int socket, int level, int option_name, void* option_value,
              socklen_t* option_len);
#elif (__SIZEOF_POINTER__ == 8)
  int __attribute__ ((weak, alias ("_ZN5posix11getsocknameEiP8sockaddrPj")))
  getsockname (int socket, sockaddr* address, socklen_t* address_len);

  int __attribute__ ((weak, alias ("_ZN5posix10getsockoptEiiiPvPj")))
  getsockopt (int socket, int level, int option_name, void* option_value,
              socklen_t* option_len);
#endif

  int __attribute__ ((weak, alias ("_ZN5posix12gettimeofdayEP7timevalPv")))
  _gettimeofday (timeval* ptimeval, void* ptimezone);

  int __attribute__ ((weak, alias ("_ZN5posix5ioctlEiiz")))
  ioctl (int fildes, int request, ...);

  int __attribute__ ((weak, alias ("_ZN5posix6isattyEi")))
  _isatty (int fildes);

  int __attribute__ ((weak, alias ("_ZN5posix4killEii")))
  _kill (pid_t pid, int sig);

  int __attribute__ ((weak, alias ("_ZN5posix4linkEPKcS1_")))
  _link (const char* existing, const char* _new);

  int __attribute__ ((weak, alias ("_ZN5posix6listenEii")))
  listen (int socket, int backlog);

  off_t __attribute__ ((weak, alias ("_ZN5posix5lseekEili")))
  _lseek (int fildes, off_t offset, int whence);

#if (__SIZEOF_POINTER__ == 4)
  int __attribute__ ((weak, alias ("_ZN5posix5mkdirEPKcm")))
  mkdir (const char* path, mode_t mode);
#elif (__SIZEOF_POINTER__ == 8)
  int __attribute__ ((weak, alias ("_ZN5posix5mkdirEPKcj")))
  mkdir (const char* path, mode_t mode);
#endif

  int __attribute__ ((weak, alias ("_ZN5posix4openEPKciz")))
  _open (const char* path, int oflag, ...);

  DIR* __attribute__ ((weak, alias ("_ZN5posix7opendirEPKc")))
  opendir (const char* dirname);

  int __attribute__ ((weak, alias ("_ZN5posix5raiseEi"))) raise (int sig);

#if (__SIZEOF_POINTER__ == 4)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix4readEiPvj")))
  _read (int fildes, void* buf, size_t nbyte);
#elif (__SIZEOF_POINTER__ == 8)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix4readEiPvm")))
  _read (int fildes, void* buf, size_t nbyte);
#endif

  dirent* __attribute__ ((weak, alias ("_ZN5posix7readdirEP3DIR")))
  readdir (DIR* dirp);

  int __attribute__ ((weak, alias ("_ZN5posix9readdir_rEP3DIRP6direntPS3_")))
  readdir_r (DIR* dirp, dirent* entry, dirent** result);

#if (__SIZEOF_POINTER__ == 4)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix8readlinkEPKcPcj")))
  _readlink (const char* path, char* buf, size_t bufsize);

  ssize_t __attribute__ ((weak, alias ("_ZN5posix4recvEiPvji")))
  recv (int socket, void* buffer, size_t length, int flags);

  ssize_t
      __attribute__ ((weak, alias ("_ZN5posix8recvfromEiPvjiP8sockaddrPm")))
      recvfrom (int socket, void* buffer, size_t length, int flags,
                sockaddr* address, socklen_t* address_len);
#elif (__SIZEOF_POINTER__ == 8)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix8readlinkEPKcPcm")))
  _readlink (const char* path, char* buf, size_t bufsize);

  ssize_t __attribute__ ((weak, alias ("_ZN5posix4recvEiPvmi")))
  recv (int socket, void* buffer, size_t length, int flags);

  ssize_t
      __attribute__ ((weak, alias ("_ZN5posix8recvfromEiPvmiP8sockaddrPj")))
      recvfrom (int socket, void* buffer, size_t length, int flags,
                sockaddr* address, socklen_t* address_len);
#endif

  ssize_t __attribute__ ((weak, alias ("_ZN5posix7recvmsgEiPNS_6msghdrEi")))
  recvmsg (int socket, struct msghdr* message, int flags);

  int __attribute__ ((weak, alias ("_ZN5posix6renameEPKcS1_")))
  rename (const char* oldfn, const char* newfn);

  void __attribute__ ((weak, alias ("_ZN5posix9rewinddirEP3DIR")))
  rewinddir (DIR* dirp);

  int __attribute__ ((weak, alias ("_ZN5posix5rmdirEPKc")))
  rmdir (const char* path);

  int __attribute__ ((weak))
  select (int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds,
          timeval* timeout)
  {
    return posix::select (nfds, readfds, writefds, errorfds, timeout);
  }

#if (__SIZEOF_POINTER__ == 4)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix4sendEiPKvji")))
  send (int socket, const void* buffer, size_t length, int flags);
#elif (__SIZEOF_POINTER__ == 8)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix4sendEiPKvmi")))
  send (int socket, const void* buffer, size_t length, int flags);
#endif

  ssize_t __attribute__ ((weak, alias ("_ZN5posix7sendmsgEiPKNS_6msghdrEi")))
  sendmsg (int socket, const msghdr* message, int flags);

#if (__SIZEOF_POINTER__ == 4)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix6sendtoEiPKvjiPK8sockaddrm")))
  sendto (int socket, const void* message, size_t length, int flags,
          const sockaddr* dest_addr, socklen_t dest_len);

  int __attribute__ ((weak, alias ("_ZN5posix10setsockoptEiiiPKvm")))
  setsockopt (int socket, int level, int option_name, const void* option_value,
              socklen_t option_len);
#elif (__SIZEOF_POINTER__ == 8)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix6sendtoEiPKvmiPK8sockaddrj")))
  sendto (int socket, const void* message, size_t length, int flags,
          const sockaddr* dest_addr, socklen_t dest_len);

  int __attribute__ ((weak, alias ("_ZN5posix10setsockoptEiiiPKvj")))
  setsockopt (int socket, int level, int option_name, const void* option_value,
              socklen_t option_len);
#endif

  int __attribute__ ((weak, alias ("_ZN5posix8shutdownEii")))
  shutdown (int socket, int how);

  int __attribute__ ((weak, alias ("_ZN5posix10sockatmarkEi")))
  sockatmark (int socket);

  int __attribute__ ((weak, alias ("_ZN5posix6socketEiii")))
  socket (int domain, int type, int protocol);

  int __attribute__ ((weak, alias ("_ZN5posix10socketpairEiiiPi")))
  socketpair (int domain, int type, int protocol, int socket_vector[2]);

  int __attribute__ ((weak, alias ("_ZN5posix4statEPKcP4stat")))
  _stat (const char* path, struct stat* buf);

  void __attribute__ ((weak, alias ("_ZN5posix4syncEv"))) sync (void);

  int __attribute__ ((weak, alias ("_ZN5posix7symlinkEPKcS1_")))
  _symlink (const char* existing, const char* _new);

  int __attribute__ ((weak, alias ("_ZN5posix6systemEPKc")))
  system (const char* command);

  clock_t __attribute__ ((weak, alias ("_ZN5posix5timesEP3tms")))
  _times (tms* buf);

  int __attribute__ ((weak, alias ("_ZN5posix8truncateEPKcl")))
  truncate (const char* path, off_t length);

  int __attribute__ ((weak, alias ("_ZN5posix6unlinkEPKc")))
  _unlink (const char* name);

  int __attribute__ ((weak, alias ("_ZN5posix5utimeEPKcPKNS_7utimbufE")))
  utime (const char* path, const struct utimbuf* times);

  pid_t __attribute__ ((weak, alias ("_ZN5posix4waitEPi")))
  _wait (int* stat_loc);

#if (__SIZEOF_POINTER__ == 4)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix5writeEiPKvj")))
  _write (int fildes, const void* buf, size_t nbyte);
#elif (__SIZEOF_POINTER__ == 8)
  ssize_t __attribute__ ((weak, alias ("_ZN5posix5writeEiPKvm")))
  _write (int fildes, const void* buf, size_t nbyte);
#endif

  ssize_t __attribute__ ((weak, alias ("_ZN5posix6writevEiPKNS_5iovecEi")))
  writev (int fildes, const struct iovec* iov, int iovcnt);

} /* extern "C" */

#endif // (__STDC_HOSTED__ != 0)

// ----------------------------------------------------------------------------


extern "C"
{
  void
  initialise_monitor_handles (void);
}

// ----------------------------------------------------------------------------

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

#endif // defined(MICRO_OS_PLUS_INCLUDE_SEMIHOSTING_SYSCALLS)

// ----------------------------------------------------------------------------

#endif // !Unix

// ----------------------------------------------------------------------------
