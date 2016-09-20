/*!
  @file       eventmap.hpp
  @brief      EventMap class definition

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#pragma once

#include "boost/observe/subject.hpp"
#include <map>

namespace boost { namespace observables {

template <class T>
class EventMap
{
  private :
    typedef typename std::map< T, Subject >              _EventMap ;
    typedef typename std::map< T, Subject >::iterator    _EventMap_iter ;
    typedef typename std::map< T, Subject >::value_type  _EventMap_pair ;

#ifdef BOOST_HAS_THREADS
    LockFreeMutex             _lock ;
#endif
    _EventMap                 _events ;
    Subject                   _default ;

  public  :
                              EventMap() {}

    Subject                  &get_default() { return _default ; }
    Subject                  *find( const T &evt_id ) 
                              {
#ifdef BOOST_HAS_THREADS
                                lock_guard<LockFreeMutex>  sc( _lock ) ;
#endif
                                _EventMap_iter  it = _events.find( evt_id ) ;
                                return (it == _events.end()) ? nullptr : &(*it).second ;
                              }
    Subject                  &get( const T &evt_id ) 
                              {
#ifdef BOOST_HAS_THREADS
                                lock_guard<LockFreeMutex>  sc( _lock ) ;
#endif
                                _EventMap_iter  it = _events.find( evt_id ) ;
                                if (it == _events.end())
                                {
                                  _events.insert( _EventMap_pair( evt_id, Subject() )) ;
                                  it = _events.find( evt_id ) ;
                                }
                                return (*it).second ;
                              }
    void                      invoke( const T &evt_id ) 
                              {
#ifdef BOOST_HAS_THREADS
                                lock_guard<LockFreeMutex>  sc( _lock ) ;
#endif
                                Subject *s = find( evt_id ) ;
                                if (s)  s->invoke({evt_id}) ;
                                else _default.invoke({evt_id}) ;
                              }
    void                      invoke( const T &evt_id, const std::vector<boost::any> &args_ ) 
                              {
#ifdef BOOST_HAS_THREADS
                                lock_guard<LockFreeMutex>  sc( _lock ) ;
#endif
                                Subject *s = find( evt_id ) ;

                                std::vector<boost::any> args = {evt_id} ;

                                for (auto it = args_.begin(); it != args_.end(); it++)  args.push_back( (*it) ) ;

//                                std::move( args_.begin(), args_.end(), std::inserter( args, args.end() )) ;

                                if (s)  s->invoke( args ) ;
                                else _default.invoke( args ) ;
                              }
} ; // class EventMap

}} ; // namespace
