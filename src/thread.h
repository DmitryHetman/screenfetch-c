/*	thread.h
	Author: William Woodruff
	Source Version: 1.2 - Release
	-------------

	This file contains the function prototypes and macros required by thread.c
	Like the rest of screenfetch-c, this file is licensed under the MIT License.
	You should have received a copy of it with this code.
*/

#if defined(__CYGWIN__)
	#include <Windows.h>
	#define THREAD HANDLE /* this is potentially dangerous, HANDLE can be used for things other than threads */
#else
	#include <pthread.h>
	#define THREAD pthread_t
#endif

/* function prototypes */
void create_thread(THREAD* thread, void* (*func_call)(void*), void* arg);
void join_thread(THREAD thread);
void exit_thread();
