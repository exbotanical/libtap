#include <assert.h>
#include <fnmatch.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libtap.h"
#include "print-assert/print_assert.h"
#include "tap.c"

typedef struct {
  int a;
  int b;
} test_util;

static void
will_fail () {
  test_util *util;
  util->a + util->b;
}

static void
will_succeed () {
  10 + 10;
}

int
main () {
  pa_setup();

  unsigned int is_skip = 1;

  plan(7);
  assert(pa_match_stdout("1..7\n") == 1);

  assert(ok(1 == 11, "result of %d == %d", 1, 11) == 0);
  assert(
    pa_match_stdout("not ok 1 - result of 1 == 11\n"
                    "# \tFailed test (t/main.c:main at line 38)\n")
    == 1
  );

  assert(ok(1 == 1, "okb!") == 1);
  assert(pa_match_stdout("ok 2 - okb!\n") == 1);

  skip_start(is_skip, 3, "not needed");
  assert(ok(1 == 1, "msg1") == 1);
  assert(ok(1 == 12, "msg2") == 0);
  assert(ok(12 == 12, "msg3") == 1);
  skip_end();

  assert(
    pa_match_stdout("ok 3 # SKIP not needed\n"
                    "ok 4 # SKIP not needed\n"
                    "ok 5 # SKIP not needed\n")
    == 1
  );

  assert(ok(100 == 100, "msg") == 1);
  assert(pa_match_stdout("ok 6 - msg\n") == 1);

  todo_start("is todo man");
  assert(ok(1 == 1, "the todo test") == 1);
  assert(pa_match_stdout("ok 7 - the todo test # TODO is todo man\n") == 1);

  assert(ok(1 == 19, "the second todo test") == 0);
  assert(
    pa_match_stdout("not ok 8 - the second todo test # TODO is todo man\n"
                    "# \tFailed (TODO)test (t/main.c:main at line 68)\n")
    == 1
  );

  todo_end();

  lives({ will_succeed(); }, "doesn't die");

  assert(pa_match_stdout("ok 9 - doesn't die\n") == 1);

  dies({ will_fail(); }, "dies");

  assert(
    pa_match_stdout("not ok 10 - dies\n"
                    "# \tFailed test (t/main.c:main at line 81)\n")
    == 1
  );

  char *x  = strdup("x");
  char *y  = strdup("y");
  char *a  = strdup("a");
  char *a2 = strdup("a");
  char *b  = strdup("b");
  char *n  = NULL;

  is(a, b, "not equal");
  assert(
    pa_match_stdout("not ok 11 - not equal\n"
                    "# \tFailed test (t/main.c:main at line 96)\n")
    == 1
  );

  is(a, a2, "equal");
  assert(pa_match_stdout("ok 12 - equal\n") == 1);

  is(n, NULL, "both null");
  assert(pa_match_stdout("ok 13 - both null\n") == 1);

  assert(exit_status() == 6);
  cleanup();

  assert(
    pa_match_stdout("# Planned 7 tests but ran 13\n"
                    "# Failed 4 tests of 13\n")
    == 1
  );

  pa_teardown();
  free(x);
  free(y);
  free(a);
  free(a2);
  free(b);
}
