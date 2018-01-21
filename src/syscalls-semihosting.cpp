/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2015 Liviu Ionescu.
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

// ----------------------------------------------------------------------------
#if defined(OS_USE_SEMIHOSTING_SYSCALLS)

#include <micro-os-plus/semihosting.h>
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
 */

// ----------------------------------------------------------------------------
#if !defined(OS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES)
#define OS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES (20)
#endif

// ----------------------------------------------------------------------------
// Local support functions.

namespace
{

  // Struct used to keep track of the file position, just so we
  // can implement fseek(fh,x,SEEK_CUR).
  struct file
  {
    int handle;
    int pos;
  };

  // The type of a field, for the array passed to the host.
#if ( __SIZEOF_POINTER__ == 4 )
  typedef uint32_t field_t;
#elif ( __SIZEOF_POINTER__ == 8 )
  typedef uint64_t field_t;
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
   * initilise_monitor_handles() - Initialize entries.
   * open()
   * close()
   *
   * Every other function must use find_slot().
   */
  struct file opened_files[OS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES];

  struct file*
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
  lseek_impl (int fd, int ptr, int dir);

  int
  stat_impl (int fd, struct stat* st);

  // -------------------------------------------------------------------

  /**
   * Return a pointer to the structure associated with
   * the user file descriptor fd.
   */
  struct file*
  find_slot (int fd)
  {
    // User file descriptor is out of range.
    if ((unsigned int) fd >= OS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES)
      {
        return nullptr;
      }

    // User file descriptor is open?
    if (opened_files[fd].handle == -1)
      {
        return nullptr;
      }

    // Valid.
    return &opened_files[fd];
  }

  /**
   * Return the next lowest numbered free file
   * structure, or -1 if we can't find one.
   */
  int
  new_slot (void)
  {
    int i;
    for (i = 0; i < OS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES; i++)
      {
        if (opened_files[i].handle == -1)
          {
            break;
          }
      }

    if (i == OS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES)
      {
        return -1;
      }

    return i;
  }

  int
  get_host_errno (void)
  {
    return os::semihosting::call_host (SEMIHOSTING_SYS_ERRNO, nullptr);
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
  int
  lseek_impl (int fd, int ptr, int dir)
  {
    struct file* pfd;

    // Valid file descriptor?
    pfd = find_slot (fd);
    if (pfd == nullptr)
      {
        errno = EBADF;
        return -1;
      }

    // Valid whence?
    if ((dir != SEEK_CUR) && (dir != SEEK_SET) && (dir != SEEK_END))
      {
        errno = EINVAL;
        return -1;
      }

    // Convert SEEK_CUR to SEEK_SET.
    if (dir == SEEK_CUR)
      {
        ptr = pfd->pos + ptr;
        // The resulting file offset would be negative.
        if (ptr < 0)
          {
            errno = EINVAL;
            if ((pfd->pos > 0) && (ptr > 0))
              {
                errno = EOVERFLOW;
              }
            return -1;
          }
        dir = SEEK_SET;
      }

    field_t fields[2];
    int res;

    if (dir == SEEK_END)
      {
        fields[0] = pfd->handle;
        res = check_error (
            os::semihosting::call_host (SEMIHOSTING_SYS_FLEN, fields));
        if (res == -1)
          {
            return -1;
          }
        ptr += res;
      }

    // This code only does absolute seeks.
    fields[0] = pfd->handle;
    fields[1] = ptr;
    res = check_error (
        os::semihosting::call_host (SEMIHOSTING_SYS_SEEK, fields));

    // At this point ptr is the current file position.
    if (res >= 0)
      {
        pfd->pos = ptr;
        return ptr;
      }
    else
      {
        return -1;
      }
  }

  int
  stat_impl (int fd, struct stat* st)
  {
    struct file* pfd;
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
    res = check_error (
        os::semihosting::call_host (SEMIHOSTING_SYS_FLEN, &pfd->handle));
    if (res == -1)
      {
        return -1;
      }

    // Return the file size.
    st->st_size = res;
    return 0;
  }

}

// ----------------------------------------------------------------------------
// ---- Standard POSIX IO functions ----------------------------------------------------

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

  field_t fields[3];
  fields[0] = (uint32_t) path;
  fields[2] = std::strlen (path);
  fields[1] = (uint32_t) aflags;

  int fh = os::semihosting::call_host (SEMIHOSTING_SYS_OPEN, fields);

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
  struct file* pfd;
  pfd = find_slot (fildes);
  if (pfd == nullptr)
    {
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

  field_t fields[1];
  fields[0] = pfd->handle;

  // Attempt to close the handle.
  int res;
  res = check_error (
      os::semihosting::call_host (SEMIHOSTING_SYS_CLOSE, fields));

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
read (int fildes, void* buf, size_t nbyte)
{
  struct file* pfd;
  pfd = find_slot (fildes);
  if (pfd == nullptr)
    {
      errno = EBADF;
      return -1;
    }

  field_t fields[3];
  fields[0] = pfd->handle;
  fields[1] = (field_t) buf;
  fields[2] = nbyte;

  int res;
  // Returns the number of bytes *not* written.
  res = check_error (os::semihosting::call_host (SEMIHOSTING_SYS_READ, fields));
  if (res == -1)
    {
      return res;
    }

  pfd->pos += nbyte - res;

  // res == nbyte is not an error,
  // at least if we want feof() to work.
  return nbyte - res;
}

ssize_t
write (int fildes, const void* buf, size_t nbyte)
{
  struct file* pfd;
  pfd = find_slot (fildes);
  if (pfd == nullptr)
    {
      errno = EBADF;
      return -1;
    }

  field_t fields[3];

  fields[0] = pfd->handle;
  fields[1] = (field_t) buf;
  fields[2] = nbyte;

  // Returns the number of bytes *not* written.
  int res;
  res = check_error (
      os::semihosting::call_host (SEMIHOSTING_SYS_WRITE, fields));
  /* Clearly an error. */
  if (res < 0)
    {
      return -1;
    }

  pfd->pos += nbyte - res;

  // Did we write 0 bytes?
  // Retrieve errno for just in case.
  if ((nbyte - res) == 0)
    {
      return with_set_errno (0);
    }

  return (nbyte - res);
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
  struct file* pfd;
  pfd = find_slot (fildes);
  if (pfd == nullptr)
    {
      errno = EBADF;
      return 0;
    }

  int tty;
  tty = os::semihosting::call_host (SEMIHOSTING_SYS_ISTTY, &pfd->handle);

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
  memset (buf, 0, sizeof(*buf));
  return stat_impl (fildes, buf);
}

// ----------------------------------------------------------------------------
// ----- POSIX file functions -----

int
stat (const char* path, struct stat* buf)
{
  int fd;
  memset (buf, 0, sizeof(*buf));
  // The best we can do is try to open the file read only.
  // If it exists, then we can guess a few things about it.
  if ((fd = open (path, O_RDONLY)) == -1)
    {
      return -1;
    }
  buf->st_mode |= S_IFREG | S_IREAD;
  int res = stat_impl (fd, buf);
  // Not interested in the error.
  close (fd);
  return res;
}

int
rename (const char* existing, const char* _new)
{
  field_t fields[4];
  fields[0] = (field_t) existing;
  fields[1] = std::strlen (existing);
  fields[2] = (field_t) _new;
  fields[3] = std::strlen (_new);
  return
      check_error (
          os::semihosting::call_host (SEMIHOSTING_SYS_RENAME, fields)) ? -1 : 0;
}

int
unlink (const char* path)
{
  field_t fields[2];
  fields[0] = (field_t) path;
  fields[1] = strlen (path);

  int res;
  res = os::semihosting::call_host (SEMIHOSTING_SYS_REMOVE, fields);
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

  field_t fields[2];
  fields[0] = (field_t) command;
  fields[1] = strlen (command);
  int err = check_error (
      os::semihosting::call_host (SEMIHOSTING_SYS_SYSTEM, fields));
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
gettimeofday (struct timeval* ptimeval, void* ptimezone)
{
  struct timezone* tzp = (struct timezone*) ptimezone;
  if (ptimeval)
    {
      // Ask the host for the seconds since the Unix epoch.
      ptimeval->tv_sec = os::semihosting::call_host (SEMIHOSTING_SYS_TIME,
                                                     nullptr);
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
  timeval = (clock_t) os::semihosting::call_host (SEMIHOSTING_SYS_CLOCK,
                                                  nullptr);

  return timeval;
}

clock_t
times (struct tms* buf)
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

// ----------------------------------------------------------------------------
// ----- POSIX file_system functions -----

// Required by Google Tests
int
mkdir (const char* path, mode_t mode)
{
#if 0
  // always return true
  return 0;
#else
  errno = ENOSYS;
  return -1;
#endif
}

int
rmdir (const char* path)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

void
sync (void)
{
  errno = ENOSYS; // Not implemented
}

// ----------------------------------------------------------------------------
// ----- Directories functions -----

DIR*
opendir (const char* dirpath)
{
  errno = ENOSYS; // Not implemented
  return nullptr;
}

struct dirent*
readdir (DIR* dirp)
{
  errno = ENOSYS; // Not implemented
  return nullptr;
}

int
readdir_r (DIR* dirp, struct dirent* entry, struct dirent** result)
{
  errno = ENOSYS; // Not implemented
  return ((ssize_t) -1);
}

void
rewinddir (DIR* dirp)
{
  errno = ENOSYS; // Not implemented
}

int
closedir (DIR* dirp)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
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
  errno = ENOSYS; // Not implemented
  return -1;
}

int
socketpair (int domain, int type, int protocol, int socket_vector[2])
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
accept (int socket, struct sockaddr* address, socklen_t* address_len)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
bind (int socket, const struct sockaddr* address, socklen_t address_len)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
connect (int socket, const struct sockaddr* address, socklen_t address_len)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
getpeername (int socket, struct sockaddr* address, socklen_t* address_len)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
getsockname (int socket, struct sockaddr* address, socklen_t* address_len)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
getsockopt (int socket, int level, int option_name, void* option_value,
            socklen_t* option_len)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
listen (int socket, int backlog)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

ssize_t
recv (int socket, void* buffer, size_t length, int flags)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

ssize_t
recvfrom (int socket, void* buffer, size_t length, int flags,
          struct sockaddr* address, socklen_t* address_len)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

ssize_t
recvmsg (int socket, struct msghdr* message, int flags)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

ssize_t
send (int socket, const void* buffer, size_t length, int flags)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

ssize_t
sendmsg (int socket, const struct msghdr* message, int flags)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

ssize_t
sendto (int socket, const void* message, size_t length, int flags,
        const struct sockaddr* dest_addr, socklen_t dest_len)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
setsockopt (int socket, int level, int option_name, const void* option_value,
            socklen_t option_len)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
shutdown (int socket, int how)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
sockatmark (int socket)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

// These functions are defined here to avoid linker errors in free
// standing applications. They might be called in some error cases
// from library code.
//
// If you detect other functions to be needed, just let us know
// and we'll add them.

// ----------------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

// ----------------------------------------------------------------------------
// Not yet implemented.

int
select (int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds,
        struct timeval* timeout)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
chdir (const char* path)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

// ----------------------------------------------------------------------------
// Not available via semihosting.

ssize_t
writev (int fildes, const struct iovec* iov, int iovcnt)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
ioctl (int fildes, int request, ...)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
fcntl (int fildes, int cmd, ...)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
ftruncate (int fildes, off_t length)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
fsync (int fildes)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
chmod (const char* path, mode_t mode)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
truncate (const char* path, off_t length)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
utime (const char* path, const struct utimbuf* times)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

// ----------------------------------------------------------------------------
// Unavailable in non-Unix embedded environments.

int
execve (const char* path, char* const argv[], char* const envp[])
{
  errno = ENOSYS; // Not implemented
  return -1;
}

pid_t
fork (void)
{
  errno = ENOSYS; // Not implemented
  return ((pid_t) -1);
}

pid_t
getpid (void)
{
  return 1;
}

int
kill (pid_t pid, int sig)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
raise (int sig)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

pid_t
wait (int* stat_loc)
{
  errno = ENOSYS; // Not implemented
  return ((pid_t) -1);
}

int
chown (const char* path, uid_t owner, gid_t group)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
link (const char* existing, const char* _new)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

int
symlink (const char* existing, const char* _new)
{
  errno = ENOSYS; // Not implemented
  return -1;
}

ssize_t
readlink (const char* path, char* buf, size_t bufsize)
{
  errno = ENOSYS; // Not implemented
  return ((ssize_t) -1);
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

extern "C"
{
  void
  initialise_monitor_handles (void);

  void
  os_terminate (int code);

  void
  os_startup_initialize_args (int* p_argc, char*** p_argv);
}

// ----------------------------------------------------------------------------

void
__attribute__ ((noreturn,weak))
os_terminate (int code)
{
#if ( __SIZEOF_POINTER__ == 4 )
  os::semihosting::call_host (
      SEMIHOSTING_SYS_EXIT,
      (void*) (
          code == 0 ? ADP_STOPPED_APPLICATION_EXIT : ADP_STOPPED_RUN_TIME_ERROR));
#elif ( __SIZEOF_POINTER__ == 8 )
  field_t fields[2];
  fields[0] = ADP_Stopped_ApplicationExit;
  fields[1] = code;
  os::semihosting::call_host (SEMIHOSTING_SYS_EXIT, fields);
#endif

  while (true)
    {
      os::arch::wfi ();
    }
  /* NOTREACHED */
}

// ----------------------------------------------------------------------------

#if !defined(OS_INTEGER_SEMIHOSTING_ARGS_BUF_ARRAY_SIZE)
#define OS_INTEGER_SEMIHOSTING_ARGS_BUF_ARRAY_SIZE 80
#endif /* OS_INTEGER_SEMIHOSTING_ARGS_BUF_ARRAY_SIZE */

#if !defined(OS_INTEGER_SEMIHOSTING_ARGV_BUF_ARRAY_SIZE)
#define OS_INTEGER_SEMIHOSTING_ARGV_BUF_ARRAY_SIZE 10
#endif /* OS_INTEGER_SEMIHOSTING_ARGV_BUF_ARRAY_SIZE */

// This is the semihosting implementation for the routine to
// process args.
// The entire command line is received from the host
// and parsed into strings.

void
os_startup_initialize_args (int* p_argc, char*** p_argv)
{
  // Array of chars to receive the command line from the host.
  static char args_buf[OS_INTEGER_SEMIHOSTING_ARGS_BUF_ARRAY_SIZE];

  // Array of pointers to store the final argv pointers (pointing
  // in the above array).
  static char* argv_buf[OS_INTEGER_SEMIHOSTING_ARGV_BUF_ARRAY_SIZE];

  int argc = 0;
  bool is_in_argument = false;

  field_t fields[2];
  fields[0] = (field_t) args_buf;
  fields[1] = sizeof(args_buf) - 1;
  int ret = os::semihosting::call_host (SEMIHOSTING_SYS_GET_CMDLINE,
                                        fields);
  if (ret == 0)
    {
      // In case the host send more than we can chew, limit the
      // string to our buffer.
      args_buf[sizeof(args_buf) - 1] = '\0';

      // The returned command line is a null terminated string.
      char* p = (char*) fields[0];

      int delim = '\0';
      int ch;

      while ((ch = *p) != '\0')
        {
          if (!is_in_argument)
            {
              if (!isblank (ch))
                {
                  if (argc
                      >= (int) ((sizeof(argv_buf) / sizeof(argv_buf[0])) - 1))
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
      // No args found in string, return a single empty name.
      args_buf[0] = '\0';
      argv_buf[0] = &args_buf[0];
      ++argc;
    }

  // Must end the array with a null pointer.
  argv_buf[argc] = nullptr;

  *p_argc = argc;
  *p_argv = &argv_buf[0];

  // Temporary here.
  initialise_monitor_handles ();

  return;
}

namespace
{
  int monitor_stdin;
  int monitor_stdout;
  int monitor_stderr;
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

  field_t volatile fields[3];

  fields[0] = (field_t) ":tt";
  fields[2] = 3; // length of filename
  fields[1] = 0; // mode "r"
  monitor_stdin = os::semihosting::call_host (SEMIHOSTING_SYS_OPEN,
                                              (void*) fields);

  fields[0] = (field_t) ":tt";
  fields[2] = 3; // length of filename
  fields[1] = 4; // mode "w"
  monitor_stdout = os::semihosting::call_host (SEMIHOSTING_SYS_OPEN,
                                               (void*) fields);

  fields[0] = (field_t) ":tt";
  fields[2] = 3; // length of filename
  fields[1] = 8; // mode "a"
  monitor_stderr = os::semihosting::call_host (SEMIHOSTING_SYS_OPEN,
                                               (void*) fields);

  // If we failed to open stderr, redirect to stdout.
  if (monitor_stderr == -1)
    {
      monitor_stderr = monitor_stdout;
    }

  for (int i = 0; i < OS_INTEGER_SEMIHOSTING_MAX_OPEN_FILES; i++)
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

#endif /* defined(OS_USE_SEMIHOSTING_SYSCALLS) */

// ----------------------------------------------------------------------------

