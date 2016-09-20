/*!
  @file       subject.h
  @brief      Observer class definition

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#pragma once

#include <vector>
#include <functional>
#include <stdarg.h> 
#include <boost/any.hpp>

namespace boost { namespace observers {

class Observer 
{
  protected :
    bool                     _enabled ;

  public    :
                             Observer() { _enabled = true  ; }
    virtual                 ~Observer() { _enabled = false ; }

    virtual void             disable(){ _enabled = false ; }
    virtual void             enable() { _enabled = true  ; }
    virtual bool             enabled(){ return _enabled  ; }
    virtual int              invoke() = 0 ;
    virtual int              invoke( const std::vector<boost::any> &args ) = 0 ;
} ; // class Observer

typedef std::vector<Observer*>            ObserverVec ;
typedef std::vector<Observer*>::iterator  ObserverVec_iter ;

class LambdaPoke : public Observer
{
  protected :
    std::function<void()>    _pf  ;

  public    :
                             LambdaPoke( std::function<void()> pf ) { _pf = pf ; }

    virtual int              invoke() { if (_enabled && (_pf != nullptr)) _pf() ; return 0 ; } 
    virtual int              invoke( const std::vector<boost::any> &args ) { if (_enabled && (_pf != nullptr)) _pf() ; return 0 ; } 
} ; // class LambdaPoke

template <class T>
class MemberPoke : public Observer
{
  protected :
    T                       *_obj ;
    void                (T::*_pf)() ;

  public    :
                             MemberPoke(  T *o, void (T::*func)() ) { _obj = o ; _pf = func ; }

    virtual int              invoke() { if (_enabled && (_pf != nullptr) && (_obj != nullptr)) (_obj->*_pf)() ; return 0 ; } 
    virtual int              invoke( const std::vector<boost::any> &args ) { if (_enabled && (_pf != nullptr) && (_obj != nullptr)) (_obj->*_pf)() ; return 0 ; } 
} ; // class MemberPoke

class Lambda : public Observer
{
  protected :
    std::function<void( const std::vector<boost::any> &)>    _pf  ;

  public    :
                             Lambda( std::function<void( const std::vector<boost::any> &)> pf ) { _pf = pf ; }

    virtual int              invoke() { if (_enabled && (_pf != nullptr)) _pf({}) ; return 0 ; } 
    virtual int              invoke( const std::vector<boost::any> &args ) { if (_enabled && (_pf != nullptr)) _pf(args) ; return 0 ; } 
} ; // class Lambda

template <class T>
class MemberFunc : public Observer
{
  protected :
    T                       *_obj ;
    void                (T::*_pf)( const std::vector<boost::any> &args ) ;

  public    :
                             MemberFunc( T *o, void (T::*func)( const std::vector<boost::any> &args ) ) { _obj = o ; _pf = func ; }

    virtual int              invoke() 
                             { if (_enabled && (_pf != nullptr) && (_obj != nullptr))
                               { 
                                 (_obj->*_pf)({}) ; 
                               }
                               return 0 ; 
                             } 
    virtual int              invoke( const std::vector<boost::any> &args ) 
                             { if (_enabled && (_pf != nullptr) && (_obj != nullptr))
                               { 
                                 (_obj->*_pf)( args ) ; 
                               }
                               return 0 ; 
                             } 
} ; // class MemberFunc

class WinLambda : public Observer
{
  protected :
    std::function<void(uint32_t, uint32_t, uint32_t, uint32_t)>    _pf  ;

  public    :
                             WinLambda( std::function<void(uint32_t, uint32_t, uint32_t, uint32_t)> pf ) { _pf = pf ; }

    virtual int              invoke() { return 0 ; } 
    virtual int              invoke( const std::vector<boost::any> &args ) 
                             { if (_enabled && (_pf != nullptr))
                               {
                                 uint32_t  hw = boost::any_cast<uint32_t>( args[1] ) ;
                                 uint32_t  im = boost::any_cast<uint32_t>( args[2] ) ;
                                 uint32_t  wp = boost::any_cast<uint32_t>( args[3] ) ;
                                 uint32_t  lp = boost::any_cast<uint32_t>( args[4] ) ;
                                 _pf( hw, im, wp, lp ) ;
                               }
                               return 0 ; 
                             } 
} ; // class WinLambda

template <class T>
class WinMemberFunc : public Observer
{
  protected :
    T                       *_obj ;
    void                (T::*_pf)(uint32_t, uint32_t, uint32_t, uint32_t) ;
    va_list                  _args ;

  public    :
                             WinMemberFunc( T *o, void (T::*func)(uint32_t, uint32_t, uint32_t, uint32_t) ) { _obj = o ; _pf = func ; }

    virtual int              invoke() { return 0 ; } 
    virtual int              invoke( const std::vector<boost::any> &args ) 
                             { if (_enabled && (_pf != nullptr))
                               {
                                 uint32_t  hw = boost::any_cast<uint32_t>( args[1] ) ;
                                 uint32_t  im = boost::any_cast<uint32_t>( args[2] ) ;
                                 uint32_t  wp = boost::any_cast<uint32_t>( args[3] ) ;
                                 uint32_t  lp = boost::any_cast<uint32_t>( args[4] ) ;
                                 (_obj->*_pf)( hw, im, wp, lp ) ;
                               }
                               return 0 ; 
                             } 
} ; // class WinMemberFunc

}} ;

