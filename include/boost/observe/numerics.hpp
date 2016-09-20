/*!
  @file       numerics.hpp
  @brief      numerics template definition

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef NUMERICS_H
#define NUMERICS_H

#include "boost/observe/subject.hpp"

/*!
  @class Numeric< T >::DivByZero
   
  <b>Description:</b>
   
  <b>Notes:</b>
  
   
  @author     rmcinnis@solidice.com 
  @par        copyright (c) 2004 Solid ICE Technologies, Inc.  All rights reserved.
*/
namespace boost { namespace observables {

template <class T> 
class Numeric 
{
  protected :
    std::atomic<T>      _x ;
    Subject             _valueCB ;

  public    :
                        Numeric() : _valueCB(this), _x( 0 ) {}
                        Numeric( const std::atomic<T> &x ) : _valueCB(this), _x( x ) {}
                        Numeric( const Numeric<T> &i ) : _valueCB(this), _x( i._x ) {}
                       ~Numeric() {}

    inline bool         is_watched() const { return (_valueCB.nWatchers() > 0) ; }
    Subject            &valueCB() { return _valueCB ; }
    Subject            &operator<< ( boost::observers::Observer *o ) { _valueCB << o ; return _valueCB ; }

    // comparison operators
    bool                operator==( const T &x ) const { return (_x == x) ; }
    bool                operator==( const Numeric<T> &i ) const { return (_x == i._x) ; }
    bool                operator!=( const T &x ) const { return (_x != x) ; }
    bool                operator!=( const Numeric<T> &i ) const { return (_x != i._x) ; }
    bool                operator< ( const T &x ) const { return (_x < x) ; }
    bool                operator< ( const Numeric<T> &i ) const { return (_x < i._x) ; }
    bool                operator<=( const T &x ) const { return (_x <= x) ; }
    bool                operator<=( const Numeric<T> &i ) const { return (_x <= i._x) ; }
    bool                operator> ( const T &x ) const { return (_x > x) ; }
    bool                operator> ( const Numeric<T> &i ) const { return (_x > i._x) ; }
    bool                operator>=( const T &x ) const { return (_x >= x) ; }
    bool                operator>=( const Numeric<T> &i ) const { return (_x >= i._x) ; }

    // assignment operators
    Numeric<T>   &operator= ( const std::atomic<T> &x ) 
                        {
                          if (_x == x)  return *this ;
                          if (_valueCB.nWatchers() > 0)
                          {
                            lock_guard<LockFreeMutex>  sc( _valueCB.lock() ) ;
                            std::atomic<T>  old( _x.load() ) ;
                            _x = x.load() ;
                            _valueCB.invoke({ _x.load(), old.load(), (void*)this }) ;
                          }
                          else
                            _x = x.load() ;
                          return *this ;
                        } 
    Numeric<T>   &operator= ( const Numeric<T> &i ) { return (*this = i._x) ; }
    Numeric<T>   &operator+= ( const T &x ) 
                        {
                          if (x == 0)  return *this ;
                          if (_valueCB.nWatchers() > 0)
                          {
                            lock_guard<LockFreeMutex>  sc( _valueCB.lock() ) ;
                            std::atomic<T>  old( _x.load() ) ;
                            _x = _x.load() + x ;
                            _valueCB.invoke({ _x.load(), old.load(), (void*)this }) ;
                          }
                          else
                            _x = _x.load() + x ;
                          return *this ;
                        } 
    Numeric<T>   &operator+= ( const Numeric<T> &i ) { return (*this += i._x) ; }
    Numeric<T>   &operator-= ( const T &x ) 
                        {
                          if (x == 0)  return *this ;
                          if (_valueCB.nWatchers() > 0)
                          {
                            lock_guard<LockFreeMutex>  sc( _valueCB.lock() ) ;
                            std::atomic<T>  old( _x ) ;
                            _x = _x.load - x ;
                            _valueCB.invoke({ _x.load(), old.load(), (void*)this }) ;
                          }
                          else
                            _x = _x.load - x ;
                          return *this ;
                        } 
    Numeric<T>   &operator-= ( const Numeric<T>  &i ) { return (*this -= i._x) ; }
    Numeric<T>   &operator*= ( const T &x ) 
                        {
                          if (x == 1)  return *this ;
                          if (_valueCB.nWatchers() > 0)
                          {
                            lock_guard<LockFreeMutex>  sc( _valueCB.lock() ) ;
                            std::atomic<T>  old( _x ) ;
                            _x = _x.load * x ;
                            _valueCB.invoke({ _x.load(), old.load(), (void*)this }) ;
                          }
                          else
                            _x = _x.load * x ;
                          return *this ;
                        } 
    Numeric<T>   &operator*= ( const Numeric<T> &i ) { return (*this *= i._x) ; }
    Numeric<T>   &operator/= ( const T &x ) 
                        {
                          if (x == 0)  throw Numeric::DivByZero() ;
                          if (x == 1)  return *this ;
                          if (_valueCB.nWatchers() > 0)
                          {
                            lock_guard<LockFreeMutex>  sc( _valueCB.lock() ) ;
                            std::atomic<T>  old( _x ) ;
                            _x = _x.load / x ;
                            _valueCB.invoke({ _x.load(), old.load(), (void*)this }) ;
                          }
                          else
                            _x = _x.load / x ;
                          return *this ;
                        } 
    Numeric<T>   &operator/= ( const Numeric<T> &i ) { return (*this /= i._x) ; }
    Numeric<T>   &operator++ () { return (*this += 1) ; }
    Numeric<T>   &operator++ ( int junk ) { return (*this += 1) ; }
    Numeric<T>   &operator-- () { return (*this -= 1) ; }
    Numeric<T>   &operator-- ( int junk ) { return (*this -= 1) ; }

    // pass thru operators
    T                   operator+ ( const T &x ) const { return (_x + x) ; }
    T                   operator+ ( const Numeric<T> &i ) const { return (i._x + _x) ; }
    T                   operator- ( const T &x ) const { return (_x - x) ; }
    T                   operator- ( const Numeric<T> &i ) const { return (_x - i._x) ; }
    T                   operator* ( const T &x ) const { return (_x * x) ; }
    T                   operator* ( const Numeric<T> &i ) const { return (i._x * _x) ; }
    T                   operator/ ( const T &x ) const { if (x == 0) throw DivByZero() ; return (_x / x) ; }
    T                   operator/ ( const Numeric<T> &i ) const { if (i._x == 0) throw DivByZero() ; return (_x / i._x) ; }

    // cast operators
                        operator T() const { return _x ; }

    // exception objects
    class DivByZero
    {
      public :
         DivByZero() {}
    } ; // Numeric Exception class
} ; // class Numeric

}} ; // namespace

#endif
