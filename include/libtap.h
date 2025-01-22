#ifndef LIBTAP_H
#define LIBTAP_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: parser for gh actions, etc

static inline char*
__tap_s_fmt__ (char* fmt, ...) {
  va_list args, args_cp;
  va_start(args, fmt);
  va_copy(args_cp, args);

  // Pass length of zero first to determine number of bytes needed
  int   n   = vsnprintf(NULL, 0, fmt, args) + 1;
  char* buf = (char*)malloc(n);
  if (!buf) {
    return NULL;
  }

  vsnprintf(buf, n, fmt, args_cp);

  va_end(args);
  va_end(args_cp);

  return buf;
}

unsigned int __tap_ok(
  unsigned int       ok,
  const char*        fn_name,
  const char*        file,
  const unsigned int line,
  char*              msg
);

void __tap_skip(unsigned int num_skips, char* msg);

int __tap_write_shared_mem(int status);

#define __tap_lives_or_dies(wants_death, code, ...)             \
  do {                                                          \
    /* set shared memory to 1 */                                \
    __tap_write_shared_mem(1);                                  \
                                                                \
    pid_t pid = fork();                                         \
    switch (pid) {                                              \
      case -1: {                                                \
        perror("fork");                                         \
        exit(EXIT_FAILURE);                                     \
      }                                                         \
      case 0: {                                                 \
        close(STDOUT_FILENO);                                   \
        close(STDERR_FILENO);                                   \
        /* execute test code, then set shared memory to zero */ \
        code __tap_write_shared_mem(0);                         \
        exit(EXIT_SUCCESS);                                     \
      }                                                         \
    }                                                           \
                                                                \
    if (waitpid(pid, NULL, 0) < 0) {                            \
      perror("waitpid");                                        \
      exit(EXIT_FAILURE);                                       \
    }                                                           \
    /* grab prev value (and reset) - if 0, code succeeded */    \
    int test_died = __tap_write_shared_mem(0);                  \
    if (!test_died) {                                           \
      code                                                      \
    }                                                           \
    __tap_ok(                                                   \
      wants_death ? test_died : !test_died,                     \
      __func__,                                                 \
      __FILE__,                                                 \
      __LINE__,                                                 \
      __tap_s_fmt__(__VA_ARGS__)                                \
    );                                                          \
  } while (0)

/* Begin public APIs */

void         todo_start(const char* fmt, ...);
void         todo_end(void);
void         diag(const char* fmt, ...);
void         plan(unsigned int num_tests);
unsigned int exit_status(void);
unsigned int bail_out(const char* fmt, ...);

#define ok(test, ...)          \
  __tap_ok(                    \
    test ? 1 : 0,              \
    __func__,                  \
    __FILE__,                  \
    __LINE__,                  \
    __tap_s_fmt__(__VA_ARGS__) \
  )

#define eq_num(a, b, ...)      \
  __tap_ok(                    \
    a == b ? 1 : 0,            \
    __func__,                  \
    __FILE__,                  \
    __LINE__,                  \
    __tap_s_fmt__(__VA_ARGS__) \
  )

#define neq_num(a, b, ...)     \
  __tap_ok(                    \
    a != b ? 1 : 0,            \
    __func__,                  \
    __FILE__,                  \
    __LINE__,                  \
    __tap_s_fmt__(__VA_ARGS__) \
  )

#define eq_str(a, b, ...)      \
  __tap_ok(                    \
    strcmp(a, b) == 0 ? 1 : 0, \
    __func__,                  \
    __FILE__,                  \
    __LINE__,                  \
    __tap_s_fmt__(__VA_ARGS__) \
  )

#define neq_str(a, b, ...)     \
  __tap_ok(                    \
    strcmp(a, b) == 0 ? 0 : 1, \
    __func__,                  \
    __FILE__,                  \
    __LINE__,                  \
    __tap_s_fmt__(__VA_ARGS__) \
  )

#define eq_null(a, ...) \
  __tap_ok(a == NULL, __func__, __FILE__, __LINE__, __tap_s_fmt__(__VA_ARGS__))

#define neq_null(a, ...) \
  __tap_ok(a != NULL, __func__, __FILE__, __LINE__, __tap_s_fmt__(__VA_ARGS__))

#define is(actual, expected, ...)              \
  __tap_ok(                                    \
    !(actual == expected ? 0                   \
      : !actual          ? -1                  \
      : !expected        ? 1                   \
                         : strcmp(actual, expected)), \
    __func__,                                  \
    __FILE__,                                  \
    __LINE__,                                  \
    __tap_s_fmt__(__VA_ARGS__)                 \
  );

#define skip_start(cond, num_skips, ...)                 \
  do {                                                   \
    if (cond) {                                          \
      __tap_skip(num_skips, __tap_s_fmt__(__VA_ARGS__)); \
      break;                                             \
    }

#define skip_end() \
  }                \
  while (0)

#define skip(test, ...) __tap_skip(1, __tap_s_fmt__(__VA_ARGS__));

#define done_testing()  return exit_status()

#define lives(...)      __tap_lives_or_dies(0, __VA_ARGS__)
#define dies(...)       __tap_lives_or_dies(1, __VA_ARGS__)

#ifdef TAP_WANT_PCRE
unsigned int __tap_match(
  const char*        string,
  const char*        pattern,
  const char*        fn_name,
  const char*        file,
  const unsigned int line,
  char*              msg
);

#  define match_str(string, pattern, ...) \
    __tap_match(                          \
      string,                             \
      pattern,                            \
      __func__,                           \
      __FILE__,                           \
      __LINE__,                           \
      __tap_s_fmt__(__VA_ARGS__)          \
    )
#endif

#ifdef __cplusplus
}
#endif

#endif /* LIBTAP_H */
