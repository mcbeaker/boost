#ifndef DATE_TIME_TIME_RESOLUTION_TRAITS_HPP
#define DATE_TIME_TIME_RESOLUTION_TRAITS_HPP

/* Copyright (c) 2002 CrystalClear Software, Inc.
 * Disclaimer & Full Copyright at end of file
 * Author: Jeff Garland, Bart Garst
 */


#include "boost/date_time/time_defs.hpp"
#include "boost/cstdint.hpp"

namespace boost {
namespace date_time {

  //! Simple function to calculate absolute value of a numeric type
  template <typename T> 
  // JDG [7/6/02 made a template], 
  // moved here from time_duration.hpp 2003-Sept-4.
  inline T absolute_value(T x) 
  {
    return x < 0 ? -x : x;
  }

  template<typename frac_sec_type, 
           time_resolutions res, 
           frac_sec_type resolution_adjust,
           unsigned short frac_digits,
           typename v_type = boost::int32_t >
  class time_resolution_traits {
  public:
    typedef frac_sec_type fractional_seconds_type;
    typedef frac_sec_type tick_type;
    typedef v_type  day_type;
    typedef v_type  hour_type;
    typedef v_type  min_type;
    typedef v_type  sec_type;

    //Would like this to be frac_sec_type, but some compilers complain
    BOOST_STATIC_CONSTANT(int, ticks_per_second = resolution_adjust);
    //    static const boost::int32_t ticks_per_second = resolution_adjust;

    static time_resolutions resolution()
    {
      return res;
    }
    static unsigned short num_fractional_digits()
    {
      return frac_digits;
    }
    static fractional_seconds_type res_adjust()
    {
      return resolution_adjust;
    }
    //! Any negative argument results in a negative tick_count
    static tick_type to_tick_count(hour_type hours,
                                   min_type  minutes,
                                   sec_type  seconds,
                                   fractional_seconds_type  fs)
    {
      if(hours < 0 || minutes < 0 || seconds < 0 || fs < 0)
      {
	hours = absolute_value(hours);
	minutes = absolute_value(minutes);
	seconds = absolute_value(seconds);
	fs = absolute_value(fs);
        return (-1 *(((fractional_seconds_type(hours)*3600) 
                 + (fractional_seconds_type(minutes)*60) 
                 + seconds)*res_adjust()) + fs);
      }
      else{
        return (((fractional_seconds_type(hours)*3600) 
                 + (fractional_seconds_type(minutes)*60) 
                 + seconds)*res_adjust()) + fs;
      }
    }
    
  };

  typedef time_resolution_traits<boost::int32_t, milli, 1000, 3 > milli_res;
  typedef time_resolution_traits<boost::int64_t, micro, 1000000, 6 > micro_res;
  typedef time_resolution_traits<boost::int64_t, nano,  1000000000, 9 > nano_res;


} } //namespace date_time

/* Copyright (c) 2002
 * CrystalClear Software, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  CrystalClear Software makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */


#endif
