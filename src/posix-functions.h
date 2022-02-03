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

#ifndef MICRO_OS_PLUS_SEMIHOSTING_POSIX_FUNCTIONS_H_
#define MICRO_OS_PLUS_SEMIHOSTING_POSIX_FUNCTIONS_H_

// ----------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/select.h>

// ----------------------------------------------------------------------------

#include <sys/times.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C"
{

  struct dirent;

  // The content of this structure is not relevant, it is here just to keep
  // POSIX compatibility, in real life the directory class is used
  // and casted to DIR.
  typedef struct
  {
    // Empty.
  } DIR;

  // Maybe 64-bits?
  typedef uint32_t socklen_t;

  typedef unsigned int sa_family_t;

  struct sockaddr;
}

namespace posix
{
  // -------------------------------------------------------------------------

  // The standard POSIX IO functions. Prototypes are from:
  // http://pubs.opengroup.org/onlinepubs/9699919799/nframe.html

  // The socket definitions are from:
  // http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_socket.h.html

  int __attribute__ ((weak))
  accept (int socket, sockaddr* address, socklen_t* address_len);

  int __attribute__ ((weak))
  bind (int socket, const sockaddr* address, socklen_t address_len);

  int __attribute__ ((weak)) chdir (const char* path);

  int __attribute__ ((weak)) chmod (const char* path, mode_t mode);

  int __attribute__ ((weak))
  chown (const char* path, uid_t owner, gid_t group);

  clock_t __attribute__ ((weak)) clock (void);

  int __attribute__ ((weak)) close (int fildes);

  int __attribute__ ((weak)) closedir (DIR* dirp);

  int __attribute__ ((weak))
  connect (int socket, const sockaddr* address, socklen_t address_len);

  int __attribute__ ((weak))
  execve (const char* path, char* const argv[], char* const envp[]);

  int __attribute__ ((weak)) fcntl (int fildes, int cmd, ...);

  pid_t __attribute__ ((weak)) fork (void);

  int __attribute__ ((weak)) fstat (int fildes, struct stat* buf);

  int __attribute__ ((weak)) ftruncate (int fildes, off_t length);

  int __attribute__ ((weak)) fsync (int fildes);

  char* __attribute__ ((weak)) getcwd (char* buf, size_t size);

  int __attribute__ ((weak))
  getpeername (int socket, sockaddr* address, socklen_t* address_len);

  pid_t __attribute__ ((weak)) getpid (void);

  int __attribute__ ((weak))
  getsockname (int socket, sockaddr* address, socklen_t* address_len);

  int __attribute__ ((weak))
  getsockopt (int socket, int level, int option_name, void* option_value,
              socklen_t* option_len);

  int __attribute__ ((weak)) gettimeofday (timeval* ptimeval, void* ptimezone);

  int __attribute__ ((weak)) ioctl (int fildes, int request, ...);

  int __attribute__ ((weak)) isatty (int fildes);

  int __attribute__ ((weak)) kill (pid_t pid, int sig);

  int __attribute__ ((weak)) link (const char* existing, const char* _new);

  int __attribute__ ((weak)) listen (int socket, int backlog);

  off_t __attribute__ ((weak)) lseek (int fildes, off_t offset, int whence);

  int __attribute__ ((weak)) mkdir (const char* path, mode_t mode);

  int __attribute__ ((weak)) open (const char* path, int oflag, ...);

  DIR* __attribute__ ((weak)) opendir (const char* dirname);

  int __attribute__ ((weak)) raise (int sig);

  ssize_t __attribute__ ((weak)) read (int fildes, void* buf, size_t nbyte);

  dirent* __attribute__ ((weak)) readdir (DIR* dirp);

  int __attribute__ ((weak))
  readdir_r (DIR* dirp, dirent* entry, dirent** result);

  ssize_t __attribute__ ((weak))
  readlink (const char* path, char* buf, size_t bufsize);

  ssize_t __attribute__ ((weak))
  recv (int socket, void* buffer, size_t length, int flags);

  ssize_t __attribute__ ((weak))
  recvfrom (int socket, void* buffer, size_t length, int flags,
            sockaddr* address, socklen_t* address_len);

  ssize_t __attribute__ ((weak))
  recvmsg (int socket, struct msghdr* message, int flags);

  int __attribute__ ((weak)) rename (const char* oldfn, const char* newfn);

  void __attribute__ ((weak)) rewinddir (DIR* dirp);

  int __attribute__ ((weak)) rmdir (const char* path);

  int __attribute__ ((weak))
  select (int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds,
          timeval* timeout);

  ssize_t __attribute__ ((weak))
  send (int socket, const void* buffer, size_t length, int flags);

  ssize_t __attribute__ ((weak))
  sendmsg (int socket, const msghdr* message, int flags);

  ssize_t __attribute__ ((weak))
  sendto (int socket, const void* message, size_t length, int flags,
          const sockaddr* dest_addr, socklen_t dest_len);

  int __attribute__ ((weak))
  setsockopt (int socket, int level, int option_name, const void* option_value,
              socklen_t option_len);

  int __attribute__ ((weak)) shutdown (int socket, int how);

  int __attribute__ ((weak)) sockatmark (int socket);

  int __attribute__ ((weak)) socket (int domain, int type, int protocol);

  int __attribute__ ((weak))
  socketpair (int domain, int type, int protocol, int socket_vector[2]);

  int __attribute__ ((weak)) stat (const char* path, struct stat* buf);

  void __attribute__ ((weak)) sync (void);

  int __attribute__ ((weak)) symlink (const char* existing, const char* _new);

  int __attribute__ ((weak)) system (const char* command);

  clock_t __attribute__ ((weak)) times (tms* buf);

  int __attribute__ ((weak)) truncate (const char* path, off_t length);

  int __attribute__ ((weak)) unlink (const char* name);

  int __attribute__ ((weak))
  utime (const char* path, const struct utimbuf* times);

  pid_t __attribute__ ((weak)) wait (int* stat_loc);

  ssize_t __attribute__ ((weak))
  write (int fildes, const void* buf, size_t nbyte);

  ssize_t __attribute__ ((weak))
  writev (int fildes, const struct iovec* iov, int iovcnt);

} // namespace posix

#endif // __cplusplus

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_SEMIHOSTING_POSIX_FUNCTIONS_H_

// ----------------------------------------------------------------------------
