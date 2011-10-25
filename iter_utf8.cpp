#include <cassert>
#include <cstring>
#include <cstddef>
#include "iter_utf8.hpp"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

utf8_iterator& utf8_iterator::operator++()
{
  if (likely(curr_seq_len_)) {
    p_ += curr_seq_len_;
    curr_seq_len_ = 0;
  } else {
    if (likely(p_ < end_)) {
      int curr_seq_len = 1;
      uint8_t v = *p_;
      if (v >= 192) {
        if (v < 224) curr_seq_len = 2;
        else if (v < 240) curr_seq_len = 3;
        else if (v < 248) curr_seq_len = 4;
      }
      p_ = std::min(p_ + curr_seq_len, end_);
    }
  }
  return *this;
}

#define ADD_PAYLOAD_BYTE(result, p)		\
  do { 						\
    result <<= 6;				\
    uint8_t payload = *++(p);		        \
    if (unlikely((payload & 0xc0) != 0x80))	\
      goto err;					\
    result |= payload & ((1 << 6) - 1);		\
  } while (0)

int32_t utf8_iterator::operator*()
{
  union { uint8_t b[4]; uint32_t i; } buf;
  int32_t result;
  const uint8_t *c = p_;
  const ptrdiff_t avail_bytes = end_ - p_;
  if (unlikely(avail_bytes < 4)) {
    if (unlikely(avail_bytes <= 0))
      goto err;
    buf.i = 0;
    memcpy(&buf.b[0], p_, avail_bytes);
    c = &buf.b[0];
  }
  curr_seq_len_ = 1;
  result = *c;
  if (result < 128)
    return result;
  if (unlikely(result < 192))
    goto err;
  if (result < 224) {
    // two bytes
    result &= ((1 << 5) - 1);
    ADD_PAYLOAD_BYTE(result, c);
    curr_seq_len_ = 2;
    return result;
  }
  if (result < 240) {
    // three bytes
    result &= ((1 << 4) - 1);
#if defined(__i386__) || defined(__x86_64__) || defined(__powerpc__)
    if (unlikely((*(const uint16_t *)(c + 1) & 0xC0C0) != 0x8080))
      goto err;
    result = (result << 12) | ((c[1] & 0x3F) << 6) | (c[2] & 0x3F);
#else
    ADD_PAYLOAD_BYTE(result, c);
    ADD_PAYLOAD_BYTE(result, c);
#endif
    curr_seq_len_ = 3;
    return result;
  }
  if (result < 248) {
    // four bytes
    result &= ((1 << 3) - 1);
    ADD_PAYLOAD_BYTE(result, c);
    ADD_PAYLOAD_BYTE(result, c);
    ADD_PAYLOAD_BYTE(result, c);
    curr_seq_len_ = 4;
    return result;
  }
err:
  return INT32_MIN;
}
