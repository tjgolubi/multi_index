/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_COMPOSITE_KEY_HPP
#define BOOST_MULTI_INDEX_COMPOSITE_KEY_HPP
#pragma once

#include <boost/multi_index/detail/cons_stdtuple.hpp>
#include <boost/multi_index/cons_tuple.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/function.hpp>
#include <boost/mp11/utility.hpp>
#include <tuple>
#include <functional>
#include <type_traits>

/* A composite key stores n key extractors and "computes" the
 * result on a given value as a packed reference to the value and
 * the composite key itself. Actual invocations to the component
 * key extractors are lazily performed when executing an operation
 * on composite_key results (equality, comparison, hashing.)
 * As the other key extractors in Boost.MultiIndex, composite_key<T,...>
 * is  overloaded to work on chained pointers to T and reference_wrappers
 * of T.
 */

namespace boost::multi_index {

namespace detail {

/* n-th key extractor of a composite key */

template<typename CompositeKey, std::size_t N>
struct nth_key_from_value {
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using type = typename mp11::mp_if_c <
    (N < cons_size<key_extractor_tuple>::value),
    cons_element<N, key_extractor_tuple>,
    mp11::mp_identity<cons_null>
  >::type;
}; // nth_key_from_value

template<typename KeyFromValue>
struct key_equal_to {
  using type = std::equal_to<typename KeyFromValue::result_type>;
};

template<> struct key_equal_to<cons_null> {
  using type = cons_null;
};

template<typename CompositeKey, std::size_t N>
struct nth_composite_key_equal_to {
  using key_from_value = typename nth_key_from_value<CompositeKey, N>::type;
  using type = typename key_equal_to<key_from_value>::type;
};

template<typename KeyFromValue>
using result_type_of = typename KeyFromValue::result_type;

template<typename T>
using tjg_equal_to = mp11::mp_identity_t<std::equal_to<T>>;

template<typename CompositeKeyResult>
struct key_equal_to_list_helper {
  using composite_key_type = typename CompositeKeyResult::composite_key_type;
  using key_extractor_tuple = typename composite_key_type::key_extractor_tuple;
  using key_extractor_list =
                            mp11::mp_rename<key_extractor_tuple, mp11::mp_list>;
  using key_result_type = mp11::mp_transform<
      result_type_of,
      mp11::mp_remove<key_extractor_list, cons_null>
  >;
  using type = mp11::mp_transform<tjg_equal_to, key_result_type>;
}; // key_equal_to_list_helper

template<typename CompositeKeyResult>
using key_equal_to_list =
                    typename key_equal_to_list_helper<CompositeKeyResult>::type;

template<typename KeyFromValue>
struct key_less {
  using type = std::less<typename KeyFromValue::result_type>;
};

template<> struct key_less<cons_null> {
  using type = cons_null;
};

template<typename CompositeKey, std::size_t N> struct nth_composite_key_less {
  using key_from_value = typename nth_key_from_value<CompositeKey, N>::type;
  using type = typename key_less<key_from_value>::type;
};

template<typename KeyFromValue>
struct key_greater {
  using type = std::greater<typename KeyFromValue::result_type>;
};

template<> struct key_greater<cons_null> {
  using type = cons_null;
};

template<typename CompositeKey, std::size_t N>
struct nth_composite_key_greater {
  using key_from_value = typename nth_key_from_value<CompositeKey, N>::type;
  using type = typename key_greater<key_from_value>::type;
};

template<typename KeyFromValue>
struct key_hash {
  using type = std::hash<typename KeyFromValue::result_type>;
};

template<> struct key_hash<cons_null> {
  using type = cons_null;
};

template<typename CompositeKey, std::size_t N>
struct nth_composite_key_hash {
  using key_from_value = typename nth_key_from_value<CompositeKey, N>::type;
  using type = typename key_hash<key_from_value>::type;
};
/* used for defining equality and comparison ops of composite_key_result */

struct generic_operator_equal {
  template<typename T, typename Q>
  bool operator()(const T& x, const Q& y) const
  { return x == y; }
};

template<typename T>
using generic_operator_equal_tuple =
  mp11::mp_fill<mp11::mp_rename<T, cons_tuple>, generic_operator_equal>;

struct generic_operator_less {
  template<typename T, typename Q>
  bool operator()(const T& x, const Q& y) const
  { return x < y; }
};

template<typename T>
using generic_operator_less_tuple =
    mp11::mp_fill<mp11::mp_rename<T, cons_tuple>, generic_operator_less>;

/* Metaprogramming machinery for implementing equality, comparison and
 * hashing operations of composite_key_result.
 *
 * equal_* checks for equality between composite_key_results and
 * between those and tuples, accepting a cons_tuple of basic equality functors.
 * compare_* does lexicographical comparison.
 * hash_* computes a combination of elementwise hash values.
 */

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename EqualCons>
struct equal_ckey_ckey; /* fwd decl. */

template < typename KeyCons1, typename Value1,
           typename KeyCons2, typename Value2,
           typename EqualCons>
struct equal_ckey_ckey_terminal {
  static bool compare(const KeyCons1&, const Value1&,
                      const KeyCons2&, const Value2&,
                      const EqualCons&)
 { return true; }
}; // equal_ckey_ckey_terminal

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename EqualCons>
struct equal_ckey_ckey_normal {
  static bool compare(const KeyCons1& c0, const Value1& v0,
                      const KeyCons2& c1, const Value2& v1,
                      const EqualCons& eq)
  {
    if (!eq.get_head()(c0.get_head()(v0), c1.get_head()(v1)))
      return false;
    return equal_ckey_ckey <
              typename KeyCons1::tail_type, Value1,
              typename KeyCons2::tail_type, Value2,
              typename EqualCons::tail_type
           >::compare(c0.get_tail(), v0, c1.get_tail(), v1, eq.get_tail());
  }
}; // equal_ckey_ckey_normal

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename EqualCons>
struct equal_ckey_ckey
  : mp11::mp_if <
      mp11::mp_or <
        std::is_same<KeyCons1, cons_null>,
        std::is_same<KeyCons2, cons_null>
      >,
      equal_ckey_ckey_terminal<KeyCons1, Value1, KeyCons2, Value2, EqualCons>,
      equal_ckey_ckey_normal  <KeyCons1, Value1, KeyCons2, Value2, EqualCons>
    >
{ };

template <typename KeyCons, typename Value,
          typename ValCons, typename EqualCons>
struct equal_ckey_cval; /* fwd decl. */

template <
  typename KeyCons, typename Value,
  typename ValCons, typename EqualCons
>
struct equal_ckey_cval_terminal {
  static bool compare(const KeyCons&, const Value&, const ValCons&,
                      const EqualCons&)
  { return true; }

  static bool compare(const ValCons&, const KeyCons&, const Value&,
                      const EqualCons&)
  { return true; }
}; // equal_ckey_cval_terminal

template <typename KeyCons, typename Value,
          typename ValCons, typename EqualCons>
struct equal_ckey_cval_normal {
  static bool compare(const KeyCons& c, const Value& v, const ValCons& vc,
                      const EqualCons& eq)
  {
    if (!eq.get_head()(c.get_head()(v), vc.get_head()))
      return false;
    return equal_ckey_cval<
              typename KeyCons::tail_type, Value,
              typename ValCons::tail_type,
              typename EqualCons::tail_type
           >::compare(c.get_tail(), v, vc.get_tail(), eq.get_tail());
  } // compare

  static bool compare(const ValCons& vc, const KeyCons& c, const Value& v,
                      const EqualCons& eq)
  {
    if (!eq.get_head()(vc.get_head(), c.get_head()(v)))
      return false;
    return equal_ckey_cval <
              typename KeyCons::tail_type, Value,
              typename ValCons::tail_type,
              typename EqualCons::tail_type
           >::compare(vc.get_tail(), c.get_tail(), v, eq.get_tail());
  } // compare
}; // equal_ckey_cval_normal

template <typename KeyCons, typename Value,
          typename ValCons, typename EqualCons>
struct equal_ckey_cval
  : mp11::mp_if <
      mp11::mp_or <
        std::is_same<KeyCons, cons_null>,
        std::is_same<ValCons, cons_null>
      >,
      equal_ckey_cval_terminal<KeyCons, Value, ValCons, EqualCons>,
      equal_ckey_cval_normal<KeyCons, Value, ValCons, EqualCons>
    >
{ };

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename CompareCons>
struct compare_ckey_ckey; /* fwd decl. */

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename CompareCons>
struct compare_ckey_ckey_terminal {
  static bool compare(
    const KeyCons1&, const Value1&,
    const KeyCons2&, const Value2&,
    const CompareCons&)
  { return false; }
}; // compare_ckey_ckey_terminal

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename CompareCons>
struct compare_ckey_ckey_normal {
  static bool compare(const KeyCons1& c0, const Value1& v0,
                      const KeyCons2& c1, const Value2& v1,
                      const CompareCons& comp)
  {
    if (comp.get_head()(c0.get_head()(v0), c1.get_head()(v1)))
      return true;
    if (comp.get_head()(c1.get_head()(v1), c0.get_head()(v0)))
      return false;
    return compare_ckey_ckey <
              typename KeyCons1::tail_type, Value1,
              typename KeyCons2::tail_type, Value2,
              typename CompareCons::tail_type
           >::compare(c0.get_tail(), v0, c1.get_tail(), v1, comp.get_tail());
  }
}; // compare_ckey_ckey_normal

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename CompareCons >
struct compare_ckey_ckey
  : mp11::mp_if <
      mp11::mp_or <
        std::is_same<KeyCons1, cons_null>,
        std::is_same<KeyCons2, cons_null>
      >,
      compare_ckey_ckey_terminal<KeyCons1, Value1,
                                 KeyCons2, Value2,
                                 CompareCons>,
      compare_ckey_ckey_normal<KeyCons1, Value1,
                               KeyCons2, Value2,
                               CompareCons>
    >
{ };

template <typename KeyCons, typename Value,
          typename ValCons, typename CompareCons>
struct compare_ckey_cval; /* fwd decl. */

template <typename KeyCons, typename Value,
          typename ValCons, typename CompareCons>
struct compare_ckey_cval_terminal {
  static bool compare(const KeyCons&, const Value&, const ValCons&,
                      const CompareCons&)
  { return false; }

  static bool compare(const ValCons&, const KeyCons&, const Value&,
                      const CompareCons&)
  { return false; }
}; // compare_ckey_cval_terminal

template <typename KeyCons, typename Value,
          typename ValCons, typename CompareCons>
struct compare_ckey_cval_normal {
  static bool compare(const KeyCons& c, const Value& v, const ValCons& vc,
                      const CompareCons& comp)
  {
    if (comp.get_head()(c.get_head()(v), vc.get_head()))
      return true;
    if (comp.get_head()(vc.get_head(), c.get_head()(v)))
      return false;
    return compare_ckey_cval <
              typename KeyCons::tail_type, Value,
              typename ValCons::tail_type,
              typename CompareCons::tail_type
           >::compare(c.get_tail(), v, vc.get_tail(), comp.get_tail());
  } // compare

  static bool compare(const ValCons& vc, const KeyCons& c, const Value& v,
                      const CompareCons& comp)
  {
    if (comp.get_head()(vc.get_head(), c.get_head()(v)))
      return true;
    if (comp.get_head()(c.get_head()(v), vc.get_head()))
      return false;
    return compare_ckey_cval <
              typename KeyCons::tail_type, Value,
              typename ValCons::tail_type,
              typename CompareCons::tail_type
           >::compare(vc.get_tail(), c.get_tail(), v, comp.get_tail());
  } // compare
}; // compare_ckey_cval_normal

template <typename KeyCons, typename Value,
          typename ValCons, typename CompareCons>
struct compare_ckey_cval
  : mp11::mp_if <
      mp11::mp_or <
        std::is_same<KeyCons, cons_null>,
        std::is_same<ValCons, cons_null>
      >,
      compare_ckey_cval_terminal<KeyCons, Value, ValCons, CompareCons>,
      compare_ckey_cval_normal<KeyCons, Value, ValCons, CompareCons>
    >
{ };

template<typename KeyCons, typename Value, typename HashCons>
struct hash_ckey; /* fwd decl. */

template<typename KeyCons, typename Value, typename HashCons>
struct hash_ckey_terminal {
  static std::size_t hash(const KeyCons&, const Value&, const HashCons&,
                          std::size_t carry)
  { return carry; }
}; // hash_ckey_terminal

template<typename KeyCons, typename Value, typename HashCons>
struct hash_ckey_normal {
  static std::size_t hash(const KeyCons& c, const Value& v, const HashCons& h,
                          std::size_t carry = 0)
  {
    /* same hashing formula as boost::hash_combine */

    carry ^= h.get_head()(c.get_head()(v)) + 0x9e3779b9 + (carry << 6) +
             (carry >> 2);
    return hash_ckey <
              typename KeyCons::tail_type, Value,
              typename HashCons::tail_type
           >::hash(c.get_tail(), v, h.get_tail(), carry);
  }
}; // hash_ckey_normal

template<typename KeyCons, typename Value, typename HashCons>
struct hash_ckey
  : mp11::mp_if <
      std::is_same<KeyCons, cons_null>,
      hash_ckey_terminal<KeyCons, Value, HashCons>,
      hash_ckey_normal<KeyCons, Value, HashCons>
    >
{ };

template<typename ValCons, typename HashCons>
struct hash_cval; /* fwd decl. */

template<typename ValCons, typename HashCons>
struct hash_cval_terminal {
  static std::size_t hash(const ValCons&, const HashCons&, std::size_t carry)
  { return carry; }
}; // hash_cval_terminal

template<typename ValCons, typename HashCons>
struct hash_cval_normal {
  static std::size_t hash(const ValCons& vc, const HashCons& h,
                          std::size_t carry = 0)
  {
    carry ^= h.get_head()(vc.get_head()) + 0x9e3779b9
             + (carry << 6) + (carry >> 2);
    return hash_cval <
              typename ValCons::tail_type,
              typename HashCons::tail_type
           >::hash(vc.get_tail(), h.get_tail(), carry);
  }
}; // hash_cval_normal

template<typename ValCons, typename HashCons>
struct hash_cval
  : mp11::mp_if <
      std::is_same<ValCons, cons_null>,
      hash_cval_terminal<ValCons, HashCons>,
      hash_cval_normal<ValCons, HashCons>
    >
{ };

} // detail

/* composite_key_result */

template<typename CompositeKey>
struct composite_key_result {
  using composite_key_type = CompositeKey;
  using value_type = typename composite_key_type::value_type;

  composite_key_result(const composite_key_type& composite_key_,
                       const value_type& value_)
    : composite_key(composite_key_), value(value_)
  {}

  const composite_key_type& composite_key;
  const value_type&         value;
}; // composite_key_result

/* composite_key */

template <typename Value, typename... KeyFromValueList>
struct composite_key
  : private cons_tuple<KeyFromValueList...>
{
private:
  using super = cons_tuple<KeyFromValueList...>;

public:
  using key_extractor_tuple = super;
  using value_type = Value;
  using result_type = composite_key_result<composite_key>;

  composite_key() : super() { }

  explicit composite_key(const KeyFromValueList&... keys) : super(keys...) {}

  composite_key(const key_extractor_tuple& x) : super(x) {}

  const key_extractor_tuple& key_extractors() const { return *this; }

  key_extractor_tuple&       key_extractors()       { return *this; }

  template<typename ChainedPtr>
  typename std::enable_if_t <
    !std::is_convertible_v<const ChainedPtr&, const value_type&>,
    result_type>
  operator()(const ChainedPtr& x) const { return operator()(*x); }

  result_type operator()(const value_type& x) const
  { return result_type(*this, x); }

  result_type operator()(const std::reference_wrapper<const value_type>& x)
      const
  { return result_type(*this, x.get()); }

  result_type operator()(const std::reference_wrapper<value_type>& x) const
  { return result_type(*this, x.get()); }
}; // composite_key

/* comparison operators */

/* == */

template<typename CompositeKey1, typename CompositeKey2>
inline bool operator==(const composite_key_result<CompositeKey1>& x,
                       const composite_key_result<CompositeKey2>& y)
{
  using key_extractor_tuple1 = typename CompositeKey1::key_extractor_tuple;
  using value_type1 = typename CompositeKey1::value_type;
  using key_extractor_tuple2 = typename CompositeKey2::key_extractor_tuple;
  using value_type2 = typename CompositeKey2::value_type;

  static_assert(   cons_size<key_extractor_tuple1>::value
                == cons_size<key_extractor_tuple2>::value);

  using operator_tuple =
      detail::generic_operator_equal_tuple<key_extractor_tuple1>;

  return detail::equal_ckey_ckey <
            key_extractor_tuple1, value_type1,
            key_extractor_tuple2, value_type2,
            operator_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    y.composite_key.key_extractors(), y.value,
                    operator_tuple());
}

template <typename CompositeKey, typename... Values>
inline bool operator==(const composite_key_result<CompositeKey>& x,
                       const cons_tuple<Values...>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = cons_tuple<Values...>;

  static_assert(   cons_size<key_extractor_tuple>::value
                == cons_size<key_tuple>::value);

  using operator_tuple =
      detail::generic_operator_equal_tuple<key_extractor_tuple>;

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            operator_tuple
         >::compare(x.composite_key.key_extractors(), x.value, y,
                    operator_tuple());
}

template <typename CompositeKey, typename... Values>
inline bool operator==(const cons_tuple<Values...>& x,
                       const composite_key_result<CompositeKey>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = cons_tuple<Values...>;

  static_assert(   cons_size<key_extractor_tuple>::value
                == cons_size<key_tuple>::value);

  using operator_tuple =
      detail::generic_operator_equal_tuple<key_extractor_tuple>;

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            operator_tuple
         >::compare(x, y.composite_key.key_extractors(), y.value,
                    operator_tuple());
}

template<typename CompositeKey, typename... Values>
inline bool operator==(const composite_key_result<CompositeKey>& x,
                       const std::tuple<Values...>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = std::tuple<Values...>;
  using cons_key_tuple =
      typename detail::cons_stdtuple_ctor < key_tuple >::result_type;

  static_assert(cons_size<key_extractor_tuple>::value
                == std::tuple_size<key_tuple>::value);

  using operator_tuple =
      detail::generic_operator_equal_tuple<key_extractor_tuple>;

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            cons_key_tuple,
            operator_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    detail::make_cons_stdtuple(y),
                    operator_tuple());
}

template<typename CompositeKey, typename... Values>
inline bool operator==(const std::tuple<Values...>& x,
                       const composite_key_result<CompositeKey>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = std::tuple<Values...>;
  using cons_key_tuple =
      typename detail::cons_stdtuple_ctor <key_tuple >::result_type;

  static_assert(cons_size<key_extractor_tuple>::value
                == std::tuple_size<key_tuple>::value);

  using operator_tuple =
      detail::generic_operator_equal_tuple<key_extractor_tuple>;

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            cons_key_tuple,
            operator_tuple
         >::compare(detail::make_cons_stdtuple(x),
                    y.composite_key.key_extractors(),
                    y.value,
                    operator_tuple());
}

/* < */

template<typename CompositeKey1, typename CompositeKey2>
inline bool operator<(const composite_key_result<CompositeKey1>& x,
                      const composite_key_result<CompositeKey2>& y)
{
  using key_extractor_tuple1 = typename CompositeKey1::key_extractor_tuple;
  using value_type1 = typename CompositeKey1::value_type;
  using key_extractor_tuple2 = typename CompositeKey2::key_extractor_tuple;
  using value_type2 = typename CompositeKey2::value_type;

  using operator_tuple =
      detail::generic_operator_less_tuple<key_extractor_tuple1>;

  return detail::compare_ckey_ckey <
            key_extractor_tuple1, value_type1,
            key_extractor_tuple2, value_type2,
            operator_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    y.composite_key.key_extractors(), y.value,
                    operator_tuple());
}

template <typename CompositeKey, typename... Values>
inline bool operator<(const composite_key_result<CompositeKey>& x,
                      const cons_tuple<Values...>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = cons_tuple<Values...>;

  using operator_tuple =
      detail::generic_operator_less_tuple<key_extractor_tuple>;

  return detail::compare_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            operator_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    y,
                    operator_tuple());
}

template <typename CompositeKey, typename... Values>
inline bool operator<(const cons_tuple<Values...>& x,
                      const composite_key_result<CompositeKey>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = cons_tuple<Values...>;

  using operator_tuple =
      detail::generic_operator_less_tuple<key_extractor_tuple>;

  return detail::compare_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            operator_tuple
         >::compare(x, y.composite_key.key_extractors(),
                    y.value,
                    operator_tuple());
}

template<typename CompositeKey, typename... Values>
inline bool operator<(const composite_key_result<CompositeKey>& x,
                      const std::tuple<Values...>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = std::tuple<Values...>;
  using cons_key_tuple =
      typename detail::cons_stdtuple_ctor<key_tuple>::result_type;

  using operator_tuple =
      detail::generic_operator_less_tuple<key_extractor_tuple>;

  return detail::compare_ckey_cval <
            key_extractor_tuple, value_type,
            cons_key_tuple,
            operator_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    detail::make_cons_stdtuple(y),
                    operator_tuple());
}

template<typename CompositeKey, typename... Values>
inline bool operator<(const std::tuple<Values...>& x,
                      const composite_key_result<CompositeKey>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = std::tuple<Values...>;
  using cons_key_tuple =
      typename detail::cons_stdtuple_ctor< key_tuple>::result_type;

  using operator_tuple =
      detail::generic_operator_less_tuple<key_extractor_tuple>;

  return detail::compare_ckey_cval <
          key_extractor_tuple, value_type,
          cons_key_tuple,
          operator_tuple
         >::compare(detail::make_cons_stdtuple(x),
                    y.composite_key.key_extractors(),
                    y.value,
                    operator_tuple());
}

/* rest of comparison operators */

#define BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS(t1,t2,a1,a2)                  \
template<t1, t2> inline bool operator!=(const a1& x, const a2& y)            \
{ return !(x==y); }                                                          \
                                                                             \
template<t1, t2> inline bool operator>(const a1& x, const a2& y)             \
{ return y<x; }                                                              \
                                                                             \
template<t1, t2> inline bool operator>=(const a1& x, const a2& y)            \
{ return !(x<y); }                                                           \
                                                                             \
template<t1, t2> inline bool operator<=(const a1& x, const a2& y)            \
{ return !(y<x); }                                                           \

BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename CompositeKey1,
  typename CompositeKey2,
  composite_key_result<CompositeKey1>,
  composite_key_result<CompositeKey2>
)

BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename CompositeKey,
  typename... Values,
  composite_key_result<CompositeKey>,
  cons_tuple<Values...>
)

BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename... Values,
  typename CompositeKey,
  cons_tuple<Values...>,
  composite_key_result<CompositeKey>
)

BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename CompositeKey,
  typename... Values,
  composite_key_result<CompositeKey>,
  std::tuple<Values...>
)

BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename CompositeKey,
  typename... Values,
  std::tuple<Values...>,
  composite_key_result<CompositeKey>
)

#undef BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS

/* composite_key_equal_to */

template <typename... PredList>
struct composite_key_equal_to
  : private cons_tuple<PredList...>
{
private:
  using super = cons_tuple<PredList...>;

public:
  using key_eq_tuple = super;

  composite_key_equal_to() : super() { }

  explicit composite_key_equal_to(const PredList&... args) : super(args...) {}

  composite_key_equal_to(const key_eq_tuple& x) : super(x) {}

  const key_eq_tuple& key_eqs() const { return *this; }
  key_eq_tuple&       key_eqs()       { return *this; }

  template<typename CompositeKey1, typename CompositeKey2>
  bool operator()(const composite_key_result<CompositeKey1>& x,
                  const composite_key_result<CompositeKey2>& y) const
  {
    using key_extractor_tuple1 = typename CompositeKey1::key_extractor_tuple;
    using value_type1 = typename CompositeKey1::value_type;
    using key_extractor_tuple2 = typename CompositeKey2::key_extractor_tuple;
    using value_type2 = typename CompositeKey2::value_type;

    static_assert(     cons_size<key_extractor_tuple1>::value
                    <= cons_size<key_eq_tuple>::value
                  &&   cons_size<key_extractor_tuple1>::value
                    == cons_size<key_extractor_tuple2>::value);

    return detail::equal_ckey_ckey <
              key_extractor_tuple1, value_type1,
              key_extractor_tuple2, value_type2,
              key_eq_tuple
           >::compare(x.composite_key.key_extractors(), x.value,
                      y.composite_key.key_extractors(), y.value,
                      key_eqs());
  }

  template <typename CompositeKey, typename... Values>
  bool operator()(const composite_key_result<CompositeKey>& x,
                  const cons_tuple<Values...>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = cons_tuple<Values...>;

    static_assert(     cons_size<key_extractor_tuple>::value
                    <= cons_size<key_eq_tuple>::value
                  &&   cons_size<key_extractor_tuple>::value
                    == cons_size<key_tuple>::value);

    return detail::equal_ckey_cval <
              key_extractor_tuple, value_type,
              key_tuple,
              key_eq_tuple
           >::compare(x.composite_key.key_extractors(), x.value, y, key_eqs());
  }

  template <typename CompositeKey, typename... Values>
  bool operator()(const cons_tuple<Values...>& x,
                  const composite_key_result<CompositeKey>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = cons_tuple<Values...>;

    static_assert(     cons_size<key_tuple>::value
                    <= cons_size<key_eq_tuple>::value
                  && cons_size<key_tuple>::value
                    == cons_size<key_extractor_tuple>::value);

    return detail::equal_ckey_cval <
              key_extractor_tuple, value_type,
              key_tuple,
              key_eq_tuple
           >::compare(x, y.composite_key.key_extractors(), y.value, key_eqs());
  }

  template<typename CompositeKey, typename... Values>
  bool operator()(const composite_key_result<CompositeKey>& x,
                  const std::tuple<Values...>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = std::tuple<Values...>;
    using cons_key_tuple =
        typename detail::cons_stdtuple_ctor<key_tuple >::result_type;

    static_assert(cons_size<key_extractor_tuple>::value
                <= cons_size<key_eq_tuple>::value
              && cons_size<key_extractor_tuple>::value
                == std::tuple_size<key_tuple>::value);

    return detail::equal_ckey_cval <
              key_extractor_tuple, value_type,
              cons_key_tuple,
              key_eq_tuple
           >::compare(x.composite_key.key_extractors(), x.value,
                      detail::make_cons_stdtuple(y),
                      key_eqs());
  }

  template<typename CompositeKey, typename... Values>
  bool operator()(const std::tuple<Values...>& x,
                  const composite_key_result<CompositeKey>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = std::tuple<Values...>;
    using cons_key_tuple =
        typename detail::cons_stdtuple_ctor<key_tuple>::result_type;

    static_assert(std::tuple_size<key_tuple>::value
            <= cons_size<key_eq_tuple>::value
          && std::tuple_size<key_tuple>::value
            == cons_size<key_extractor_tuple>::value);

    return detail::equal_ckey_cval <
              key_extractor_tuple, value_type,
              cons_key_tuple,
              key_eq_tuple
           >::compare(detail::make_cons_stdtuple(x),
                      y.composite_key.key_extractors(),
                      y.value,
                      key_eqs());
  }
}; // composite_key_equal_to

/* composite_key_compare */

template <typename... CompareList>
struct composite_key_compare
  : private cons_tuple<CompareList...>
{
private:
  using super = cons_tuple<CompareList...>;

public:
  using key_comp_tuple = super;

  composite_key_compare() : super() {}

  explicit composite_key_compare(const CompareList&... args) : super(args...) {}

  composite_key_compare(const key_comp_tuple& x) : super(x) {}

  const key_comp_tuple& key_comps() const { return *this; }
  key_comp_tuple&       key_comps()       { return *this; }

  template<typename CompositeKey1, typename CompositeKey2>
  bool operator()(const composite_key_result<CompositeKey1>& x,
                  const composite_key_result<CompositeKey2>& y)
  const {
    using key_extractor_tuple1 = typename CompositeKey1::key_extractor_tuple;
    using value_type1 = typename CompositeKey1::value_type;
    using key_extractor_tuple2 = typename CompositeKey2::key_extractor_tuple;
    using value_type2 = typename CompositeKey2::value_type;

    static_assert(     cons_size<key_extractor_tuple1>::value
                    <= cons_size<key_comp_tuple>::value
                  ||   cons_size<key_extractor_tuple2>::value
                    <= cons_size<key_comp_tuple>::value);

    return detail::compare_ckey_ckey <
              key_extractor_tuple1, value_type1,
              key_extractor_tuple2, value_type2,
              key_comp_tuple
           >::compare(x.composite_key.key_extractors(), x.value,
                      y.composite_key.key_extractors(), y.value,
                      key_comps());
  }

  template<typename CompositeKey, typename Value>
  bool operator()(const composite_key_result<CompositeKey>& x, const Value& y)
    const
  { return operator()(x, make_cons_tuple(std::cref(y))); }

  template <typename CompositeKey, typename... Values>
  bool operator()(const composite_key_result<CompositeKey>& x,
                  const cons_tuple<Values...>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = cons_tuple<Values...>;

    static_assert(     cons_size<key_extractor_tuple>::value
                    <= cons_size<key_comp_tuple>::value
                  ||   cons_size<key_tuple>::value
                    <= cons_size<key_comp_tuple>::value);

    return detail::compare_ckey_cval <
              key_extractor_tuple, value_type,
              key_tuple,
              key_comp_tuple
           >::compare(x.composite_key.key_extractors(), x.value,
                      y,
                      key_comps());
  }

  template<typename Value, typename CompositeKey>
  bool operator()(const Value& x, const composite_key_result<CompositeKey>& y)
    const
  { return operator()(make_cons_tuple(std::cref(x)), y); }

  template <typename CompositeKey, typename... Values>
  bool operator()(const cons_tuple<Values...>& x,
                  const composite_key_result<CompositeKey>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = cons_tuple<Values...>;

    static_assert(     cons_size<key_tuple>::value
                    <= cons_size<key_comp_tuple>::value
                  ||   cons_size<key_extractor_tuple>::value
                    <= cons_size<key_comp_tuple>::value);

    return detail::compare_ckey_cval <
              key_extractor_tuple, value_type,
              key_tuple,
              key_comp_tuple
           >::compare(x, y.composite_key.key_extractors(),
                      y.value,
                      key_comps());
  }

  template<typename CompositeKey, typename... Values>
  bool operator()(const composite_key_result<CompositeKey>& x,
                  const std::tuple<Values...>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = std::tuple<Values...>;
    using cons_key_tuple =
        typename detail::cons_stdtuple_ctor<key_tuple >::result_type;

    static_assert( cons_size<key_extractor_tuple>::value
                <= cons_size<key_comp_tuple>::value
              || std::tuple_size<key_tuple>::value
                <= cons_size<key_comp_tuple>::value);

    return detail::compare_ckey_cval <
              key_extractor_tuple, value_type,
              cons_key_tuple,
              key_comp_tuple
           >::compare(x.composite_key.key_extractors(), x.value,
                      detail::make_cons_stdtuple(y),
                      key_comps());
  }

  template<typename CompositeKey, typename... Values>
  bool operator()(const std::tuple<Values...>& x,
                  const composite_key_result<CompositeKey>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = std::tuple<Values...>;
    using cons_key_tuple =
        typename detail::cons_stdtuple_ctor<key_tuple >::result_type;

    static_assert(   std::tuple_size<key_tuple>::value
                  <= cons_size<key_comp_tuple>::value
                ||   cons_size<key_extractor_tuple>::value
                  <= cons_size<key_comp_tuple>::value);

    return detail::compare_ckey_cval <
              key_extractor_tuple, value_type,
              cons_key_tuple,
              key_comp_tuple
           >::compare(detail::make_cons_stdtuple(x),
                      y.composite_key.key_extractors(),
                      y.value,
                      key_comps());
  }
}; // composite_key_compare

/* composite_key_hash */

template <typename... HashList>
struct composite_key_hash
  : private cons_tuple<HashList...>
{
private:
  using super = cons_tuple<HashList...>;

public:
  using key_hasher_tuple = super;

  composite_key_hash() : super() { }

  explicit composite_key_hash(const HashList&... args...) : super(args...) {}

  composite_key_hash(const key_hasher_tuple& x) : super(x) {}

  const key_hasher_tuple& key_hash_functions() const { return *this; }

  key_hasher_tuple&       key_hash_functions()       { return *this; }

  template<typename CompositeKey>
  std::size_t operator()(const composite_key_result<CompositeKey>& x) const {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;

    static_assert(   cons_size<key_extractor_tuple>::value
                  == cons_size<key_hasher_tuple>::value);

    return detail::hash_ckey <
              key_extractor_tuple, value_type,
              key_hasher_tuple
           >::hash(x.composite_key.key_extractors(), x.value,
                   key_hash_functions());
  }

  template<typename... Values>
  std::size_t operator()(const cons_tuple<Values...>& x) const {
    using key_tuple = cons_tuple<Values...>;

    static_assert(   cons_size<key_tuple>::value
                  == cons_size<key_hasher_tuple>::value);

    return detail::hash_cval <
              key_tuple,
              key_hasher_tuple
           >::hash(x, key_hash_functions());
  }

  template<typename... Values>
  std::size_t operator()(const std::tuple<Values...>& x) const {
    using key_tuple = std::tuple<Values...>;
    using cons_key_tuple =
        typename detail::cons_stdtuple_ctor<key_tuple>::result_type;

    static_assert(std::tuple_size<key_tuple>::value
              == cons_size<key_hasher_tuple>::value);

    return detail::hash_cval <
              cons_key_tuple,
              key_hasher_tuple
           >::hash(detail::make_cons_stdtuple(x), key_hash_functions());
  }
}; // composite_key_hash

namespace detail {

template<typename CompositeKeyResult>
struct composite_key_result_equal_to
  : private mp11::mp_apply<composite_key_equal_to,
                           key_equal_to_list<CompositeKeyResult>>
{
 private:
  using super = mp11::mp_apply<composite_key_equal_to,
                               key_equal_to_list<CompositeKeyResult>>;

 public:
  using first_argument_type  = CompositeKeyResult;
  using second_argument_type = first_argument_type;
  using result_type = bool;

  using super::operator();
}; // composite_key_result_equal_to

template<typename CompositeKeyResult>
struct composite_key_result_less
  : private composite_key_compare<
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 0>::type,
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 1>::type,
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 2>::type,
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 3>::type,
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 4>::type,
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 5>::type,
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 6>::type,
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 7>::type,
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 8>::type,
              typename detail::nth_composite_key_less<
                typename CompositeKeyResult::composite_key_type, 9>::type
              >
{
 private:
  using super = composite_key_compare<
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 0>::type,
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 1>::type,
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 2>::type,
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 3>::type,
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 4>::type,
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 5>::type,
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 6>::type,
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 7>::type,
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 8>::type,
                  typename detail::nth_composite_key_less<
                    typename CompositeKeyResult::composite_key_type, 9>::type
      >;

 public:
  using first_argument_type  = CompositeKeyResult;
  using second_argument_type = first_argument_type;
  using result_type = bool;

  using super::operator();
}; // composite_key_result_less

template<typename CompositeKeyResult>
struct composite_key_result_greater
  : private composite_key_compare<
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 0>::type,
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 1>::type,
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 2>::type,
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 3>::type,
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 4>::type,
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 5>::type,
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 6>::type,
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 7>::type,
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 8>::type,
              typename detail::nth_composite_key_greater<
                typename CompositeKeyResult::composite_key_type, 9>::type
            >
{
 private:
  using super = composite_key_compare<
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 0>::type,
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 1>::type,
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 2>::type,
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 3>::type,
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 4>::type,
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 5>::type,
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 6>::type,
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 7>::type,
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 8>::type,
                  typename detail::nth_composite_key_greater<
                    typename CompositeKeyResult::composite_key_type, 9>::type
                >;

 public:
  using first_argument_type  = CompositeKeyResult;
  using second_argument_type = first_argument_type;
  using result_type = bool;

  using super::operator();
}; // composite_key_result_greater

template<typename CompositeKeyResult>
struct composite_key_result_hash
  : private composite_key_hash<
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 0>::type,
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 1>::type,
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 2>::type,
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 3>::type,
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 4>::type,
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 5>::type,
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 6>::type,
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 7>::type,
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 8>::type,
              typename detail::nth_composite_key_hash<
                typename CompositeKeyResult::composite_key_type, 9>::type
            >
{
 private:
  using super = composite_key_hash<
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 0>::type,
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 1>::type,
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 2>::type,
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 3>::type,
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 4>::type,
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 5>::type,
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 6>::type,
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 7>::type,
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 8>::type,
                  typename detail::nth_composite_key_hash<
                    typename CompositeKeyResult::composite_key_type, 9>::type
                >;

 public:
  using argument_type = CompositeKeyResult;
  using result_type = std::size_t;

  using super::operator();
}; // composite_key_result_hash

} // detail

} // boost::multi_index

/* Specializations of std::equal_to, std::less, std::greater and std::hash
 * for composite_key_results enabling interoperation with tuples of values.
 */

namespace std {

template<typename CompositeKey>
struct equal_to<boost::multi_index::composite_key_result<CompositeKey>>
  : boost::multi_index::detail::composite_key_result_equal_to<
      boost::multi_index::composite_key_result<CompositeKey>
    >
{ };

template<typename CompositeKey>
struct less<boost::multi_index::composite_key_result<CompositeKey>>
  : boost::multi_index::detail::composite_key_result_less<
      boost::multi_index::composite_key_result<CompositeKey>
    >
{ };

template<typename CompositeKey>
struct greater<boost::multi_index::composite_key_result<CompositeKey>>
  : boost::multi_index::detail::composite_key_result_greater<
      boost::multi_index::composite_key_result<CompositeKey>
    >
{ };

template<typename CompositeKey>
struct hash<boost::multi_index::composite_key_result<CompositeKey>>
  : boost::multi_index::detail::composite_key_result_hash<
      boost::multi_index::composite_key_result<CompositeKey>
    >
{ };

} // std

#endif
