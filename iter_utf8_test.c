#include <stdio.h>
#include "utf8_get.h"

static int put_utf8(uint32_t u, void *start, const void *one_past_end)
{
  uint8_t *p = (uint8_t *)start;
  const uint8_t * const end_minus_3 = (const uint8_t *)one_past_end - 3;
  if (u < 0x7F) {
    if (unlikely(p >= end_minus_3))
      if (unlikely(end_minus_3 + 3 - p < 1))
        goto err;
    p[0] = u;
    return 1;
  } else if (u < 0x7FF) {
    if (unlikely(p >= end_minus_3))
      if (unlikely(end_minus_3 + 3 - p < 2))
        goto err;
    p[1] = (u & 0x3F) | 0x80;
    p[0] = (u >> 6) | 0xC0;
    return 2;
  } else if (u < 0xFFFF) {
    if (unlikely(p >= end_minus_3))
      if (unlikely(end_minus_3 + 3 - p < 3))
        goto err;
    p[2] = (u & 0x3F) | 0x80;
    p[1] = ((u >> 6) & 0x3F) | 0x80;
    p[0] = (u >> 12) | 0xE0;
    return 3;
  } else if (u < 0x10FFFF) {
    if (unlikely(p >= end_minus_3))
      if (unlikely(end_minus_3 + 3 - p < 4))
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
  /*
  {
    const char *test = "日本語テスト彳";
    const char *end = test + strlen(test);
    for (;;) {
      uint32_t u;
      int step = utf8_get(test, end, &u);
      if (!step)
        break;
      test += step;
      printf("%06X ", u);
    }
    puts("");
  }
  {
    const char *test = "abcd\xE0";
    const char *end = test + strlen(test);
    int n = 0;
    for (;;) {
      uint32_t u;
      int step = utf8_get(test, end, &u);
      if (!step)
        break;
      test += step;
      printf("%06X ", u);
      ++n;
    }
    printf("%d\n", n);
  }
  */
  static const uint32_t min_val[] = {0, 0x80, 0x800, 0x10000};
  const int nbytes = 4000;
  char *test = malloc(nbytes);
  for (int seq_len = 0; seq_len < 4; seq_len++) {
    char *p = test;
    for (int i = 0; i < (nbytes >> 2); i++)
      p += put_utf8(min_val[seq_len] + i, p, test + nbytes);
    const char *end = p;
    int step;
    for (int benchmark = 0; benchmark < 100000; benchmark++) {
      int n = 0;
      p = test;
      for (;;) {
        uint32_t u;
        if (!(step = utf8_get(p, end, &u)))
          break;
        p += step;
        if (min_val[seq_len] + n == u)
          ++n;
      }
      if (unlikely(n != (nbytes >> 2)))
        printf("%d\n", n);
    }
  }
  free(test);
  return 0;
}
