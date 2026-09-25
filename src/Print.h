#pragma once
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

class Print {
public:
  virtual ~Print() = default;
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t *buffer, size_t size) {
    size_t n = 0;
    while (size--) {
      n += write(*buffer++);
    }
    return n;
  }
  virtual void flush() {}

  // Add missing overloads from Print
  size_t print(const char *s) { return write((const uint8_t *)s, strlen(s)); }
  size_t println(const char *s) {
    size_t n = print(s);
    n += print("\r\n");
    return n;
  }
  size_t println(int n) { return println("1"); }

  // Mirrors Arduino core's Print::printf: format into a stack buffer for the
  // common case, heap-allocate only if the formatted output overflows it.
  size_t printf(const char *format, ...) {
    char stackBuf[256];
    va_list args;
    va_start(args, format);
    va_list argsCopy;
    va_copy(argsCopy, args);
    const int needed = vsnprintf(stackBuf, sizeof(stackBuf), format, args);
    va_end(args);
    if (needed < 0) {
      va_end(argsCopy);
      return 0;
    }
    if (static_cast<size_t>(needed) < sizeof(stackBuf)) {
      va_end(argsCopy);
      return write(reinterpret_cast<const uint8_t *>(stackBuf), static_cast<size_t>(needed));
    }
    std::vector<char> heapBuf(static_cast<size_t>(needed) + 1);
    vsnprintf(heapBuf.data(), heapBuf.size(), format, argsCopy);
    va_end(argsCopy);
    return write(reinterpret_cast<const uint8_t *>(heapBuf.data()), static_cast<size_t>(needed));
  }
};
