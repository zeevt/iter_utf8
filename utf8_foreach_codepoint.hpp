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
         (uint32_t)(r0 | (r1 << 8) | (r2 << 16) | (r3 << 24));
#else
  return ((p[0] && m0) == r0) && ((p[1] && m1) == r1) &&
         ((p[2] && m2) == r2) && ((p[3] && m3) == r3);
#endif
}

/* TODO:
 * Extract and shift bits using whole int operations.
 * Single unaligned memory read: *(uint32_t*)c into eax, then tag byte is al. Operate on data in registers.
 */

#define PROLOGUE                \
    c = p;                      \
    if (unlikely(p >= end_minus_3)) {   \
      avail = end_minus_3 + 3 - p;      \
      if (unlikely(avail <= 0)) \
        goto done;              \
      memset(buf, 0, 4);        \
      memcpy(buf, p, avail);    \
      c = buf;                  \
    }                           \
    result = c[0];

template<class Callback>
#if defined(__clang__)
void utf8_foreach_codepoint(const void *start, size_t num_bytes, Callback& callback)
#else
void utf8_foreach_codepoint(const void *start, size_t num_bytes, Callback callback)
#endif
{
  const uint8_t *p = reinterpret_cast<const uint8_t *>(start);
  const uint8_t * const end_minus_3 = p + num_bytes - 3;
  const uint8_t *c;
  ptrdiff_t avail;
  uint8_t buf[4];
  int32_t result;
  for (;;) {
    PROLOGUE
full_test_1:
    if (result < 128)
      goto curr_len_1;
full_test_2:
    if (result < 224)
      goto curr_len_2;
full_test_3:
    if (result < 240)
      goto curr_len_3;
full_test_4:
    if (result < 248)
      goto curr_len_4;
err:
    callback(INT32_MIN);
    p += 1;
  }
  for (;;) {
    PROLOGUE
    if (unlikely(result >= 128))
      goto full_test_2;
curr_len_1:
    callback(result);
    p += 1;
  }
  for (;;) {
    PROLOGUE
    if (unlikely(result < 128))
      goto curr_len_1;
    if (unlikely(result >= 224))
      goto full_test_3;
curr_len_2:
    if (unlikely((result < 192) || !mask_cmp_4b(c,0,0xC0,0,0,0,0x80,0,0)))
      goto err;
    result = ((result & 0x1F) << 6) | (c[1] & 0x3F);
    callback(result);
    p += 2;
  }
  for (;;) {
    PROLOGUE
    if (unlikely(result < 224))
      goto full_test_1;
    if (unlikely(result >= 240))
      goto full_test_4;
curr_len_3:
    if (unlikely(!mask_cmp_4b(c,0,0xC0,0xC0,0,0,0x80,0x80,0)))
      goto err;
    result = ((result & 0x0F) << 12) | ((c[1] & 0x3F) << 6) |
              (c[2] & 0x3F);
    callback(result);
    p += 3;
  }
  for (;;) {
    PROLOGUE
    if (unlikely(result < 240))
      goto full_test_1;
    if (unlikely(result >= 248))
      goto err;
curr_len_4:
    if (unlikely(!mask_cmp_4b(c,0,0xC0,0xC0,0xC0,0,0x80,0x80,0x80)))
      goto err;
    result = ((result & 0x07) << 18) | ((c[1] & 0x3F) << 12) |
              ((c[2] & 0x3F) << 6) | (c[3] & 0x3F);
    callback(result);
    p += 4;
  }
done:;
}

#endif /* !defined(UTF8_FOREACH_CODEPOINT_) */
