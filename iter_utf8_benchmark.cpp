#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <vector>
#include <algorithm>
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
  {
    UTF8 utf8(str, nbytes);
    uint32_t x2 = 0;
    for (int i = 0; i < 100001; i++) {
      uint32_t x = 0;
      for (auto iter = utf8.begin(); iter != utf8.end(); ++iter) {
        x ^= *iter;
      }
      x2 ^= x;
    }
    printf("Anti-Cheat: %08X\n", x2);
/*
    vector<int32_t> vec;
    vec.reserve(nbytes);
    for (int i = 0; i < 100000; i++) {
      vec.clear();
      for_each(utf8.begin(), utf8.end(), [&vec](int32_t i) { vec.push_back(i); });
    }
    printf("vec.size: %zu\n", vec.size());
*/
/*
    for (auto i = utf8.begin(); i != utf8.end(); ++i) {
      int code_point = *i;
      if (code_point == '\n')
        puts("");
      else
        printf("%06X ", code_point);
    }
    puts("");
*/
  }
  delete [] str;
  return 0;
}
