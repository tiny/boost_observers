/*!
  @file       ovector.hpp
  @brief      oVector template definition

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#pragma once

#include <boost/observe/subject.hpp>
#include <vector>

namespace boost { namespace observables {

template<class _Value, class _Gate = LockFreeMutex >
class oVector : public std::vector< _Value > 
{
  private:
    typedef std::vector< _Value >     _Parent;
    typedef oVector< _Value, _Gate >  _TGOVector;

#ifdef BOOST_HAS_THREADS
    _Gate          _gate;
#endif
    iterator       _current;
    Subject        _postInsertCB;
    Subject        _preEraseCB;

  public:
                   oVector() 
                   : _postInsertCB( this ), _preEraseCB( this )
                   {}
                   oVector(size_type _N ) 
                   : _Parent( _N ), _postInsertCB( this ), _preEraseCB( this )
                   {}
                   oVector( const _TGOVector& _X) 
                   : _postInsertCB( this ), _preEraseCB( this )
                   {
                     _TGOVector &other = const_cast<_TGOVector &>(_X);
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( other._gate ) ;
#endif
                     insert( begin(), other.begin(), other.end() );
                   }
    virtual       ~oVector() {}

    _TGOVector    &operator=( const _TGOVector &cother_ ) 
                   {
                     _TGOVector &other = const_cast<_TGOVector &>(cother_);
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc1( other._gate ) ;
                     lock_guard<_Gate>  sc2( _gate ) ;
#endif
                     for( _current = begin(); _current != end(); _current++ ) 
                     {
                       _preEraseCB.invoke({ _current, this });
                     }
                     _Parent::clear();
                     for( _current = other.begin(); _current != other.end(); _current++ ) 
                     {
                       _Parent::push_back( (*_current) );
                       _postInsertCB.invoke({ _current, this });
                     }
                     return( *this );
                   }
    void           reserve(size_type _N) 
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     _Parent::reserve( _N );
                   }
    void           resize(size_type _N, _Value x = T() )
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     _Parent::resize( _N, x );
                   }
    void           push_back(const _Value& _X)
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     _Parent::push_back( _X );
                     _current = (end() - 1);    
                     _postInsertCB.invoke({ _current, this });
                   }
    void           pop_back()
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     if( size() < 1 ) 
                     {
                       return;
                     }
                     _current = (end()-1);
                     _preEraseCB.invoke({ _current, this });
                     _Parent::pop_back();
                   }
    void           assign( iterator _F, iterator _L )
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     _Parent::assign( _F, _L );
                   }
    void           assign(size_type _N, const _Value& _X = _Value() )
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     _Parent::assign( _N, _X );
                   }
    iterator       insert(iterator _P, size_type n, const _Value& _X = _Value() )
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     _current = _Parent::insert( _P, n, _X );
                     if( end() != _current ) 
                     {
                       _postInsertCB.invoke({ _current, this });
                     }
                     return( _current );
                   }
    void           insert(iterator _P, iterator _F, iterator _L)
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     if( _F == _L ) 
                     {
                       return;
                     }
                     _current = _P;
                     for( iterator iter = _F; iter != _L; iter++ ) 
                     {
                       insert( _P, (*iter) );
                       _postInsertCB.invoke({ _current, this });
                     }
                   }

    iterator       erase(iterator _P)
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     if( end() != _P ) 
                     {
                       _current = _P;
                       _preEraseCB.invoke({ _current, this });
                     }
                     return( _Parent::erase( _P ) );
                   }
    iterator       erase(iterator _F, iterator _L)
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     for( _current = _F; _current != _L; _current++ ) 
                     {
                       _preEraseCB.invoke({ _current, this });
                     }
                     return( _Parent::erase( _F, _L ) );
                   }
    void           clear()
                   {
#ifdef BOOST_HAS_THREADS
                     lock_guard<_Gate>  sc( _gate ) ;
#endif
                     for( _current = begin(); _current != end(); _current++ ) 
                     {
                       _preEraseCB.invoke({ _current, this });
                     }
                     _Parent::clear();
                   }

    // access methods
#ifdef BOOST_HAS_THREADS
    _Gate         &gate() { return( _gate ); }
#endif
    Subject       &postInsertCB() { return( _postInsertCB ); }
    Subject       &preEraseCB() { return( _preEraseCB ); }
    iterator      &current() { return( _current ); }
} ; // template oVector

}} ; // namespace

