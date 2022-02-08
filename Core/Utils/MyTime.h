#ifndef MY_TIME_H
#define MY_TIME_H


#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif

typedef long long int64;
// http://stackoverflow.com/questions/6013245/are-types-like-uint32-int32-uint64-int64-defined-in-any-stdlib-header
typedef unsigned long long uint64;
typedef long long int64;
typedef unsigned long long uint64;
/* Returns the amount of milliseconds elapsed since the UNIX epoch. Works on both
 * windows and linux. */

int64 GetTimeMs64();

#endif
