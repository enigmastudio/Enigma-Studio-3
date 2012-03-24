/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "system.hpp"

void eSleep(eU32 ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec time;

    time.tv_sec = 0;
    time.tv_nsec = ms*1000;

    nanosleep(&time, NULL);
#endif
}

ePtr eThreadStart(void (* func)(ePtr arg), ePtr arg, eBool critical)
{
#ifdef _WIN32
    eU32 tid;
    HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, (LPDWORD)&tid);

    if (critical)
    {
        SetThreadPriority(h, THREAD_PRIORITY_TIME_CRITICAL);
    }

    return (ePtr)h;
#else
    pthread_t *t = new pthread_t;
    eASSERT(t != eNULL);
    pthread_create(t, NULL, (void *(*)(ePtr))func, arg);
    return (ePtr)t;
#endif
}

void eThreadEnd(ePtr handle, eBool wait)
{
#ifdef _WIN32
    if (wait)
    {
        WaitForSingleObject((HANDLE)handle, INFINITE);
    }

    CloseHandle((HANDLE)handle);
    handle = eNULL;
#else
    pthread_t *t = (pthread_t*)handle;
    eASSERT(t != eNULL);
    pthread_join(*t, NULL);
    eSAFE_DELETE(t);
#endif
}

ePtr eCriticalSectionCreate()
{
#ifdef _WIN32
    CRITICAL_SECTION *cs = new CRITICAL_SECTION;
    eASSERT(cs != eNULL);

    InitializeCriticalSection(cs);
    return (ePtr)cs;
#else
    pthread_mutex_t *m = new pthread_mutex_t();
    pthread_mutex_init(m, NULL);    
    return (ePtr)m;
#endif
}

void eCriticalSectionDelete(ePtr handle)
{
#ifdef _WIN32
    CRITICAL_SECTION *cs = (CRITICAL_SECTION*)handle;
    eASSERT(cs != eNULL);
    DeleteCriticalSection(cs);
    eSAFE_DELETE(cs);
#else
    pthread_mutex_t *m = (pthread_mutex_t *)handle;
    eASSERT(m != eNULL);
    pthread_mutex_destroy(m);
    eSAFE_DELETE(m);
#endif
}

void eCriticalSectionEnter(ePtr handle)
{
#ifdef _WIN32
    EnterCriticalSection((LPCRITICAL_SECTION)handle);
#else
    pthread_mutex_lock((pthread_mutex_t *)handle);
#endif
}

void eCriticalSectionLeave(ePtr handle)
{
#ifdef _WIN32
    LeaveCriticalSection((LPCRITICAL_SECTION)handle);
#else
    pthread_mutex_unlock((pthread_mutex_t *)handle);
#endif
}