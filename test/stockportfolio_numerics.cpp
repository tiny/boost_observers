/*!
  @file       main_test.cpp
  @brief      main test file for observers

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
#include "boost/observers/numerics.hpp"
#include <boost/timer/timer.hpp>

#define  N_ITERS                  10
#define  MAX_ITERS                50000
#define  MIN_ITERS                35000
#define  rand_between(a,b)        (uint32_t)(((double)((double)rand() / (double)RAND_MAX) * (double)(b - a)) + (double)a)

double     __fastrands[100] ;
uint32_t   __frand_ndx = 0 ;
uint32_t   n_calcs = 0 ;
uint32_t   cycle = 0  ;

uint32_t fastrand_between( uint32_t a, uint32_t b)
{
  return (uint32_t)((double)(__fastrands[(__frand_ndx++ % 100)] * (double)(b - a)) + (double)a) ;
} // :: fastrand_between

// initialize fake rands to remove rand() from speed test
//
void initialize_fake_rands()
{
  srand( time(NULL) ) ;
  for (int i = 0; i < 100; i++)
    __fastrands[i] = (double)rand() / (double)RAND_MAX ;
} // :: initialize_fake_rands

//-----------------------------------------------------------------------------
//
//  stock portfolio management test
//

//  quick and dirty market index
//  --
//  array index is the stock-id 
//  prices is the current market value of stock-id 
//  syms is the symbol for that stock-id
//
std::string   syms  [20] ;
double        prices[20] ;
std::map<std::string, double> market = { { "MSFT",  57.67 }, 
                                         { "AAPL", 107.71 }, 
                                         { "FB"  , 130.80 },
                                         { "TWTR",  20.09 },
                                         { "GOOG", 779.94 }, 
                                         { "T"   ,  41.07 },
                                         { "AMZN", 785.59 },
                                         { "TSLA", 201.64 },
                                         { "NFLX",  99.10 },
                                         { "AMD" ,   6.83 },
                                         { "BAC" ,  15.74 },
                                         { "WDC" ,  53.29 },
                                         { "WFM" ,  29.20 },
                                         { "DAL" ,  39.06 },
                                         { "F"   ,  12.69 }, 
                                         { "GE"  ,  30.91 }  
                                       };

//-----------------------------------------------------------------------------
//
class Stock
{
  public  :
    uint32_t                              id ;
    boost::observers::Numeric< double >   price ;
    boost::observers::Numeric< uint32_t > volume ;

                                          Stock( uint32_t id_, double spot = 0.0 ) 
                                          { id     = id_ ;
                                            price  = spot ; 
                                            volume = 0 ;
                                          }

    const std::string                    &name() { return syms[id] ; }
} ; // class Stock

class StockTick
{
  public :
    uint32_t       id ;
    double         price ;
    uint32_t       qty ;

                   StockTick( const StockTick &c ) { *this = c ; }
                   StockTick( uint32_t id_, double p, uint32_t q ) 
                   : price(p)
                   , qty(q) 
                   { id = id_ ; }

    StockTick     &operator= ( const StockTick &c )
                   {
                     if (&c != this)
                     {
                       id    = c.id ;
                       price = c.price ;
                       qty   = c.qty ;
                     }
                     return *this ;
                   }
    void           report() const
                   { printf( "  %-6.6s  %4ld  @  %6.2lf \n", syms[id].c_str(), qty, price ) ;
                   }
} ; // class StockTick

#include <map>
typedef std::map< uint32_t, Stock* >               StockMap ;
typedef std::map< uint32_t, Stock* >::iterator     StockMap_iter ;
typedef std::map< uint32_t, Stock* >::value_type   StockMap_pair ;

class StockFeed
{
  public  :
    StockMap        stocks ;

                    StockFeed() {}
                   ~StockFeed() { clear() ; }

    void            clear()
                    { for (auto e : stocks)
                        delete e.second ;
                      stocks.clear() ;
                    }
    Stock          &get( uint32_t stock_id ) 
                    {
                      StockMap_iter  it = stocks.find( stock_id ) ;
                      if (it == stocks.end())
                      {
                        Stock  *s = new Stock( stock_id ) ;
                        stocks.insert( StockMap_pair( stock_id, s )) ;
                        return *s ;
                      }
                      return *(*it).second ;
                    }
    const StockTick generate_random_tick()
                    {
                      uint32_t ndx  = fastrand_between( 0, stocks.size() ) ;              // random stock-id
                      uint32_t qty  = fastrand_between( 1, 9 ) * 100 ;                    // random qty; 100-900 shrs
                      double   p    = prices[ndx] * (fastrand_between( 1, 3 ) / 1000.0) ; // price adjustment; 0.1 - 0.3%
                      double   nu_p = prices[ndx] + p * ((fastrand_between(1,10) > 5) ? 1.0 : -1.0) ;

                      return StockTick( ndx, nu_p, qty ) ;
                    }
    void            report()
                    {
                      printf( "the current market: \n" ) ;
                      printf( "------------------- \n" ) ;
                      for (auto e : stocks)
                        printf( "  %-6.6s  @  %6.2lf \n", syms[ e.first ].c_str(), (double)e.second->price ) ;
                      printf( " \n" ) ;
                    }
    virtual void    tick( const StockTick &tick ) 
                    {
                      StockMap_iter  it = stocks.find( tick.id ) ;
                      if (it == stocks.end())
                      {
                        stocks.insert( StockMap_pair( tick.id, new Stock( tick.id, tick.price ))) ;
                      }
                      else
                      {
                        (*it).second->price = tick.price ;
                      }
                      prices[ tick.id ] = tick.price ;
//                      tick.report() ;
                    }
} ; // class StockFeed

class EquityPosition
{
  public :
    uint32_t       stock_id ;
    uint32_t       buy_ts ;
    uint32_t       buy_qty ;
    double         buy_price ;
    double         buy_fee ;
    uint32_t       sell_ts ;
    uint32_t       sell_qty ;
    double         sell_price ;
    double         sell_fee ;

                   EquityPosition( uint32_t stock_id_, 
                                   uint32_t buy_ts_, uint32_t buy_q, double buy_p, double buy_f,
                                   uint32_t sell_ts_ = 0, uint32_t sell_q = 0, double sell_p = 0.0, double sell_f = 0.0
                                 ) 
                   {
                     buy_ts      = buy_ts_ ; 
                     buy_qty     = buy_q ;
                     buy_price   = buy_p ;
                     buy_fee     = buy_f ;
                     sell_ts     = sell_ts_ ; 
                     sell_qty    = sell_q ;
                     sell_price  = sell_p ;
                     sell_fee    = sell_f ;
                   }

    bool           closed() { return (buy_qty == sell_qty) ; }
    uint32_t       qoh() { return (buy_qty - sell_qty) ; }  // quantity on hand
} ; // class EquityPosition

typedef std::vector< EquityPosition* >                EquityVec ;
typedef std::vector< EquityPosition* >::iterator      EquityVec_iter ;

class PositionEntry
{
  public  :
    uint32_t                                stock_id ;
    EquityVec                               sheets ; // list of equity sales slips
    boost::observers::Numeric< double >     value ;
    double                                  spot ; // last strike price
    uint32_t                                qoh ;

    virtual void        on_price_update( const std::vector<boost::any> &args )
                        {
                          double price = boost::any_cast<double>( args[0] ) ;

                          value = ((double)qoh) * price ;
                          spot  = price ;
                        }

                        PositionEntry( Stock &stock ) 
                        : stock_id( stock.id )
                        { value = 0.0 ;
                          qoh   = 0 ;
                          spot  = stock.price ;
                          stock.price << new boost::observers::MemberFunc<PositionEntry>( this, &PositionEntry::on_price_update ) ;
                        } 

    virtual void        new_position( EquityPosition *p )
                        {
                          if (p == NULL)  return ;
                          sheets.push_back( p ) ;
                          qoh  += p->qoh() ;
                          value = qoh * spot ;
                        }
    void                report()
                        { printf( "  %-6.6s  %5ld  @  %6.2lf  %10.2lf \n", syms[ stock_id ].c_str(), qoh, spot, (double)value ) ;
                        }
} ; // class PositionEntry

typedef std::map< uint32_t, PositionEntry* >                PositionEntryMap ;
typedef std::map< uint32_t, PositionEntry* >::iterator      PositionEntryMap_iter ;
typedef std::map< uint32_t, PositionEntry* >::value_type    PositionEntryMap_pair ;

class EquityLedger
{
  public  :
    uint32_t                             stock_id ;         // stock symbol
    PositionEntryMap                     positions ;
    boost::observers::Numeric< double >  value ;

    virtual void        on_value_change( const std::vector<boost::any> &args )
                        {
                          double new_price = boost::any_cast<double>( args[0] ) ;
                          double old_price = boost::any_cast<double>( args[1] ) ;

                          value += (new_price - old_price) ;
                        }

                        EquityLedger( uint32_t stock_id_ ) 
                        : stock_id( stock_id_ )
                        {
                          value = 0.0 ;
                        }

    PositionEntry      &get( Stock &stock ) 
                        {
                          PositionEntryMap_iter  it = positions.find( stock.id ) ;
                          if (it == positions.end())
                          {
                            PositionEntry  *e = new PositionEntry( stock ) ;
                            positions.insert( PositionEntryMap_pair( stock.id, e )) ;
                            e->value << new boost::observers::MemberFunc<EquityLedger>( this, &EquityLedger::on_value_change ) ;
                            return *e ;
                          }
                          return *(*it).second ;
                        }
    int16_t             buy ( Stock &stock, uint32_t qty, double price, bool at_market ) 
                        { double    buy_fee = 0.00 ;
                          uint32_t  buy_ts = ::time(NULL) ;
                          double    cost = qty * price + buy_fee ;
                          get( stock ).new_position( new EquityPosition( stock.id, buy_ts, qty, price, buy_fee )) ;
                          return 0 ;
                        }
    int16_t             sell( Stock &stock, uint32_t qty, double price, bool at_market ) 
                        { double    sell_fee = 0.00 ;
                          uint32_t  sell_ts = ::time(NULL) ;
                          double    cost = qty * price + sell_fee ;
// not implemented yet
//                          get( stock ).new_position( new EquityPosition( stock.name, buy_ts, qty, price, buy_fee ) ;
                          return 0 ;
                        }
    void                report()
                        { for (auto e : positions)
                          {
                            e.second->report() ;
                          }
                        }
} ; // class EquityLedger

typedef std::map< uint32_t, EquityLedger* >                EquityLedgerMap ;
typedef std::map< uint32_t, EquityLedger* >::iterator      EquityLedgerMap_iter ;
typedef std::map< uint32_t, EquityLedger* >::value_type    EquityLedgerMap_pair ;

class Portfolio
{
  public  :
    uint32_t            customer_id ;
    EquityLedgerMap     ledgers ;
    boost::observers::Numeric< double >  balance ;

    virtual void        on_value_change( const std::vector<boost::any> &args )
                        {
                          double new_price = boost::any_cast<double>( args[0] ) ;
                          double old_price = boost::any_cast<double>( args[1] ) ;

                          balance += (new_price - old_price) ;
                        }

                        Portfolio( uint32_t  cid ) 
                        { 
                          customer_id = cid ; 
                          balance     = 0.00 ;
                        }

    EquityLedger       &get( uint32_t stock_id ) 
                        {
                          EquityLedgerMap_iter  it = ledgers.find( stock_id ) ;
                          if (it == ledgers.end())
                          {
                            EquityLedger  *e = new EquityLedger( stock_id ) ;
                            ledgers.insert( EquityLedgerMap_pair( stock_id, e )) ;
                            e->value << new boost::observers::MemberFunc<Portfolio>( this, &Portfolio::on_value_change ) ;
                            return *e ;
                          }
                          return *(*it).second ;
                        }
    int16_t             buy ( Stock &stock, uint32_t qty, double price, bool at_market ) 
                        { return get( stock.id ).buy ( stock, qty, price, at_market ) ;
                        }
    int16_t             sell( Stock &stock, uint32_t qty, double price, bool at_market ) 
                        { return get( stock.id ).sell( stock, qty, price, at_market ) ;
                        }
    void                report()
                        { printf( "id: %2ld  ------------------------------\n", customer_id ) ;
                          for (auto e : ledgers)
                          {
                            e.second->report() ;
                          }
                          printf( "------------------ balance: %10.2lf \n", (double)balance ) ;
                        }
} ; // class Portfolio

typedef std::map< uint32_t, Portfolio* >                PortfolioMap ;
typedef std::map< uint32_t, Portfolio* >::iterator      PortfolioMap_iter ;
typedef std::map< uint32_t, Portfolio* >::value_type    PortfolioMap_pair ;

class PortfolioMgr
{
  private :
    PortfolioMap        portfolios ;
    StockFeed          &_feed ;

  public  :
                        PortfolioMgr( StockFeed &f ) : _feed( f ) {}

    Portfolio          &get( uint32_t customer_id ) 
                        {
                          PortfolioMap_iter  it = portfolios.find( customer_id ) ;
                          if (it == portfolios.end())
                          {
                            Portfolio  *p = new Portfolio( customer_id ) ;
                            portfolios.insert( PortfolioMap_pair( customer_id, p )) ;
                            return *p ;
                          }
                          return *(*it).second ;
                        }

    // actions
    int16_t             buy ( uint32_t customer_id, uint32_t stock_id, uint32_t qty, double price, bool at_market = false ) 
                        { get( customer_id ).buy ( _feed.get( stock_id ), qty, price, at_market ) ;
                          return 0 ;
                        }
    int16_t             sell( uint32_t customer_id, uint32_t stock_id, uint32_t qty, double price, bool at_market = false ) 
                        { get( customer_id ).sell( _feed.get( stock_id ), qty, price, at_market ) ;
                          return 0 ;
                        }
    void                report()
                        { for (auto e : portfolios)
                          {
                            e.second->report() ;
                          }
                          printf( "\n" ) ;
                        }
} ; // class PortfolioMgr

//-----------------------------------------------------------------------------
//
// generate initial market condition
//
#define  MAX_CUSTOMERS  1000
#define  MAX_TICKS      2000

void generate_initial_market( StockFeed &feed )
{
  uint32_t  stock_id = 0 ;
  for (auto e : market)
  {
    feed.tick( StockTick( stock_id, e.second, 1 )) ;
    syms[ stock_id ]   = e.first ;
    prices[ stock_id ] = e.second ;
    stock_id++ ;
  }
} // :: generate_initial_market

void generate_random_portfolios( PortfolioMgr &mgr )
{
  // generate random portfolios
  // ie:  mgr.buy( 1, "MSFT", 100,  57.67 ) ;  
  // 
  uint32_t   market_sz = market.size() ;
  for (uint32_t cust_id = 1; cust_id < MAX_CUSTOMERS; cust_id++)
  {
    uint32_t   n = rand_between( 5, market_sz ) ;
    for (uint32_t i = 0; i < n; i++)
    {
      uint32_t   stock_id = rand_between( 0, market_sz ) ;
      uint32_t   qty      = rand_between( 1, 9 ) * 100 ;

      mgr.buy( cust_id, stock_id, qty, prices[stock_id] ) ;
    }
  }
} // :: generate_random_portfolios

void test_portfolio( StockFeed &feed, PortfolioMgr &mgr )
{
  // status info
  feed.report() ;
  printf( "portfolio for customer-id 1 (initial) \n" ) ;
  mgr.get( 1 ).report() ;

  StockTick tick = feed.generate_random_tick() ;
  printf( "\npushing one tick:  " ) ;
  tick.report() ;
  feed.tick( tick ) ;
  printf( "\nportfolio of customer-1 after tick\n" ) ;
  mgr.get( 1 ).report() ;

  // simulation stopwatch
  boost::posix_time::time_duration  dt ;
  boost::posix_time::ptime          tm1(boost::posix_time::microsec_clock::local_time());

  // simulated feed 
  //
  for (uint32_t i = 0; i < MAX_TICKS; i++)
  {
    feed.tick( feed.generate_random_tick() ) ;
  }

  boost::posix_time::ptime          tm2(boost::posix_time::microsec_clock::local_time());
  dt = tm2 - tm1;
  long msec = dt.total_milliseconds();

  printf( "\n" ) ;
  printf( "# portfolios:   %ld  \n", MAX_CUSTOMERS ) ;
  printf( "# ticks:        %ld  \n", MAX_TICKS ) ;
  printf( "time(ms):       %ld ms \n", msec ) ;
  printf( "max ticks/sec   %8.0lf \n", floor( ((double)MAX_TICKS / (double)msec)*1000.0 )) ;

  printf( "\n" ) ;
  printf( "portfolio for customer-id 1 after updates\n" ) ;
  mgr.get( 1 ).report() ;

} // :: test_portfolio

//-----------------------------------------------------------------------------
//
//

int main()
{
  StockFeed             feed ;
  PortfolioMgr          mgr( feed ) ;

  initialize_fake_rands() ;
  generate_initial_market( feed ) ;
  generate_random_portfolios( mgr ) ;
  test_portfolio( feed, mgr ) ;

  return 0 ;
} // :: main

