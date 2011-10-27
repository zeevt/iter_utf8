#include <cstdio>
#include "utf8_foreach_codepoint.hpp"

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
  return 0;
}
