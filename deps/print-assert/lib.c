#include <fnmatch.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#include "print_assert.h"

#define PRINT_BUFFER_SZ 1024

static unsigned int is_running = 0;

static int stdoutfd;
static int stdout_p[2];

static int stderrfd;
static int stderr_p[2];

// TODO: move to libutil/io
static void
write_buf (int fd, const void *buf, size_t sz)
{
  while (sz) {
    ssize_t done;

    done = write(fd, buf, sz);
    if (done <= 0) {
      _exit(1);
    }
    buf  = (const char *)buf + done;
    sz  -= done;
  }
}

static unsigned int
match (int fd, const char *pattern)
{
  char buf[PIPE_BUF + 1];

  int r = read(fd, buf, sizeof(buf) - 1);
  if (r < 0) {
    pa_printerr("failed to read from pipe\n");
    _exit(EXIT_FAILURE);
  }
  buf[r] = '\0';

  if (fnmatch(pattern, buf, 0) != 0) {
    if (strlen(buf) > 100) {
      buf[100] = '\0';
      buf[99]  = '.';
      buf[98]  = '.';
      buf[97]  = '.';
    }
    pa_printerr("expected '%s' but got '%s'\n", pattern, buf);
    return 0;
  }

  return 1;
}

void
pa_printerr (const char *fmt, ...)
{
  char    buf[PRINT_BUFFER_SZ];
  va_list ap;

  va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
  va_end(ap);

  write_buf(stderrfd, buf, strlen(buf));
  write_buf(stderrfd, "\n", 1);
}

void
pa_setup (void)
{
  if (is_running) {
    pa_printerr("setup has already been called");
    _exit(EXIT_FAILURE);
  }
  is_running = 1;

  // Disable buffering so output is immediately accessible
  setbuf(stdout, 0);
  setbuf(stderr, 0);

  stderrfd = dup(STDERR_FILENO);
  if (stderrfd < 0) {
    pa_printerr("failed to dup stderr");
    _exit(EXIT_FAILURE);
  }

  stdoutfd = dup(STDOUT_FILENO);
  if (stdoutfd < 0) {
    pa_printerr("failed to dup stdout");
    _exit(EXIT_FAILURE);
  }

  if (pipe(stdout_p) != 0) {
    pa_printerr("failed to create stdout pipe");
    _exit(EXIT_FAILURE);
  }

  if (pipe(stderr_p) != 0) {
    pa_printerr("failed to create stderr pipe");
    _exit(EXIT_FAILURE);
  }

  if (dup2(stderr_p[1], STDERR_FILENO) < 0 || dup2(stdout_p[1], STDOUT_FILENO) < 0) {
    pa_printerr("failed to dup2 file descriptors");
    _exit(EXIT_FAILURE);
  }
}

void
pa_teardown (void)
{
  close(stderr_p[0]);
  close(stderr_p[1]);
  close(stdout_p[0]);
  close(stdout_p[1]);

  if (dup2(stderrfd, STDERR_FILENO) < 0 || dup2(stdoutfd, STDOUT_FILENO) < 0) {
    pa_printerr("failed to restore file descriptors");
    exit(EXIT_FAILURE);
  }

  close(stderrfd);
  close(stdoutfd);

  setbuf(stderr, NULL);
  setbuf(stdout, NULL);

  is_running = 0;
}

unsigned int
pa_match_stdout (const char *pattern)
{
  return match(stdout_p[0], pattern);
}

unsigned int
pa_match_stderr (const char *pattern)
{
  return match(stderr_p[0], pattern);
}
