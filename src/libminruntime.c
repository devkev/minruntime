#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>


// from libbsd's <sys/time.h>
// https://gitlab.freedesktop.org/libbsd/libbsd/-/blob/c2d9d8408869af50fd5802542a4f032375e5e7bc/include/bsd/sys/time.h#L104-142

/* Operations on timespecs. */
#ifndef timespecclear
#define	timespecclear(tsp)		(tsp)->tv_sec = (tsp)->tv_nsec = 0
#endif

#ifndef timespecisset
#define	timespecisset(tsp)		((tsp)->tv_sec || (tsp)->tv_nsec)
#endif

#ifndef timespeccmp
#define	timespeccmp(tsp, usp, cmp)					\
	(((tsp)->tv_sec == (usp)->tv_sec) ?				\
	    ((tsp)->tv_nsec cmp (usp)->tv_nsec) :			\
	    ((tsp)->tv_sec cmp (usp)->tv_sec))
#endif

#ifndef timespecadd
#define	timespecadd(tsp, usp, vsp)					\
	do {								\
		(vsp)->tv_sec = (tsp)->tv_sec + (usp)->tv_sec;		\
		(vsp)->tv_nsec = (tsp)->tv_nsec + (usp)->tv_nsec;	\
		if ((vsp)->tv_nsec >= 1000000000L) {			\
			(vsp)->tv_sec++;				\
			(vsp)->tv_nsec -= 1000000000L;			\
		}							\
	} while (0)
#endif

#ifndef timespecsub
#define	timespecsub(tsp, usp, vsp)					\
	do {								\
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		\
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	\
		if ((vsp)->tv_nsec < 0) {				\
			(vsp)->tv_sec--;				\
			(vsp)->tv_nsec += 1000000000L;			\
		}							\
	} while (0)
#endif



long parse_long_envvar(const char* envvar) {
	long result = 0;
	const char* str = getenv(envvar);
	if (str) {
		errno = 0;
		char* endptr;
		result = strtol(str, &endptr, 10);
		if (errno) {
			fprintf(stderr, "libminruntime: error: unable to parse %s, using 0 instead: %s\n", envvar, strerror(errno));
			result = 0;
		} else if (endptr == str) {
			fprintf(stderr, "libminruntime: error: unable to parse %s, using 0 instead\n", envvar);
			result = 0;
		}
	}
	return result;
}

struct timespec target = { 0, 0 };

static void _libminruntime_init(void) __attribute__((constructor));
static void _libminruntime_fini(void) __attribute__((destructor));

static void _libminruntime_init(void) {
	const char *ld_preload = getenv("LD_PRELOAD");
	if (ld_preload == NULL) {
		// wtf?
	} else {
		// FIXME: remove ourselves properly
		unsigned int i = 0;
		while (ld_preload[i] && ld_preload[i] != ':' && !isspace(ld_preload[i])) {
			i++;
		}
		setenv("LD_PRELOAD", ld_preload + i, 1);
	}

	struct timespec start;
	if (clock_gettime(CLOCK_MONOTONIC, &start) == 0) {
		struct timespec mintime = { parse_long_envvar("MINRUNTIME_SECS"), parse_long_envvar("MINRUNTIME_NSECS") };
		timespecadd(&start, &mintime, &target);
	} else {
		fprintf(stderr, "libminruntime: error: unable to get start time: %s\n", strerror(errno));
	}
}

static void _libminruntime_fini(void) {
	// stderr is already closed, so if this fails, we can't report it anyway.
	while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &target, NULL) == EINTR)
		;
}


