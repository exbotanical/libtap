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
  ssize_t a;
  ssize_t b;
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

  size_t is_skip = 1;

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

  eq_str("xXx", "xXx", "strings equal");
  assert(pa_match_stdout("ok 14 - strings equal\n") == 1);

  neq_str("xXX", "xXx", "strings not equal");
  assert(pa_match_stdout("ok 15 - strings not equal\n") == 1);

  eq_null(NULL, "is NULL");
  assert(pa_match_stdout("ok 16 - is NULL\n") == 1);

  neq_null("xXx", "not NULL");
  assert(pa_match_stdout("ok 17 - not NULL\n") == 1);

  eq_num(3, 3, "uints equal");
  assert(pa_match_stdout("ok 18 - uints equal\n") == 1);

  eq_num(-3, -3, "signed ints equal");
  assert(pa_match_stdout("ok 19 - signed ints equal\n") == 1);

  eq_num(3.3, 3.3, "floats equal");
  assert(pa_match_stdout("ok 20 - floats equal\n") == 1);

  neq_num(3, 4, "uints not equal");
  assert(pa_match_stdout("ok 21 - uints not equal\n") == 1);

  neq_num(-3, -4, "signed ints not equal");
  assert(pa_match_stdout("ok 22 - signed ints not equal\n") == 1);

  neq_num(3.3, 3.4, "floats not equal");
  assert(pa_match_stdout("ok 23 - floats not equal\n") == 1);

  assert(exit_status() == 16);
  cleanup();

  assert(
    pa_match_stdout("# Planned 7 tests but ran 23\n"
                    "# Failed 4 tests of 23\n")
    == 1
  );

  pa_teardown();
  free(x);
  free(y);
  free(a);
  free(a2);
  free(b);
}
