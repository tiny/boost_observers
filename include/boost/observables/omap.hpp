/*!
  @file       omap.hpp
  @brief      oMap template definition

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#pragma once

#include <boost/observables/subject.hpp>
#include <map>

namespace boost { namespace observables {

template <class Key, class Value, class _Pr = std::less<Key> >
class oMap : public std::map< Key, Value, _Pr >
{
  protected:
    typedef typename oMap<Key,Value,_Pr>::iterator       gomap_iter ;
    typedef typename oMap< Key, Value, _Pr >::value_type gomap_pair ;
    Subject             _preEraseCB;
    Subject             _postInsertCB;
    LockFreeMutex       _gate;
    gomap_iter          _current;

  public:
                        oMap() 
                        : _preEraseCB(this), _postInsertCB(this) 
                        {}
                        oMap( const oMap &other_ )
                        : _preEraseCB(this), _postInsertCB(this) 
                        {
                          *this = other_;
                        }
                       ~oMap() 
                        {}
  oMap                &operator=( const oMap &rhs_ ) 
                        { oMap &other = const_cast<oMap&>(rhs_);
                          lock_guard<LockFreeMutex>  sc1( other._gate ) ;
                          lock_guard<LockFreeMutex>  sc2( _gate ) ;
                          clear();
                          typename std::<Key,Value>::const_iterator  iter ;
                          for (iter = rhs_.begin(); iter != rhs_.end(); iter++)
                            std::map<Key,Value>::insert( std::pair<Key, Value>( (*iter).first, (*iter).second )) ;
                          return( *this );
                        }

  LockFreeMutex          &gate() { return( _gate ); }
  Subject                &preEraseCB() { return( _preEraseCB ); }
  Subject                &postInsertCB() { return( _postInsertCB ); }
  
  std::pair<gomap_iter, bool>  update(const gomap_pair &obj)
                        {
                          lock_guard<LockFreeMutex>  sc( _gate ) ;

                          _current = find(obj.first);
                          // if not already in the map, then only insert
                          if( end() == _current ) 
                          {
                            std::pair<gomap_iter, bool> insert_result = std::map<Key,Value>::insert(obj);
                            _current = insert_result.first;
                            _postInsertCB.invoke({ _current, this }) ;
                            return( insert_result );
                          }
                          // erase current iter in preparation for insert.
                          _preEraseCB.invoke({ _current, this });
                          std::map<Key,Value>::erase(_current);
                          std::pair<gomap_iter, bool> insert_result = std::map<Key,Value>::insert(obj);
                          _current = insert_result.first;
                          _postInsertCB.invoke({ _current, this }) ;
                          return insert_result ;
                        }

  std::pair<gomap_iter, bool>  insert(const Key &k, const Value &v )
                        {
                          return insert( gomap_pair( k, v ) ) ;
                        }
  
  std::pair<gomap_iter, bool>  insert(const gomap_pair& obj)
                        {
                          lock_guard<LockFreeMutex>  sc( _gate ) ;
                          std::pair<gomap_iter, bool> insert_result = std::map<Key,Value>::insert(obj);
                          _current = insert_result.first;
                          if( insert_result.second ) 
                          {
                            _postInsertCB.invoke({ _current, this }) ;
                          }
                          return insert_result ;
                        }

  std::pair<gomap_iter, bool>  insert( gomap_iter pos, const gomap_pair& obj)
                        {
                          lock_guard<LockFreeMutex>  sc( _gate ) ;
                          std::pair<gomap_iter, bool> insert_result = std::map<Key,Value>::insert(pos,obj);
                          _current = insert_result.first;
                          if( insert_result.second ) 
                          {
                          _postInsertCB.invoke({ _current, this }) ;
                          } 
                          return insert_result ;
                        }

  size_t                erase(const Key & key) 
                        {
                          lock_guard<LockFreeMutex>  sc( _gate ) ;
                          _current = find(key);
                          if( end() == _current )
                          { 
                            return( 0 ); 
                          }
                          _preEraseCB.invoke({ _current, this }) ;
                          std::map<Key,Value>::erase(_current);
                          return( 1 );
                        }

  void                  erase(gomap_iter it)
                        {
                          lock_guard<LockFreeMutex>  sc( _gate ) ;
                          if( end() == it )
                          { 
                            return; 
                          }
                          _current = it; // con
                          _preEraseCB.invoke({ _current, this }) ;
                          std::map<Key,Value>::erase(it); 
                        }

  void                  erase(gomap_iter f, gomap_iter l)
                        {
                          lock_guard<LockFreeMutex>  sc( _gate ) ;
                          for( _current = f; _current != l; _current++ )
                          {
                            _preEraseCB.invoke({ _current, this }) ;
                            std::map<Key,Value>::erase(_current); 
                          }
                        }
} ; // template oMap

}} ; // namespace


