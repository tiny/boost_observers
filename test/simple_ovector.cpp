/*
  @file       simple_ovector.cpp
  @brief      main file for simple-ovector test app 

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#include <stdio.h>
#include "boost/observables/ovector.hpp"

//-----------------------------------------------------------------------------
//
//
typedef  boost::observables::oVector< std::string >::iterator   oStringVec_iter ;

void on_erase( const std::vector<boost::any> &args )
{
  oStringVec_iter  it = boost::any_cast< oStringVec_iter >( args[0] ) ;

  printf( "erased  :  %s \n", (*it).c_str() ) ;
} // :: on_erase

void on_insert( const std::vector<boost::any> &args )
{
  oStringVec_iter  it = boost::any_cast< oStringVec_iter >( args[0] ) ;

  printf( "inserted:  %s \n", (*it).c_str() ) ;
} // :: on_insert

int main()
{
  boost::observables::oVector< std::string >  vec ;

  vec.postInsertCB() << new boost::observers::Lambda( on_insert ) ;
  vec.preEraseCB  () << new boost::observers::Lambda( on_erase  ) ;
 
  vec.push_back( "fred" ) ;
  vec.push_back( "sally" ) ;
  vec.push_back( "bob" ) ;
  vec.pop_back () ;
  vec.push_back( "sue" ) ;

  return 0 ;
} // :: main


