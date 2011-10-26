#include <cstdio>
#include <cstring>
//#include "iter_utf8.hpp"
#include "utf8_foreach_codepoint.hpp"

int main()
{
  const char *test = "日本語テスト彳";
  utf8_foreach_codepoint(test, strlen(test), [](int32_t u) { printf("%06X ", u); });
  puts("");
/*
  {
    const char *test = "日本語テスト彳";
    UTF8 utf8(test, strlen(test));
    for (auto i = utf8.begin(); i != utf8.end(); ++i)
      printf("%06X ", *i);
    puts("");
  }
  {
    const char *test = "abcd\xE0";
    UTF8 utf8(test, strlen(test));
    int n = 0;
    for (auto i = utf8.begin(); i != utf8.end(); ++i)
      ++n;
    printf("%d\n", n);
  }
  {
    const char *test = "abcd\xE0";
    UTF8 utf8(test, strlen(test));
    int n = 0;
    for (auto i = utf8.begin(); i != utf8.end(); ++i) {
      printf("%06X ", *i);
      ++n;
    }
    printf("%d\n", n);
  }
*/
  return 0;
}
