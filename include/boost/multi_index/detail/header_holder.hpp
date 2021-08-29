/* Copyright 2003-2008 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_HEADER_HOLDER_HPP
#define BOOST_MULTI_INDEX_DETAIL_HEADER_HOLDER_HPP
#pragma once

#include <boost/multi_index/detail/noncopyable.hpp>

namespace boost::multi_index::detail {

/* A utility class used to hold a pointer to the header node.
 * The base from member idiom is used because index classes, which are
 * superclasses of multi_index_container, need this header in construction
 * time. The allocation is made by the allocator of the multi_index_container
 * class --hence, this allocator needs also be stored resorting
 * to the base from member trick.
 */

template<typename NodeTypePtr, typename Final>
struct header_holder: private noncopyable {
  header_holder(): member(final().allocate_node()) {}
  ~header_holder()
  {
    final().deallocate_node(&*member);
  }

  NodeTypePtr member;

private:
  Final& final()
  {
    return *static_cast<Final*>(this);
  }
};

} // boost::multi_index::detail

#endif
