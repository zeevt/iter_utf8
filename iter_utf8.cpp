#include <cassert>
#include "iter_utf8.hpp"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

utf8_iterator& utf8_iterator::operator++()
{
  if (likely(curr_seq_len_)) {
    p_ += curr_seq_len_;
    curr_seq_len_ = 0;
  } else {
    assert(p_ >= utf8_->s_);
    if (likely(p_ < utf8_->end_)) {
      int curr_seq_len = 1;
      uint8_t v = *p_;
      if (v >= 192) {
        if (v < 224) curr_seq_len = 2;
        else if (v < 240) curr_seq_len = 3;
        else if (v < 248) curr_seq_len = 4;
      }
      p_ = std::min(p_ + curr_seq_len, utf8_->end_);
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
  const uint8_t *c = p_;
  int32_t result;
  assert(c >= utf8_->s_);
  if (unlikely(c >= utf8_->end_))
    goto err;
  curr_seq_len_ = 1;
  result = *c;
  if (result < 128)
    return result;
  if (unlikely(result < 192))
    goto err;
  if (result < 224) {
    // two bytes
    if (unlikely(utf8_->end_ - c < 2))
      goto err;
    curr_seq_len_ = 2;
    result &= ((1 << 5) - 1);
    ADD_PAYLOAD_BYTE(result, c);
    return result;
  }
  if (result < 240) {
    // three bytes
    if (unlikely(utf8_->end_ - c < 3))
      goto err;
    curr_seq_len_ = 3;
    result &= ((1 << 4) - 1);
    ADD_PAYLOAD_BYTE(result, c);
    ADD_PAYLOAD_BYTE(result, c);
    return result;
  }
  if (result < 248) {
    // four bytes
    if (unlikely(utf8_->end_ - c < 4))
      goto err;
    curr_seq_len_ = 4;
    result &= ((1 << 3) - 1);
    ADD_PAYLOAD_BYTE(result, c);
    ADD_PAYLOAD_BYTE(result, c);
    ADD_PAYLOAD_BYTE(result, c);
    return result;
  }
err:
  return INT32_MIN;
}
