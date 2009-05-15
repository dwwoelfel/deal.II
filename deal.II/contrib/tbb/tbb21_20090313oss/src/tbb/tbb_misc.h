/*
    Copyright 2005-2009 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks.

    Threading Building Blocks is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    Threading Building Blocks is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Threading Building Blocks; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/

#ifndef _TBB_tbb_misc_H
#define _TBB_tbb_misc_H

#include "tbb/tbb_stddef.h"
#include "tbb/tbb_machine.h"

#if _WIN32||_WIN64
#include <windows.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#elif defined(__sun)
#include <sys/sysinfo.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(__FreeBSD__)
#include <unistd.h>
#endif

#if USE_PTHREAD
#include <pthread.h>
#endif /* USE_PTHREAD */

namespace tbb {

namespace internal {

#if defined(__TBB_DetectNumberOfWorkers)
static inline int DetectNumberOfWorkers() {
    return __TBB_DetectNumberOfWorkers(); 
}

#else

#if _WIN32||_WIN64
static inline int DetectNumberOfWorkers() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return static_cast<int>(si.dwNumberOfProcessors);
}

#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__sun) 
static inline int DetectNumberOfWorkers() {
    long number_of_workers;

#if (defined(__FreeBSD__) || defined(__sun)) && defined(_SC_NPROCESSORS_ONLN) 
    number_of_workers = sysconf(_SC_NPROCESSORS_ONLN);

// In theory, sysconf should work everywhere.
// But in practice, system-specific methods are more reliable
#elif defined(__linux__)
    number_of_workers = get_nprocs();
#elif defined(__APPLE__)
    int name[2] = {CTL_HW, HW_AVAILCPU};
    int ncpu;
    size_t size = sizeof(ncpu);
    sysctl( name, 2, &ncpu, &size, NULL, 0 );
    number_of_workers = ncpu;
#else
#error DetectNumberOfWorkers: Method to detect the number of online CPUs is unknown
#endif

// Fail-safety strap
    if ( number_of_workers < 1 ) {
        number_of_workers = 1;
    }
    
    return number_of_workers;
}

#else
#error DetectNumberOfWorkers: OS detection method is unknown

#endif /* os kind */

#endif

// assertion_failure is declared in tbb/tbb_stddef.h because it user code
// needs to see its declaration.

//! Throw std::runtime_error of form "(what): (strerror of error_code)"
/* The "what" should be fairly short, not more than about 64 characters.
   Because we control all the call sites to handle_perror, it is pointless
   to bullet-proof it for very long strings.

   Design note: ADR put this routine off to the side in tbb_misc.cpp instead of
   Task.cpp because the throw generates a pathetic lot of code, and ADR wanted
   this large chunk of code to be placed on a cold page. */
void __TBB_EXPORTED_FUNC handle_perror( int error_code, const char* what );

//! True if environment variable with given name is set and not 0; otherwise false.
bool GetBoolEnvironmentVariable( const char * name );

//! Print TBB version information on stderr
void PrintVersion();

//! Print extra TBB version information on stderr
void PrintExtraVersionInfo( const char* category, const char* description );

//! Class that implements exponential backoff.
/** See implementation of SpinwaitWhileEq for an example. */
class ExponentialBackoff {
    //! Time delay, in units of "pause" instructions. 
    /** Should be equal to approximately the number of "pause" instructions
        that take the same time as an context switch. */
    static const int LOOPS_BEFORE_YIELD = 0x10;
    int count;
public:
    ExponentialBackoff() : count(1) {}

    //! Pause for a while.
    void pause() {
        if( count<=LOOPS_BEFORE_YIELD ) {
            __TBB_Pause(count);
            // Pause twice as long the next time.
            count*=2;
        } else {
            // Pause is so long that we might as well yield CPU to scheduler.
            __TBB_Yield();
        }
    }
    void reset() {
        count = 1;
    }
};

//! Spin WHILE the value of the variable is equal to a given value
/** T and U should be comparable types. */
template<typename T, typename U>
static inline void SpinwaitWhileEq( const volatile T& location, U value ) {
    ExponentialBackoff backoff;
    while( location==value ) {
        backoff.pause();
    }
}

//! Spin UNTIL the value of the variable is equal to a given value
/** T and U should be comparable types. */
template<typename T, typename U>
static inline void SpinwaitUntilEq( const volatile T& location, const U value ) {
    ExponentialBackoff backoff;
    while( location!=value ) {
        backoff.pause();
    }
}

typedef void (*tls_dtor_t)(void*);

//! Class to use native TLS support directly
template <typename T>
class tls {
#if USE_WINTHREAD
    typedef DWORD tls_key_t;
public:
    int create() {
        tls_key_t tmp = TlsAlloc();
        if( tmp==TLS_OUT_OF_INDEXES )
            return TLS_OUT_OF_INDEXES;
        my_key = tmp;
        return 0;
    }
    int  destroy()      { TlsFree(my_key); my_key=0; return 0; }
    void set( T value ) { TlsSetValue(my_key, (LPVOID)value); }
    T    get()          { return (T)TlsGetValue(my_key); }
#else /* USE_PTHREAD */
    typedef pthread_key_t tls_key_t;
public:
    int  create( tls_dtor_t dtor = NULL ) {
        return pthread_key_create(&my_key, dtor);
    }
    int  destroy()      { return pthread_key_delete(my_key); }
    void set( T value ) { pthread_setspecific(my_key, (void*)value); }
    T    get()          { return (T)pthread_getspecific(my_key); }
#endif
private:
    tls_key_t my_key;
};

} // namespace internal

} // namespace tbb

#endif /* _TBB_tbb_misc_H */
