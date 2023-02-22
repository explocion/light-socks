#pragma once

#include "rusty.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <error.h>
#include <iostream>

class PosixError;

#define SAFE_POSIX(expr)                                                       \
  if (expr) {                                                                  \
    return PosixError();                                                       \
  }

#define THROW_POSIX(expr)                                                      \
  if (expr) {                                                                  \
    throw std::runtime_error(PosixError().to_string());                        \
  }

#define UNWRAP_POSIX(expr, T)                                                  \
  if (expr) {                                                                  \
    PosixError::panic<T>(PosixError());                                        \
  }

class PosixError {
protected:
  error_t error_code;

public:
  using Self = PosixError;

  PosixError();

  std::string to_string() const;

  template <typename T> static T panic(Self &&self) {
    std::cerr << "Posix error" << self.to_string();
    exit(EXIT_FAILURE);
  }
};
