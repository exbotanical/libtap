#ifndef PRINT_ASSERT_H
#define PRINT_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

void pa_setup(void);

void pa_teardown(void);

void pa_printerr(const char *fmt, ...);

unsigned int pa_match_stdout(const char *pattern);

unsigned int pa_match_stderr(const char *pattern);

#ifdef __cplusplus
}
#endif

#endif /* PRINT_ASSERT_H */
