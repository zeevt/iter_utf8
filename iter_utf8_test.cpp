#include <cstdio>
#include "utf8_foreach_codepoint.hpp"

template<bool safe = true>
static int put_utf8(uint32_t u, void *start, const void *one_past_end)
{
  uint8_t *p = reinterpret_cast<uint8_t *>(start);
  const uint8_t * const end_minus_3 = reinterpret_cast<const uint8_t *>(one_past_end) - 3;
  if (u < 0x7F) {
    if (safe && unlikely(p >= end_minus_3))
      if (safe && unlikely(end_minus_3 + 3 - p < 1))
        goto err;
    p[0] = u;
    return 1;
  } else if (u < 0x7FF) {
    if (safe && unlikely(p >= end_minus_3))
      if (safe && unlikely(end_minus_3 + 3 - p < 2))
        goto err;
    p[1] = (u & 0x3F) | 0x80;
    p[0] = (u >> 6) | 0xC0;
    return 2;
  } else if (u < 0xFFFF) {
    if (safe && unlikely(p >= end_minus_3))
      if (safe && unlikely(end_minus_3 + 3 - p < 3))
        goto err;
    p[2] = (u & 0x3F) | 0x80;
    p[1] = ((u >> 6) & 0x3F) | 0x80;
    p[0] = (u >> 12) | 0xE0;
    return 3;
  } else if (u < 0x10FFFF) {
    if (safe && unlikely(p >= end_minus_3))
      if (safe && unlikely(end_minus_3 + 3 - p < 4))
        goto err;
    p[3] = (u & 0x3F) | 0x80;
    p[2] = ((u >> 6) & 0x3F) | 0x80;
    p[1] = ((u >> 12) & 0x3F) | 0x80;
    p[0] = (u >> 18) | 0xF0;
    return 4;
  }
err:
  return 0;
}

int main()
{
  {
    const char *test = "日本語テスト彳";
    utf8_foreach_codepoint(test, strlen(test), [](int32_t u) { printf("%06X ", u); });
    puts("");
  }
  {
    const char *test = "abcd\xE0";
    int n = 0;
    utf8_foreach_codepoint(test, strlen(test), [&n](int32_t u) { printf("%06X ", u); ++n; });
    printf("%d\n", n);
  }
  static const uint32_t min_val[] = {0, 0x80, 0x800, 0x10000};
  const int nbytes = 4000;
  char *test = new char[nbytes];
  for (int seq_len = 0; seq_len < 4; seq_len++) {
    char *p = test;
    for (int i = 0; i < (nbytes >> 2); i++)
      p += put_utf8(min_val[seq_len] + i, p, test + nbytes);
    int n = 0;
    utf8_foreach_codepoint(test, p - test, [seq_len,&n](int32_t u) { if (min_val[seq_len] + n == u) ++n; });
    printf("%d\n", n);
  }
  delete [] test;
  return 0;
}
