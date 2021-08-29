/* Used in Boost.MultiIndex tests.
 *
 * Copyright 2003-2008 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_TEST_PRE_MULTI_INDEX_HPP
#define BOOST_MULTI_INDEX_TEST_PRE_MULTI_INDEX_HPP

#include <boost/multi_index/safe_mode_errors.hpp>

#define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING

#define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE

struct safe_mode_exception {
  safe_mode_exception(boost::multi_index::safe_mode::error_code error_code_):
    error_code(error_code_)
  {}

  boost::multi_index::safe_mode::error_code error_code;
};

#define BOOST_MULTI_INDEX_SAFE_MODE_ASSERT(expr,error_code) \
if(!(expr)){throw safe_mode_exception(error_code);}

#endif
