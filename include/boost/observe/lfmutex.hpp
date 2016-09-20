/*!
  @file       lfmutex.hpp
  @brief      LockFreeMutex class definition

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#pragma once

#include <cstdint>
#include <atomic>
#include <boost/predef.h>

#if BOOST_OS_WINDOWS
#  include <windows.h>
#  define  get_thread_id     GetCurrentThreadId
#elif BOOST_OS_LINUX
#  include <pthread.h>
#  define  get_thread_id     pthread_self
#endif

namespace boost { namespace observables {

typedef std::atomic<uint32_t>   hAtomic ;

class LockFreeMutex
{
  private :
    hAtomic              _lock ;
    uint16_t             _cnt ;

  public  :
                         LockFreeMutex() 
                         {
                           _lock = 0 ;
                           _cnt  = 0 ;
                         }
                        ~LockFreeMutex() 
                         {
                           _lock = 0 ;
                           _cnt  = 0 ;
                         }

    void                 lock() 
                         {
#ifdef BOOST_HAS_THREADS
                           uint32_t  tid  = get_thread_id() ; 
                           uint32_t  zero = 0 ;

                           // lock-free mutex
                           while (std::atomic_compare_exchange_strong( &_lock, &zero, tid ) == false)
                           {
                             if (zero == tid)
                               break ;
                             zero = 0 ;
                           }
                           _cnt++ ;
#endif
                         }
    void                 unlock() 
                         {
#ifdef BOOST_HAS_THREADS
                           _cnt-- ;
                           if (_cnt <= 0)
#if BOOST_OS_LINUX
                             _lock.store( 0 ) ;
#else
                             std::atomic_store( &_lock, 0 ) ;
#endif
#endif
                         }
} ; // class LockFreeMutex

}} ; // namespace

