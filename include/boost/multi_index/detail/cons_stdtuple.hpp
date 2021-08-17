/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_CONS_STDTUPLE_HPP
#define BOOST_MULTI_INDEX_DETAIL_CONS_STDTUPLE_HPP
#pragma once

#include <boost/multi_index/cons_tuple.hpp>
#include <tuple>

namespace boost::multi_index::detail {

/* std::tuple wrapper providing the cons-based interface of cons_tuple for
 * composite_key interoperability.
 */

template<typename StdTuple, std::size_t I=0>
struct cons_stdtuple;

struct cons_stdtuple_ctor_terminal {
  using result_type = cons_null;

  template<typename StdTuple>
  static result_type create(const StdTuple&) { return cons_null(); }
}; // cons_stdtuple_ctor_terminal

template<typename StdTuple, std::size_t I>
struct cons_stdtuple_ctor_normal {
  using result_type = cons_stdtuple<StdTuple, I>;

  static result_type create(const StdTuple& t) { return result_type(t); }
}; // cons_stdtuple_ctor_normal

template<typename StdTuple, std::size_t I=0>
struct cons_stdtuple_ctor
  : std::conditional_t<
      (I < std::tuple_size_v<StdTuple>),
      cons_stdtuple_ctor_normal<StdTuple, I>,
      cons_stdtuple_ctor_terminal
    >
{ }; // cons_stdtuple_ctor

template<typename StdTuple, std::size_t I>
struct cons_stdtuple {
  using head_type = typename std::tuple_element_t<I, StdTuple>;
  using tail_ctor = cons_stdtuple_ctor<StdTuple, I+1>;
  using tail_type = typename tail_ctor::result_type;
  
  cons_stdtuple(const StdTuple& t_) : t(t_) {}

  const head_type& get_head() const { return std::get<I>(t); }
  tail_type get_tail() const { return tail_ctor::create(t); }
    
  const StdTuple& t;
}; // cons_stdtuple

template<typename StdTuple>
auto make_cons_stdtuple(const StdTuple& t) {
  return cons_stdtuple_ctor<StdTuple>::create(t);
}

} // boost::multi_index::detail

namespace std {

template<typename... Types>
struct tuple_size<boost::multi_index::detail::cons_stdtuple<
    std::tuple<Types...>>>
  : tuple_size<std::tuple<Types...>>
{ };

template<std::size_t I, typename... Types>
struct tuple_element<I, boost::multi_index::detail::cons_stdtuple<
    std::tuple<Types...>>>
  : tuple_element<I, std::tuple<Types...>>
{ };

template<std::size_t I, typename... Types>
auto get(
    const boost::multi_index::detail::cons_stdtuple<std::tuple<Types...>>& t)
{ return get<I>(t.t); }

template<std::size_t I, typename... Types>
auto get(boost::multi_index::detail::cons_stdtuple<std::tuple<Types...>>& t)
{ return get<I>(t.t); }

} // std

#endif
