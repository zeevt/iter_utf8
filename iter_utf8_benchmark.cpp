#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
//#include <vector>
#include "iter_utf8.hpp"

using namespace std;

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

static void __attribute__((noreturn))
handle_error(const char *err, const char *file_name, int lineno)
{
  fprintf(stderr, "%s @ %s:%d\n", err, file_name, lineno);
  exit(1);
}
#define perror(s) handle_error(s, __FILE__, __LINE__)

#if defined(__clang__)
class C {
public:
  int32_t x;
  C() : x(0) {}
  void operator()(int32_t u) { x ^= u; }
};
#endif

int main(int argc, char *argv[])
{
  if (unlikely(argc < 2)) {
    printf("Usage: iterate_utf8 <file_name>\n");
    return 2;
  }
  size_t nbytes;
  FILE *f = fopen(argv[1], "r");
  if (unlikely(!f))
    perror("fopen");
  {
    struct stat st;
    if (unlikely(fstat(fileno(f), &st)))
      perror("fstat");
    nbytes = st.st_size;
  }
  char *str = new char[nbytes];
  if (unlikely(!str))
    perror("malloc");
  if (unlikely(!fread(str, 1, nbytes, f)))
    perror("fread");
  if (unlikely(fclose(f)))
    perror("fclose");
  uint32_t x2 = 0;
  for (int i = 0; i < 100001; i++) {
#if defined(__clang__)
    C c;
    utf8_foreach_codepoint(str, nbytes, c);
    x2 ^= c.x;
#else
    uint32_t x = 0;
    UTF8 utf8(str, nbytes);
    for (auto iter = utf8.begin(); iter != utf8.end(); ++iter) x ^= *iter;
    x2 ^= x;
#endif
  }
  printf("Anti-Cheat: %08X\n", x2);
/*
  vector<int32_t> vec;
  vec.reserve(nbytes);
  for (int i = 0; i < 100000; i++) {
    vec.clear();
    utf8_foreach_codepoint(str, nbytes, [&vec](int32_t u) { vec.push_back(u); });
  }
  printf("vec.size: %zu\n", vec.size());
*/
/*
  utf8_foreach_codepoint(str, nbytes, [](int32_t code_point) {
    if (code_point == '\n')
      puts("");
    else
      printf("%06X ", code_point);
  });
  puts("");
*/
  delete [] str;
  return 0;
}
