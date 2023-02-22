#pragma once

#include "rusty.h"
#include "safe_posix.h"

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <string_view>
#include <vector>

class Socket;
template <typename Derived> class SockAddr;

class IPv4Addr {
public:
  using RawAddr = sockaddr_in;

  RawAddr raw;

  IPv4Addr();
  IPv4Addr(std::u16 port);
  IPv4Addr(std::i32 addr, std::u16 port);
  IPv4Addr(const std::string &name, std::u16 port);
};

class Socket {
protected:
  int sock_fd;

public:
  static constexpr int INVALID_FD = -1;

  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  Socket();
  Socket(int fd);
  Socket(int domain, int type, int protocol = 0);
  ~Socket();

  Socket(Socket &&sock);
  Socket &operator=(Socket &&sock);

  int get() const;
  int release();

  template <typename T> void bind(const T &addr) const {
    UNWRAP_POSIX(::bind(this->sock_fd, &addr, sizeof(addr)), void);
  }

  template <typename T> std::pair<Socket, T> accept() const {
    auto addr = SockAddr<T>::make();
    socklen_t length = sizeof(addr);
    Socket sock = ::accept(this->sock_fd, &addr, &length);
    if (sock.get() == INVALID_FD) {
      PosixError::panic<void>(PosixError());
    }
    return std::make_pair(std::move(sock), std::move(addr));
  }

  template <typename T> void connect(const T &addr) const {
    UNWRAP_POSIX(::connect(this->sock_fd,
                           reinterpret_cast<const sockaddr *>(&addr),
                           sizeof(addr)),
                 void);
  }

  void listen(std::u32 backlog) const;
  void close();

  std::usize sized_receive(void *buffer, std::usize length,
                           std::i32 flags = 0) const;
  std::pair<std::usize, std::usize>
  sized_receive_until(void *buffer, std::usize length, std::string_view pattern,
                      std::i32 flags = 0) const;
  std::pair<std::vector<char>, std::usize>
  receive_until(std::usize size_estimate, std::string_view pattern,
                std::i32 flags = 0) const;

  std::usize send(const void *buffer, std::usize length,
                  std::i32 flags = 0) const;
  std::usize send_all(const void *buffer, std::usize length,
                      std::i32 flags = 0) const;

  template <typename T>
  void set_option(std::i32 level, std::i32 option_name, const T &option_value) {
    UNWRAP_POSIX(::setsockopt(this->get(), level, option_name, &option_value,
                              sizeof(option_value)),
                 void);
  }

  template <typename T>
  void get_option(std::i32 level, std::i32 option_name, T &option_value) {
    socklen_t length = sizeof(option_value);
    UNWRAP_POSIX(
        ::getsockopt(this->get(), level, &option_name, &option_value, &length),
        void);
  }
};

template <typename _Derived> class SockAddr {
public:
  using Derived = _Derived;
  using Self = SockAddr<Derived>;

  static Derived make() {
    Derived addr;
    std::memset(&addr, 0, sizeof(addr));
    return addr;
  }

  static Derived from(const Socket &sock) {
    Derived addr = Self::make();
    socklen_t length = sizeof(addr);
    UNWRAP_POSIX(
        getsockname(sock.get(), reinterpret_cast<sockaddr *>(&addr), &length),
        void);
  }
};
