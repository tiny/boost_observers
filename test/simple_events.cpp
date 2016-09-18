/*!
  @file       simple_events.cpp
  @brief      main test file for events observer example

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#include <stdio.h>
#include "boost/observables/eventmap.hpp"
#include <boost/any.hpp>
#include <vector> 

using namespace boost ;

typedef  boost::observables::EventMap< uint32_t >   Events ;

enum FakeMsgId { FM_LBUTTONDN, FM_RBUTTONDN, FM_LBUTTONUP, FM_RBUTTONUP, FM_CREATE, FM_DESTROY, FM_MOUSEMOVE } ;

class FakeWindow
{
  protected :
    Events         evts ; 

    virtual void   hook_msgs() 
                   {
                     evts.get( FM_LBUTTONDN ) << new observers::MemberFunc< FakeWindow> ( this, &FakeWindow :: on_lbutton_dn ) ;
                     evts.get( FM_RBUTTONDN ) << new observers::MemberFunc< FakeWindow> ( this, &FakeWindow :: on_rbutton_dn ) ;
                     evts.get( FM_LBUTTONUP ) << new observers::MemberFunc< FakeWindow> ( this, &FakeWindow :: on_lbutton_up ) ;
                     evts.get( FM_RBUTTONUP ) << new observers::MemberFunc< FakeWindow> ( this, &FakeWindow :: on_rbutton_up ) ;
                     evts.get( FM_MOUSEMOVE ) << new observers::MemberFunc< FakeWindow> ( this, &FakeWindow :: on_mouse_move ) ;
                   }
    virtual void   on_lbutton_dn( const std::vector<boost::any> &args ) 
                   { printf( "on_lbutton_dn   wp: %0x04lx   lp: %0x04lx \n", boost::any_cast<uint32_t>( args[1] ), boost::any_cast<uint32_t>( args[2] ) ) ; }
    virtual void   on_lbutton_up( const std::vector<boost::any> &args ) 
                   { printf( "on_lbutton_up   wp: %0x04lx   lp: %0x04lx \n", boost::any_cast<uint32_t>( args[1] ), boost::any_cast<uint32_t>( args[2] ) ) ; }
    virtual void   on_rbutton_dn( const std::vector<boost::any> &args ) 
                   { printf( "on_rbutton_dn   wp: %0x04lx   lp: %0x04lx \n", boost::any_cast<uint32_t>( args[1] ), boost::any_cast<uint32_t>( args[2] ) ) ; }
    virtual void   on_rbutton_up( const std::vector<boost::any> &args ) 
                   { printf( "on_rbutton_up   wp: %0x04lx   lp: %0x04lx \n", boost::any_cast<uint32_t>( args[1] ), boost::any_cast<uint32_t>( args[2] ) ) ; }
    virtual void   on_mouse_move( const std::vector<boost::any> &args ) 
                   { printf( "on_mouse_move   wp: %0x04lx   lp: %0x04lx \n", boost::any_cast<uint32_t>( args[1] ), boost::any_cast<uint32_t>( args[2] ) ) ; }

  public    :
    uint32_t       hwnd ;

                   FakeWindow( uint32_t hwnd_ ) { hwnd = hwnd_ ; }
    virtual       ~FakeWindow() { destroy() ; }

    virtual void   create() { hook_msgs() ; }
    virtual void   destroy() {}
    virtual void   process( uint32_t imsg, uint32_t wp, uint32_t lp ) 
                   {
                     auto it = evts.find( imsg ) ;
                     if (it == nullptr)
                     {
                       printf( "hwnd: %ld  imsg: %ld  wp: %ld  lp: %ld \n", hwnd, imsg, wp, lp ) ;
                       return ;
                     }
                     else
                     {
                       evts.get( imsg ).invoke({ imsg,  wp, lp }) ;
                     }
                   }
} ; // class FakeWindow

typedef  std::map< uint32_t, FakeWindow* >                 WindowMap ;
typedef  std::map< uint32_t, FakeWindow* >::iterator       WindowMap_iterator ;
typedef  std::map< uint32_t, FakeWindow* >::value_type     WindowMap_pair ;

void FakeWindow_process( WindowMap &wnds, uint32_t hwnd, uint32_t imsg, uint32_t wp, uint32_t lp )
{
  WindowMap_iterator  it = wnds.find( hwnd ) ;
  if (it != wnds.end())
  {
    (*it).second->process( imsg, wp, lp ) ;
  }
} // :: FakeWindow_process

void test_fake_window_events()
{
  WindowMap   wnds ;
  FakeWindow  w1( 1 ) ;
  FakeWindow  w2( 2 ) ;
  FakeWindow  w3( 3 ) ;

  wnds.insert( WindowMap_pair( w1.hwnd, &w1 )) ;
  wnds.insert( WindowMap_pair( w2.hwnd, &w2 )) ;
  wnds.insert( WindowMap_pair( w3.hwnd, &w3 )) ;

  FakeWindow_process( wnds, 1, FM_LBUTTONDN, 0x01, 0x0001 ) ;
  FakeWindow_process( wnds, 1, FM_MOUSEMOVE, 0x02, 0x0001 ) ;
  FakeWindow_process( wnds, 1, FM_MOUSEMOVE, 0x03, 0x0001 ) ;
  FakeWindow_process( wnds, 1, FM_MOUSEMOVE, 0x04, 0x0001 ) ;
  FakeWindow_process( wnds, 1, FM_LBUTTONUP, 0x05, 0x0001 ) ;
  FakeWindow_process( wnds, 2, FM_RBUTTONDN, 0x06, 0x0001 ) ;
  FakeWindow_process( wnds, 2, FM_MOUSEMOVE, 0x07, 0x0001 ) ;
  FakeWindow_process( wnds, 2, FM_MOUSEMOVE, 0x08, 0x0001 ) ;
  FakeWindow_process( wnds, 2, FM_RBUTTONUP, 0x09, 0x0001 ) ;

} // :: test_fake_window_events

void test_string_events()
{
  boost::observables::EventMap< std::string >  evt ;
  evt.get( "sam" ) << new observers::Lambda( [](const std::vector<boost::any> &args){ printf( "%s here.\n", boost::any_cast<std::string>( args[0] ).c_str() ) ; }) ;
  evt.get( "sue" ) << new observers::Lambda( [](const std::vector<boost::any> &args){ printf( "%s here.\n", boost::any_cast<std::string>( args[0] ).c_str() ) ; }) ;
  evt.get( "bob" ) << new observers::Lambda( [](const std::vector<boost::any> &args){ printf( "%s here.\n", boost::any_cast<std::string>( args[0] ).c_str() ) ; }) ;
  evt.default()    << new observers::Lambda( [](const std::vector<boost::any> &args){ printf( "%s not here.\n", boost::any_cast<std::string>( args[0] ).c_str() ) ; }) ;

  evt.invoke( "sam" ) ;
  evt.invoke( "abe" ) ;
  evt.invoke( "bob" ) ;
  evt.invoke( "sue" ) ;
  evt.invoke( "ron" ) ;

} // :: test_string_events

//-----------------------------------------------------------------------------
//
//

int main()
{
  test_string_events() ;

  test_fake_window_events() ;

  return 0 ;
} // :: main

