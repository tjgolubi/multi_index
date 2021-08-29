/* Boost.MultiIndex test for Mp11 operations.
 *
 * Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#include "test_mpl_ops.hpp"
#include "pre_multi_index.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

#include <type_traits>

using namespace boost::multi_index;

void test_mpl_ops()
{
  using indexed_t1 = multi_index_container <
                     int,
                     indexed_by <
                     ordered_unique<identity<int>>,
                     ordered_non_unique<identity<int>>
                     >
                     >;

  static_assert((std::is_same_v <
                 boost::mp11::mp_at_c<indexed_t1::index_specifier_type_list, 0>,
                 ordered_unique<identity<int>> >));
  static_assert((std::is_same_v <
                 boost::mp11::mp_at_c<indexed_t1::index_specifier_type_list, 1>,
                 ordered_non_unique<identity<int>> >));

  typedef boost::mp11::mp_push_front <
  indexed_t1::index_specifier_type_list,
             sequenced<>
             >                           index_list_t;

  typedef multi_index_container <
  int,
  index_list_t
  >                           indexed_t2;

  static_assert((std::is_same_v <
                 boost::mp11::mp_at_c<indexed_t2::index_specifier_type_list, 0>,
                 sequenced<> >));
  static_assert((std::is_same_v <
                 boost::mp11::mp_at_c<indexed_t2::index_specifier_type_list, 1>,
                 boost::mp11::mp_at_c<indexed_t1::index_specifier_type_list, 0> >));
  static_assert((std::is_same_v <
                 boost::mp11::mp_at_c<indexed_t2::index_specifier_type_list, 2>,
                 boost::mp11::mp_at_c<indexed_t1::index_specifier_type_list, 1> >));

  typedef multi_index_container <
  int,
  boost::mp11::mp_list <
  ordered_unique<identity<int>>,
  ordered_non_unique<identity<int>>
  >
  >                           indexed_t3;

  static_assert((std::is_same_v <
                 boost::mp11::mp_at_c<indexed_t3::index_specifier_type_list, 0>,
                 boost::mp11::mp_at_c<indexed_t1::index_specifier_type_list, 0> >));
  static_assert((std::is_same_v <
                 boost::mp11::mp_at_c<indexed_t3::index_specifier_type_list, 1>,
                 boost::mp11::mp_at_c<indexed_t1::index_specifier_type_list, 1> >));
}
