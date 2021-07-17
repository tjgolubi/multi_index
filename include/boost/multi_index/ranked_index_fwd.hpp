/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_RANKED_INDEX_FWD_HPP
#define BOOST_MULTI_INDEX_RANKED_INDEX_FWD_HPP
#pragma once

#include <boost/multi_index/detail/ord_index_impl_fwd.hpp>
#include <boost/multi_index/detail/ord_index_args.hpp>

namespace boost{

namespace multi_index{

/* ranked_index specifiers */

template<typename Arg1,typename Arg2=void,typename Arg3=void>
struct ranked_unique;

template<typename Arg1,typename Arg2=void,typename Arg3=void>
struct ranked_non_unique;

} /* namespace multi_index */

} /* namespace boost */

#endif
