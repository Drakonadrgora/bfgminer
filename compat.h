#ifndef __COMPAT_H__
#define __COMPAT_H__

#ifdef WIN32
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#include <windows.h>

// NOTE: Windows strtok uses a thread-local static buffer, so this is safe
#define SETUP_STRTOK_TS  /*nothing needed*/
#define strtok_ts  strtok

#include "miner.h"  // for timersub

static inline int nanosleep(const struct timespec *req, struct timespec *rem)
{
	struct timeval tstart;
	DWORD msecs;

	gettimeofday(&tstart, NULL);
	msecs = (req->tv_sec * 1000) + ((999999 + req->tv_nsec) / 1000000);

	if (SleepEx(msecs, true) == WAIT_IO_COMPLETION) {
		if (rem) {
			struct timeval tdone, tnow, tleft;
			tdone.tv_sec = tstart.tv_sec + req->tv_sec;
			tdone.tv_usec = tstart.tv_usec + ((999 + req->tv_nsec) / 1000);
			if (tdone.tv_usec > 1000000) {
				tdone.tv_usec -= 1000000;
				++tdone.tv_sec;
			}

			gettimeofday(&tnow, NULL);
			if (timercmp(&tnow, &tdone, >))
				return 0;
			timersub(&tdone, &tnow, &tleft);

			rem->tv_sec = tleft.tv_sec;
			rem->tv_nsec = tleft.tv_usec * 1000;
		}
		errno = EINTR;
		return -1;
	}
	return 0;
}

static inline int sleep(unsigned int secs)
{
	struct timespec req, rem;
	req.tv_sec = secs;
	req.tv_nsec = 0;
	if (!nanosleep(&req, &rem))
		return 0;
	return rem.tv_sec + (rem.tv_nsec ? 1 : 0);
}

enum {
	PRIO_PROCESS		= 0,
};

static inline int setpriority(int which, int who, int prio)
{
	return -!SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
}

typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;

#ifndef __SUSECONDS_T_TYPE
typedef long suseconds_t;
#endif

#define PTH(thr) ((thr)->pth.p)
#else /* ! WIN32 */

#define PTH(thr) ((thr)->pth)

#define SETUP_STRTOK_TS  char*_strtok_ts_saveptr
#define strtok_ts(str, delim)  strtok_r(str, delim, &_strtok_ts_saveptr)

#endif /* WIN32 */

#endif /* __COMPAT_H__ */
