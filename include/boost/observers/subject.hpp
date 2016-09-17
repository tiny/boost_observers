/*!
  @file       subject.h
  @brief      Subject class definition

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#pragma once

#include <stdint.h>
#include <atomic>
#include "boost/observers/observer.hpp"
#include "boost/observers/lfmutex.hpp"

namespace boost { namespace observers {

class Subject 
{
    private  :
      short              _block ;        // count of blocks - trigger when first hits 0
      bool               _invoked ;      // true when blocked, then tripped by invoke
      bool               _is_dead ;      // useful for globals that go out of scope(protection mechanism)
      void              *_src ;          // who was the originator of the msgs
      LockFreeMutex      _lock ;
      ObserverVec        _vec ;

    public   :
                         Subject ( void *src_ = nullptr ) 
                         {
                           _block      = 0 ;
                           _invoked    = false ;
                           _is_dead    = false ;
                           _src        = src_ ;
                         }
                         Subject ( const Subject &s ) 
                         {
                           _block      = s._block ;
                           _invoked    = s._invoked ;
                           _is_dead    = s._is_dead ;
                           _src        = s._src ;
                           // vec not being copied
                         }
                        ~Subject () 
                         {
                           clear() ;
                           _is_dead = true ;
                         }

      void               block() { _block++ ; } // disables Observer
      void               clear() 
                         {
                           Scope<LockFreeMutex>  sc( _lock ) ;
                           ObserverVec_iter  it ;
                           for (it = _vec.begin(); it != _vec.end(); it++)
                           {
                             delete( (*it) ) ;
                           }
                           _vec.clear() ;
                         }
      Observer          *install ( Observer *c ) 
                         {
                           if (c == nullptr)
                             return c ;

                           Scope<LockFreeMutex>  sc( _lock ) ;
                           _vec.push_back( c ) ;

                           return c ;
                         }
      void               invoke () 
                          {
                            if (_block > 0)
                            {
                              _invoked = true ;
                              return ;
                            }

                            Scope<LockFreeMutex>  sc( _lock ) ;

                            // locked... do some work
                            ObserverVec_iter  it ;
                            for (it = _vec.begin(); it != _vec.end(); it++)
                            {
                              if ((*it)->invoke() != 0)
                                (*it)->disable() ;
                            }
                            _invoked = false ;
                          }
      void               invoke ( const std::vector<boost::any> &args ) 
                          {
                            if (_block > 0)
                            {
                              _invoked = true ;
                              return ;
                            }
                            Scope<LockFreeMutex>  sc( _lock ) ;

                            // locked... do some work
                            ObserverVec_iter  it ;
                            for (it = _vec.begin(); it != _vec.end(); it++)
                            {
                              if ((*it)->invoke( args ) != 0)
                                (*it)->disable() ;
                            }
                            _invoked = false ;
                          }
      Observer          *remove ( Observer *cb ) 
                          {
                            if (cb == nullptr)
                              return cb ;

                            Scope<LockFreeMutex>  sc( _lock ) ;

                            // locked... do some work
                            ObserverVec_iter  it ;
                            for (it = _vec.begin(); it != _vec.end(); it++)
                            {
                              if ((*it) == cb)
                              {
                                _vec.erase(it) ;
                              }
                            }
                            return cb ;
                          }
      int                unblock() // enables Observer && triggers
                          {
                            if (_block != 0)
                            {
                              _block-- ;
                              if ((_block == 0) && _invoked)
                              {
                                invoke() ;
                              }
                            }
                            return _block ;
                          }
      int                unblock( const std::vector<boost::any> &args ) 
                          {
                            if (_block != 0)
                            {
                              _block-- ;
                              if (_block == 0)
                                invoke(args) ;
                            }
                            return _block ;
                          }

      Subject            &operator<< ( Observer *o ) { if (o) install( o ) ; return *this ; }

      // access methods
      inline bool        enabled() const { return (_block == 0) ; }
      inline size_t      nWatchers() const { return _vec.size() ; }
      LockFreeMutex     &lock() { return _lock ; }
      void              *src() const { return _src ; }
} ; // class Subject

}} ;

