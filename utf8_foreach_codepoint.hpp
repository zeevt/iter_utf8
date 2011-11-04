#ifndef UTF8_FOREACH_CODEPOINT_
#define UTF8_FOREACH_CODEPOINT_

#include <cstdint>
#include <cstddef>
#include <cstring>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

static inline bool mask_cmp_4b(
  const uint8_t *p,
  uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3,
  uint8_t r0, uint8_t r1, uint8_t r2, uint8_t r3)
{
#if defined(__i386__) || defined(__x86_64__) || defined(__powerpc__)
  const uint32_t *c = reinterpret_cast<const uint32_t *>(p);
  return (*c & (m0 | (m1 << 8) | (m2 << 16) | (m3 << 24))) ==
         (uint32_t)(r0 | (r1 << 8) | (r2 << 16) | (r3 << 24));
#else
  return ((p[0] && m0) == r0) && ((p[1] && m1) == r1) &&
         ((p[2] && m2) == r2) && ((p[3] && m3) == r3);
#endif
}

#define PROLOGUE                                \
    c = p;                                      \
    if (unlikely(p >= end_minus_3)) {           \
      ptrdiff_t avail = end_minus_3 + 3 - p;    \
      if (unlikely(avail <= 0))                 \
        goto done;                              \
      memset(buf, 0, 4);                        \
      memcpy(buf, p, avail);                    \
      c = buf;                                  \
    }

template<class Callback>
void utf8_foreach_codepoint(
  const void *start, size_t num_bytes, Callback callback)
{
  const uint8_t *p = reinterpret_cast<const uint8_t *>(start),
                *end_minus_3 = p + num_bytes - 3, *c;
  uint8_t buf[4];
  for (;;) {
    PROLOGUE
    if (likely(c[0] < 128)) {
curr_len_1:
      int32_t result = c[0];
      p += 1;
      callback(result);
      continue;
    } else if (c[0] < 224) {
      goto curr_len_2;
    } else if (c[0] < 240) {
      goto curr_len_3;
    } else if (c[0] < 248) {
      goto curr_len_4;
    }
err:
    p += 1;
    callback(INT32_MIN);
  }
  for (;;) {
    PROLOGUE
    if (unlikely(c[0] < 128)) {
      goto curr_len_1;
    } else if (likely(c[0] < 224)) {
curr_len_2:
      if (likely((c[0] >= 192) && mask_cmp_4b(c,0,0xC0,0,0,0,0x80,0,0))) {
        int32_t result = ((c[0] & 0x1F) << 6) | (c[1] & 0x3F);
        p += 2;
        callback(result);
        continue;
      }
    } else if (c[0] < 240) {
      goto curr_len_3;
    } else if (c[0] < 248) {
      goto curr_len_4;
    }
    goto err;
  }
  for (;;) {
    PROLOGUE
    if (unlikely(c[0] < 224)) {
      if (c[0] < 128)
        goto curr_len_1;
      else 
        goto curr_len_2;
    } else if (likely(c[0] < 240)) {
curr_len_3:
      if (likely(mask_cmp_4b(c,0,0xC0,0xC0,0,0,0x80,0x80,0))) {
        int32_t result = ((c[0] & 0x0F) << 12) | ((c[1] & 0x3F) << 6) |
                          (c[2] & 0x3F);
        p += 3;
        callback(result);
        continue;
      }
    } else if (c[0] < 248) {
      goto curr_len_4;
    }
    goto err;
  }
  for (;;) {
    PROLOGUE
    if (unlikely(c[0] < 240)) {
      if (c[0] < 128)
        goto curr_len_1;
      if (c[0] < 224)
        goto curr_len_2;
      else
        goto curr_len_3;
    } else if (likely(c[0] < 248)) {
curr_len_4:
      if (likely(mask_cmp_4b(c,0,0xC0,0xC0,0xC0,0,0x80,0x80,0x80))) {
        int32_t result = ((c[0] & 0x07) << 18) | ((c[1] & 0x3F) << 12) |
                         ((c[2] & 0x3F) << 6)  |  (c[3] & 0x3F);
        p += 4;
        callback(result);
        continue;
      }
    }
    goto err;
  }
done:;
}

extern "C" void
utf8_foreach_codepoint_c(
  const void *start, size_t num_bytes, void (*callback)(void *,int32_t), void *opaque)
{
  utf8_foreach_codepoint(start, num_bytes, [callback,opaque](int32_t u){ callback(opaque, u); });
}

#endif /* !defined(UTF8_FOREACH_CODEPOINT_) */
