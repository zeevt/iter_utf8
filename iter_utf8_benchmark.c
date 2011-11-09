#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "utf8_get.h"

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
  char *str = malloc(nbytes);
  if (unlikely(!str))
    perror("malloc");
  if (unlikely(!fread(str, 1, nbytes, f)))
    perror("fread");
  if (unlikely(fclose(f)))
    perror("fclose");
  uint32_t x2 = 0;
  for (int i = 0; i < 100001; i++) {
    uint32_t x = 0;
    for (const char *p = str, *end = str + nbytes; ;) {
      uint32_t u, step;
      if (!(step = utf8_get(p, end, &u)))
        break;
      p += step;
      x ^= u;
    }
    x2 ^= x;
  }
  printf("Anti-Cheat: %08X\n", x2);
/*
  for (const char *p = str, *end = str + nbytes; ;) {
    uint32_t code_point, step;
    if (!(step = utf8_get(p, end, &code_point)))
      break;
    p += step;
    if (code_point == '\n')
      puts("");
    else
      printf("%06X ", code_point);
  }
  puts("");
*/
  free(str);
  return 0;
}
