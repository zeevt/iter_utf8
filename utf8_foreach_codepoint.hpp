#ifndef UTF8_FOREACH_CODEPOINT_
#define UTF8_FOREACH_CODEPOINT_

#include <cstdint>
#include <cstddef>
#include <cstring>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

inline bool mask_cmp_4b(
  const uint8_t *p,
  uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3,
  uint8_t r0, uint8_t r1, uint8_t r2, uint8_t r3)
{
#if defined(__i386__) || defined(__x86_64__) || defined(__powerpc__)
  const uint32_t *c = reinterpret_cast<const uint32_t *>(p);
  return (*c & (m0 | (m1 << 8) | (m2 << 16) | (m3 << 24))) ==
         (r0 | (r1 << 8) | (r2 << 16) | (r3 << 24));
#else
  return ((p[0] && m0) == r0) && ((p[1] && m1) == r1) &&
         ((p[2] && m2) == r2) && ((p[3] && m3) == r3);
#endif
}

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
void utf8_foreach_codepoint(const void *start, size_t num_bytes, Callback callback)
{
  const uint8_t *p = reinterpret_cast<const uint8_t *>(start);
  const uint8_t * const end = p + num_bytes;
  const uint8_t *c;
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
      if (likely((result >= 192) && mask_cmp_4b(c,0,0xC0,0,0,0,0x80,0,0))) {
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
      if (likely(mask_cmp_4b(c,0,0xC0,0xC0,0,0,0x80,0x80,0))) {
        result = ((result & 0x0F) << 12) | ((c[1] & 0x3F) << 6) |
                 (c[2] & 0x3F);
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
      if (likely(mask_cmp_4b(c,0,0xC0,0xC0,0xC0,0,0x80,0x80,0x80))) {
        result = ((result & 0x07) << 18) | ((c[1] & 0x3F) << 12) |
                 ((c[2] & 0x3F) << 6) | (c[3] & 0x3F);
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
