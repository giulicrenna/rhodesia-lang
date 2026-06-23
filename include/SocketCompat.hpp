/**
 * @file SocketCompat.hpp
 * @brief Thin portability shim so the net module compiles on POSIX and Windows.
 *
 * Goals:
 *  - On Linux/macOS: behave exactly as before (POSIX sockets, close, errno).
 *  - On Windows (MSVC, MinGW, Strawberry): pull in winsock2 + ws2tcpip,
 *    initialise Winsock once via socket_init(), translate close() → closesocket(),
 *    and map errno to WSAGetLastError() at the macro level.
 *
 * Not included directly here: it must be #included AFTER all standard headers
 * and BEFORE NetworkModule.hpp is textually included by Builtins.hpp.
 */
#ifndef RHODESIA_SOCKET_COMPAT_HPP
#define RHODESIA_SOCKET_COMPAT_HPP

#ifdef _WIN32
  // Pull in Winsock. Winsock must be initialised with WSAStartup before use
  // and torn down with WSACleanup at exit (handled by socket_init/_cleanup).
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0601  // Windows 7+
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h>

  // POSIX aliases used by the net module.
  #define close_socket(fd)  ::closesocket((fd))
  #define LAST_SOCKET_ERROR()  ::WSAGetLastError()
  // EINPROGRESS is defined by winsock2.h via errno.h compatibility shim, but
  // some toolchains need the explicit value.
  #ifndef EINPROGRESS
    #define EINPROGRESS  WSAEINPROGRESS
  #endif
  #ifndef EWOULDBLOCK
    #define EWOULDBLOCK  WSAEWOULDBLOCK
  #endif

  // WSAStartup is required before any socket call.
  inline int socket_init() {
      WSADATA wsa;
      int rc = WSAStartup(MAKEWORD(2, 2), &wsa);
      return rc == 0 ? 0 : -1;
  }
  inline void socket_cleanup() {
      WSACleanup();
  }

  // Non-blocking flag handling: Winsock uses ioctlsocket + FIONBIO.
  inline int socket_set_nonblocking(int fd) {
      u_long mode = 1;
      return ::ioctlsocket(fd, FIONBIO, &mode);
  }
  inline int socket_set_blocking(int fd) {
      u_long mode = 0;
      return ::ioctlsocket(fd, FIONBIO, &mode);
  }
#else
  #include <sys/socket.h>
  #include <sys/select.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <cerrno>

  #define close_socket(fd)     ::close((fd))
  #define LAST_SOCKET_ERROR()  errno

  inline int  socket_init()    { return 0; }
  inline void socket_cleanup() {}

  // Non-blocking flag handling: POSIX uses fcntl + O_NONBLOCK.
  inline int socket_set_nonblocking(int fd) {
      int flags = ::fcntl(fd, F_GETFL, 0);
      if (flags < 0) return -1;
      return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  }
  inline int socket_set_blocking(int fd) {
      int flags = ::fcntl(fd, F_GETFL, 0);
      if (flags < 0) return -1;
      return ::fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
  }
#endif // _WIN32

#endif // RHODESIA_SOCKET_COMPAT_HPP