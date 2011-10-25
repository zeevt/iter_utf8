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

int32_t utf8_iterator::operator*()
{
  union b4 { uint8_t b[4]; uint32_t i; } buf;
  const union b4 *c = (const union b4 *)p_;
  const ptrdiff_t avail_bytes = end_ - p_;
  if (unlikely(avail_bytes < 4)) {
    if (unlikely(avail_bytes <= 0))
      goto err;
    buf.i = 0;
    memcpy(&buf, p_, avail_bytes);
    c = &buf;
  }
  curr_seq_len_ = 1;
  if (likely(c->b[0] < 128)) {
    return c->b[0];
  } else if (likely(c->b[0] < 224)) {
    if (unlikely(c->b[0] < 192))
      goto err;
    if (unlikely((c->i & 0x0000c000U) != 0x00008000U))
      goto err;
    buf.i = ((c->b[0] & 0x1f) <<  6) | (c->b[1] & 0x3f);
    curr_seq_len_ = 2;
  } else if (likely(c->b[0] < 240)) {
    if (unlikely((c->i & 0x00c0c000U) != 0x00808000U))
      goto err;
    buf.i = ((c->b[0] & 0x0f) << 12) | ((c->b[1] & 0x3f) <<  6) |
            (c->b[2] & 0x3f);
    curr_seq_len_ = 3;
  } else if (likely(c->b[0] < 248)) {
    if (unlikely((c->i & 0xc0c0c000U) != 0x80808000U))
      goto err;
    buf.i = ((c->b[0] & 0x07) << 18) | ((c->b[1] & 0x3f) << 12) |
            ((c->b[2] & 0x3f) <<  6) | (c->b[3] & 0x3f);
    curr_seq_len_ = 4;
  } else {
    goto err;
  }
  return buf.i;
err:
  return INT32_MIN;
}
