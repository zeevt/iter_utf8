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

template<class Callback>
void utf8_foreach_codepoint(
  const void *start, size_t num_bytes, Callback callback)
{
  const uint8_t *s = reinterpret_cast<const uint8_t *>(start),
                *end_minus_3 = s + num_bytes - 3, *p;
  int32_t result;
  uint8_t buf[4];
  for (;;) {
    p = s;
    if (unlikely(s >= end_minus_3)) {
      ptrdiff_t avail = end_minus_3 + 3 - s;
      if (unlikely(avail <= 0))
        break;
      memset(buf, 0, 4);
      memcpy(buf, s, avail);
      p = buf;
    }
    if (p[0] < 128) {
      result = p[0];
      s += 1;
    } else if (p[0] < 224) {
      if (unlikely(p[0] < 192))
        goto err;
      if (unlikely(!mask_cmp_4b(p,0,0xC0,0,0,0,0x80,0,0)))
        goto err;
      result = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
      s += 2;
    } else if (p[0] < 240) {
      if (unlikely(!mask_cmp_4b(p,0,0xC0,0xC0,0,0,0x80,0x80,0)))
        goto err;
      result = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
      s += 3;
    } else if (p[0] < 248) {
      if (unlikely(!mask_cmp_4b(p,0,0xC0,0xC0,0xC0,0,0x80,0x80,0x80)))
        goto err;
      result = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
               ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
      s += 4;
    } else goto err;
ret:
    callback(result);
    continue;
err:
    s += 1;
    result = INT32_MIN;
    goto ret;
  }
}

extern "C" void
utf8_foreach_codepoint_c(
  const void *start, size_t num_bytes, void (*callback)(void *,int32_t), void *opaque)
{
  utf8_foreach_codepoint(start, num_bytes, [callback,opaque](int32_t u){ callback(opaque, u); });
}

#endif /* !defined(UTF8_FOREACH_CODEPOINT_) */
