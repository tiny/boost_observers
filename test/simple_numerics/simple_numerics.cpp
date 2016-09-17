/*
  @file       simple_numerics.cpp
  @brief      main file for simple-numneric observer test app 

  @author     Robert McInnis
  @date       september 10, 2016
  @par        copyright (c) 2016 Solid ICE Technologies, Inc.  All rights reserved.

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#include <stdio.h>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <set>
#include "boost/observers/numerics.h"
#include <boost/timer/timer.hpp>

using namespace boost ;

//------------------------------------------------------------------------
// for performance testing
//
#define  N_ITERS                  5
#define  MAX_ITERS                50000
#define  MIN_ITERS                35000
#define  rand_between(a,b)        (uint32_t)(((double)((double)rand() / (double)RAND_MAX) * (double)(b - a)) + (double)a)

double     __fastrands[100] ;
uint32_t   __frand_ndx = 0 ;
uint32_t   cycle = 0  ;
uint32_t   n_invokes = 0 ;

uint32_t fastrand_between( uint32_t a, uint32_t b)
{
  return (uint32_t)((double)(__fastrands[(__frand_ndx++ % 100)] * (double)(b - a)) + (double)a) ;
} // :: fastrand_between

// using globals for test.  obviously not recommended for any real application
//
// rules:
//   x = literal
//   y = x + 3 
//   z = y * 2 
//
boost::observers::Numeric< long >    x ;
boost::observers::Numeric< long >    y ;
boost::observers::Numeric< long >    z ;

void hook_business_rules()
{
  // push observer action onto the numeric
  //
  x << new observers::LambdaPoke( [](){ y = x + (long) 3 ; n_invokes++ ; } ) ;
  y << new observers::LambdaPoke( [](){ z = y * (long) 2 ; n_invokes++ ; } ) ;

#ifdef INCLUDE_VALIDATION_TEST
  z << new observers::Lambda( [](){ 
    double a = x ;
    double b = y ;
    double c = z ;
    double valz = (a + 3.0) * 2.0 ;

    if ((double)z != valz) 
    { 
      printf("FAIL.  a: %.lf  b: %.lf  c: %.lf  valz: %.lf  z: %.lf\n", a, b, c, valz, (double)z ) ; 
      fflush(stdout) ;
    } 
  }) ;
#endif
} // :: hook_business_rules

void print_rules()
{
  printf( " rules:  \n" ) ;
  printf( "    x = literal \n" ) ;
  printf( "    y = x + 3  \n" ) ;
  printf( "    z = y * 2  \n" ) ;
  printf( "\n" ) ;
} // :: print_rules

void print_values()
{
  printf( "x: %3ld  y: %3ld  z: %3ld  \n", (long)x, (long)y, (long)z ) ;
} // :: print_values

//-----------------------------------------------------------------------------
//  simple test
//
void test_numerics_simple()
{
  printf( "--[  simple test  ]--\n\n" ) ;
  print_rules() ;

  printf( "initial      " ) ;
  print_values() ;

  printf( "set x =  5   " ) ;
  x = 5 ;
  print_values() ;

  printf( "set x = 15   " ) ;
  x = 15 ;
  print_values() ;

  printf( "set x =  1   " ) ;
  x = 1 ;
  print_values() ;

  printf( "\n" ) ;
} // :: test_numerics_simple

//-----------------------------------------------------------------------------
//  speed test
//
void test_numerics_body( bool single_threaded = true )
{
  uint32_t a = n_invokes ;
  boost::posix_time::time_duration  dt ;
  boost::posix_time::ptime          tm1(boost::posix_time::microsec_clock::local_time());

  if (single_threaded)
  {
    // main thread
    uint32_t n = rand_between( MIN_ITERS, MAX_ITERS ) ;
    long     dx = 1 ;

    for (uint32_t i = 0; i < n; i++)
      x += dx ;
  }
  else
  {
    uint32_t n = rand_between( MIN_ITERS, MAX_ITERS ) ;
    long     dx = 1 ;

    boost::thread t1 { [dx, n]() {
      for (auto i = 0; i < n; i++)
        x += dx ;
    }};

    n = rand_between( MIN_ITERS, MAX_ITERS ) ;
    dx = 2 ;
    boost::thread t2{ [dx, n]() {
      for (auto i = 0; i < n; i++)
          x += dx ;
    }};

    t1.join() ;
    t2.join() ;
  }

  boost::posix_time::ptime          tm2(boost::posix_time::microsec_clock::local_time());
  uint32_t b = n_invokes ;

  dt = tm2 - tm1;
 
  //number of elapsed miliseconds
  long msec = dt.total_milliseconds();
  long n_calcs = b - a ;
  printf( "%2ld.  dt:  %ld ms   #calcs:  %6ld   invokes/sec:  %7.0lf\n", 
          ++cycle, msec, n_calcs, floor( ((double)n_calcs / (double)msec)*1000.0 ) ) ;

  ::fflush( stdout ) ;
} // :: test_numerics_body

void test_numerics_performance()
{
  srand( ::timeGetTime() % 1000 ) ; // initialize randomness
  printf( "--[  speed test : single thread ]--\n\n" ) ;
  cycle = 0 ;

  for (int i = 0; i < N_ITERS; i++)
  {
    test_numerics_body( true ) ;
    Sleep(500) ;
  }

  printf( "\n" ) ;
  printf( "--[  speed test : multi thread  ]--\n\n" ) ;
  cycle = 0 ;

  for (int i = 0; i < N_ITERS; i++)
  {
    test_numerics_body( false ) ;
    Sleep(500) ;
  }

  printf( "\n--------------------\n\n" ) ;
} // :: test_numerics_performance

//-----------------------------------------------------------------------------
//
//
int main()
{
  hook_business_rules() ;

  test_numerics_simple() ;

  test_numerics_performance() ;
  return 0 ;
} // :: main
