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
will_fail ()
{
  test_util *util;
  util->a + util->b;
}

static void
will_succeed ()
{
  10 + 10;
}

int
main ()
{
  pa_setup();

  unsigned int is_skip = 1;

  plan(7);
  assert(pa_match_stdout("1..7\n") == 1);

  assert(ok(1 == 11, "result of %d == %d", 1, 11) == 0);
  assert(
    pa_match_stdout("not ok 1 - result of 1 == 11\n"
                    "# \tFailed test (t/main.c:main at line 41)\n")
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
                    "# \tFailed (TODO)test (t/main.c:main at line 71)\n")
    == 1
  );

  todo_end();

  lives(
    {
      will_succeed();
      printf("test\n");
    },
    "doesn't die"
  );

  assert(pa_match_stdout("ok 9 - doesn't die\n") == 1);

  dies(
    {
      will_fail();
      printf("test\n");
    },
    "dies"
  );

  assert(
    pa_match_stdout("not ok 10 - dies\n"
                    "# \tFailed test (t/main.c:main at line 90)\n")
    == 1
  );

  assert(exit_status() == 3);

  cleanup();

  assert(
    pa_match_stdout("# Planned 7 tests but ran 10\n"
                    "# Failed 3 tests of 10\n")
    == 1
  );

  pa_teardown();
}
