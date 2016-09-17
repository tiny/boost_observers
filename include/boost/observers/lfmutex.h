/*!
  @file       lfmutex.h
  @brief      LockFreeMutex class definition

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#pragma once

#include <stdint.h>
#include <atomic>

#ifdef _WIN32
#  include <windows.h>
#  define  thread_self    GetCurrentThreadId
#elif defined(LINUX)
#  define  thread_self    pthread_self
#endif

namespace boost { namespace observers {

#define  hAtomic          std::atomic<uint32_t>

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

    void                 enter() 
                         {
                           uint32_t  tid  = thread_self() ; 
                           uint32_t  zero = 0 ;

                           // lock-free mutex
                           while (std::atomic_compare_exchange_strong( &_lock, &zero, tid ) == false)
                           {
                             if (zero == tid)
                               break ;
                             zero = 0 ;
                           }
                           _cnt++ ;
                         }
    void                 leave() 
                         {
                           _cnt-- ;
                           if (_cnt <= 0)
                             std::atomic_store( &_lock, 0 ) ;
                         }
} ; // class LockFreeMutex

template <class T>
class Scope
{
  private :
    T        &m ;

  public  :
              Scope( T &m_ ) : m(m_) { m.enter() ; }
             ~Scope() { m.leave() ; }
} ; // class Scope

}} ; // namespace

uint32_t thread_pid() 
{
  return thread_self() ; 
} 

