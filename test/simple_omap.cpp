/*
  @file       simple_omap.cpp
  @brief      main file for simple-omap test app 

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#include <stdio.h>
#include "boost/observe/omap.hpp"

//-----------------------------------------------------------------------------
//
//
typedef  boost::observables::oMap< uint32_t, std::string >::iterator   oStringMap_iter ;

void on_erase( const std::vector<boost::any> &args )
{
  oStringMap_iter  it = boost::any_cast< oStringMap_iter >( args[0] ) ;

  printf( "erased  : %2ld. %s \n", (*it).first, (*it).second.c_str() ) ;
} // :: on_erase

void on_insert( const std::vector<boost::any> &args )
{
  oStringMap_iter  it = boost::any_cast< oStringMap_iter >( args[0] ) ;

  printf( "inserted: %2ld. %s \n", (*it).first, (*it).second.c_str() ) ;
} // :: on_insert

int main()
{
  boost::observables::oMap< uint32_t, std::string >  key ;

  key.postInsertCB() << new boost::observers::Lambda( on_insert ) ;
  key.preEraseCB  () << new boost::observers::Lambda( on_erase  ) ;
 
  key.insert( 10, "fred"  ) ;
  key.insert(  5, "sally" ) ;
  key.insert( 15, "bob"   ) ;
  key.erase (  5 ) ;
  key.insert( 23, "sue"   ) ;

  return 0 ;
} // :: main


