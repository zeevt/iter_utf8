#ifndef UTF8_FOREACH_CODEPOINT_
#define UTF8_FOREACH_CODEPOINT_

#include <cstdint>
#include <cstddef>
#include <cstring>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define PROLOGUE                \
    c = p;                      \
    avail = end - p;            \
    if (unlikely(avail <= 3)) { \
      if (unlikely(avail <= 0)) \
        goto done;              \
      memset(buf, 0, 4);        \
      memcpy(buf, p, avail);    \
      c = buf;                  \
    }                           \
    result = c[0];

template<class Callback>
void utf8_foreach_codepoint(const void *s, size_t n, Callback callback)
{
  const uint8_t *p = reinterpret_cast<const uint8_t *>(s);
  const uint8_t *c = p;
  const uint8_t * const end = p + n;
  ptrdiff_t avail;
  uint8_t buf[4];
  int32_t result;
  for (;;) {
    PROLOGUE
    if (likely(result < 128)) {
curr_len_1:
      callback(result);
      p += 1;
      continue;
    } else if (result < 224) {
      goto curr_len_2;
    } else if (result < 240) {
      goto curr_len_3;
    } else if (result < 248) {
      goto curr_len_4;
    }
err:
    callback(INT32_MIN);
    p += 1;
  }
  for (;;) {
    PROLOGUE
    if (unlikely(result < 128)) {
      goto curr_len_1;
    } else if (likely(result < 224)) {
curr_len_2:
      if (likely((result >= 192) && ((c[1] & 0xC0) == 0x80))) {
        result = ((result & 0x1F) << 6) | (c[1] & 0x3F);
        callback(result);
        p += 2;
        continue;
      }
    } else if (result < 240) {
      goto curr_len_3;
    } else if (result < 248) {
      goto curr_len_4;
    }
    goto err;
  }
  for (;;) {
    PROLOGUE
    if (unlikely(result < 128)) {
      goto curr_len_1;
    } else if (unlikely(result < 224)) {
      goto curr_len_2;
    } else if (likely(result < 240)) {
curr_len_3:
      if (likely(((c[1] & 0xC0) == 0x80) && ((c[2] & 0xC0) == 0x80))) {
        result = ((result & 0x1F) << 12) | ((c[1] & 0x3F) << 6) | (c[2] & 0x3F);
        callback(result);
        p += 3;
        continue;
      }
    } else if (result < 248) {
      goto curr_len_4;
    }
    goto err;
  }
  for (;;) {
    PROLOGUE
    if (unlikely(result < 128)) {
      goto curr_len_1;
    } else if (unlikely(result < 224)) {
      goto curr_len_2;
    } else if (unlikely(result < 240)) {
      goto curr_len_3;
    } else if (likely(result < 248)) {
curr_len_4:
      if (likely(((c[1] & 0xC0) == 0x80) && ((c[2] & 0xC0) == 0x80) && ((c[3] & 0xC0) == 0x80))) {
        result = ((result & 0x1F) << 18) | ((c[1] & 0x3F) << 12) | ((c[2] & 0x3F) << 6) | (c[3] & 0x3F);
        callback(result);
        p += 4;
        continue;
      }
    }
    goto err;
  }
done:;
}

#endif /* !defined(UTF8_FOREACH_CODEPOINT_) */
