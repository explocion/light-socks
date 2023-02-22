#include "socket.h"

IPv4Addr::IPv4Addr() : raw() {
  raw = SockAddr<RawAddr>::make();
  raw.sin_family = AF_INET;
}

IPv4Addr::IPv4Addr(std::u16 port) : raw() {
  raw = SockAddr<RawAddr>::make();
  raw.sin_family = AF_INET;
  raw.sin_addr.s_addr = INADDR_ANY;
  raw.sin_port = htons(port);
}

IPv4Addr::IPv4Addr(std::i32 addr, std::u16 port) : raw() {
  raw = SockAddr<RawAddr>::make();
  raw.sin_family = AF_INET;
  raw.sin_addr.s_addr = addr;
  raw.sin_port = htons(port);
}

IPv4Addr::IPv4Addr(const std::string &name, std::u16 port) : raw() {
  raw = SockAddr<RawAddr>::make();
  raw.sin_family = AF_INET;
  hostent *host_of_name = nullptr;
  UNWRAP_POSIX((host_of_name = ::gethostbyname(name.c_str())) == nullptr, void);
  std::memcpy(&(raw.sin_addr), host_of_name->h_addr, host_of_name->h_length);
  raw.sin_port = htons(port);
}

Socket::Socket() : sock_fd(INVALID_FD) {}

Socket::Socket(int fd) : sock_fd(fd) {}

Socket::Socket(int domain, int type, int protocol)
    : sock_fd(socket(domain, type, protocol)) {}

Socket::~Socket() { this->close(); }

Socket::Socket(Socket &&sock) : sock_fd(sock.sock_fd) {
  sock.sock_fd = INVALID_FD;
}

Socket &Socket::operator=(Socket &&sock) {
  this->sock_fd = sock.sock_fd;
  sock.sock_fd = INVALID_FD;
  return (*this);
}

int Socket::get() const { return this->sock_fd; }

int Socket::release() {
  int result = this->sock_fd;
  this->sock_fd = INVALID_FD;
  return result;
}

void Socket::listen(std::u32 backlog) const {
  UNWRAP_POSIX(::listen(this->sock_fd, backlog), void);
}

void Socket::close(void) {
  if (this->sock_fd != INVALID_FD) {
    int cur_fd = this->release();
    UNWRAP_POSIX(::close(cur_fd), void);
  }
}

std::usize Socket::sized_receive(void *buffer, std::usize length,
                                 std::i32 flags) const {
  ssize_t result = 0;
  UNWRAP_POSIX((result = ::recv(this->get(), buffer, length, flags)) < 0, void);
  return result;
}

std::pair<std::usize, std::usize>
Socket::sized_receive_until(void *buffer, std::usize length,
                            std::string_view pattern, std::i32 flags) const {
  std::usize total_received_size = 0, end_of_pattern = std::string_view::npos;
  std::usize current_received_size = 0;
  do {
    current_received_size +=
        this->sized_receive(static_cast<char *>(buffer) + total_received_size,
                            length - total_received_size, flags);
    std::usize offset = (total_received_size > pattern.length())
                            ? (total_received_size - pattern.length())
                            : 0;
    total_received_size += current_received_size;
    if (current_received_size <= 0) {
      total_received_size += current_received_size;
      break;
    }
    std::string_view buf_str(static_cast<const char *>(buffer),
                             total_received_size);
    auto end_of_pattern_found =
        buf_str.find_first_of(pattern, offset) + pattern.length();
    if (end_of_pattern_found != std::string_view::npos) {
      end_of_pattern = end_of_pattern_found;
      break;
    }
  } while (current_received_size > 0 && total_received_size < length);
  return std::make_pair(total_received_size, end_of_pattern);
}

std::pair<std::vector<char>, std::usize>
Socket::receive_until(std::usize size_estimate, std::string_view pattern,
                      std::i32 flags) const {
  std::vector<char> buffer;
  buffer.reserve(size_estimate);
  do {
    auto [current_size, pattern_location] = this->sized_receive_until(
        buffer.data() + buffer.size(), buffer.capacity() - buffer.size(),
        pattern, flags);
    auto offset = buffer.size();
    if (pattern_location == std::string_view::npos) {
      std::usize in_between_start = (buffer.size() > pattern.length())
                                        ? (buffer.size() - pattern.length())
                                        : 0;
      std::string_view in_between(
          buffer.data() + in_between_start,
          std::max(2 * pattern.length(), offset + current_size));
      pattern_location = in_between.find_first_of(pattern);
      if (pattern_location != std::string_view::npos) {
        pattern_location += in_between_start;
      }
    }
    buffer.resize(offset + current_size);
    if (pattern_location != std::string_view::npos) {
      return std::make_pair(std::move(buffer), offset + pattern_location);
    }
  } while (true);
}

std::usize Socket::send(const void *buffer, std::usize length,
                        std::i32 flags) const {
  ssize_t bytes_sent = 0;
  UNWRAP_POSIX((bytes_sent = ::send(this->get(), buffer, length, flags)) < 0,
               void);
  return bytes_sent;
}

std::usize Socket::send_all(const void *buffer, std::usize length,
                            std::i32 flags) const {
  std::usize bytes_sent = 0, current_send = 0;
  do {
    current_send = this->send(static_cast<const char *>(buffer) + bytes_sent,
                              length - bytes_sent, flags);
    bytes_sent += current_send;
    if (current_send == 0) {
      break;
    }
  } while (current_send > 0 && bytes_sent < length);
  return bytes_sent;
}
