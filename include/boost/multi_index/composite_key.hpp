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

template<typename KeyFromValue>
using result_type_of = typename KeyFromValue::result_type;

template<typename CompositeKeyResult, template<typename> typename Compare>
struct key_list_helper {
  using composite_key_type = typename CompositeKeyResult::composite_key_type;
  using key_extractor_tuple = typename composite_key_type::key_extractor_tuple;
  using key_extractor_list =
                            mp11::mp_rename<key_extractor_tuple, mp11::mp_list>;
  using key_result_type = mp11::mp_transform<
      result_type_of,
      mp11::mp_remove<key_extractor_list, cons_null>
  >;
  using type = mp11::mp_transform<Compare, key_result_type>;
}; // key_list_helper

template<typename CompositeKeyResult, template<typename> typename Compare>
using key_list = typename key_list_helper<CompositeKeyResult, Compare>::type;

template<typename T>
using key_equal_to = mp11::mp_identity_t<std::equal_to<T>>;

template<typename T>
using key_less = mp11::mp_identity_t<std::less<T>>;

template<typename T>
using key_greater = mp11::mp_identity_t<std::greater<T>>;

template<typename T>
using key_hash = mp11::mp_identity_t<std::hash<T>>;

/* used for defining equality and comparison ops of composite_key_result */

struct generic_operator_equal {
  template<typename T, typename Q>
  bool operator()(const T& x, const Q& y) const
  { return x == y; }
};

template<typename T>
using generic_operator_equal_tuple =
  mp11::mp_fill<mp11::mp_rename<T, std::tuple>, generic_operator_equal>;

struct generic_operator_less {
  template<typename T, typename Q>
  bool operator()(const T& x, const Q& y) const
  { return x < y; }
};

template<typename T>
using generic_operator_less_tuple =
    mp11::mp_fill<mp11::mp_rename<T, std::tuple>, generic_operator_less>;

/* Metaprogramming machinery for implementing equality, comparison and
 * hashing operations of composite_key_result.
 *
 * equal_* checks for equality between composite_key_results and
 * between those and tuples, accepting a std::tuple of basic equality functors.
 * compare_* does lexicographical comparison.
 * hash_* computes a combination of elementwise hash values.
 */

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename EqualCons, std::size_t I=0>
struct equal_ckey_ckey; /* fwd decl. */

template <typename KeyCons1, typename Value1,
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
          typename EqualCons, std::size_t I>
struct equal_ckey_ckey_normal {
  static bool compare(const KeyCons1& c0, const Value1& v0,
                      const KeyCons2& c1, const Value2& v1,
                      const EqualCons& eq)
  {
    if (!std::get<I>(eq)(std::get<I>(c0)(v0), std::get<I>(c1)(v1)))
      return false;
    return equal_ckey_ckey<KeyCons1, Value1,
                           KeyCons2, Value2,
                           EqualCons, (I+1)
           >::compare(c0, v0, c1, v1, eq);
  }
}; // equal_ckey_ckey_normal

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename EqualCons, std::size_t I>
struct equal_ckey_ckey
  : std::conditional_t<
    (I < std::tuple_size_v<KeyCons1> && I < std::tuple_size_v<KeyCons2>),
    equal_ckey_ckey_normal  <KeyCons1, Value1, KeyCons2, Value2, EqualCons, I>,
    equal_ckey_ckey_terminal<KeyCons1, Value1, KeyCons2, Value2, EqualCons>
    >
{ };

template <typename KeyCons, typename Value,
          typename ValCons, typename EqualCons, std::size_t I=0>
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
          typename ValCons, typename EqualCons, std::size_t I>
struct equal_ckey_cval_normal {
  static bool compare(const KeyCons& c, const Value& v, const ValCons& vc,
                      const EqualCons& eq)
  {
    if (!std::get<I>(eq)(std::get<I>(c)(v), std::get<I>(vc)))
      return false;
    return equal_ckey_cval<KeyCons, Value,
                           ValCons,
                           EqualCons, (I+1)
           >::compare(c, v, vc, eq);
  } // compare

  static bool compare(const ValCons& vc, const KeyCons& c, const Value& v,
                      const EqualCons& eq)
  {
    if (!std::get<I>(eq)(std::get<I>(vc), std::get<I>(c)(v)))
      return false;
    return equal_ckey_cval<KeyCons, Value,
                           ValCons,
                           EqualCons, (I+1)
           >::compare(vc, c, v, eq);
  } // compare
}; // equal_ckey_cval_normal

template <typename KeyCons, typename Value,
          typename ValCons, typename EqualCons, std::size_t I>
struct equal_ckey_cval
  : std::conditional_t<
      (I < std::tuple_size_v<KeyCons> && I < std::tuple_size_v<ValCons>),
      equal_ckey_cval_normal  <KeyCons, Value, ValCons, EqualCons, I>,
      equal_ckey_cval_terminal<KeyCons, Value, ValCons, EqualCons>
    >
{ };

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename CompareCons, std::size_t I=0>
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
          typename CompareCons, std::size_t I>
struct compare_ckey_ckey_normal {
  static bool compare(const KeyCons1& c0, const Value1& v0,
                      const KeyCons2& c1, const Value2& v1,
                      const CompareCons& comp)
  {
    const auto& comp_i = std::get<I>(comp);
    const auto& c0_i   = std::get<I>(c0)(v0);
    const auto& c1_i   = std::get<I>(c1)(v1);
    if (comp_i(c0_i, c1_i))
      return true;
    if (comp_i(c1_i, c0_i))
      return false;
    return compare_ckey_ckey<KeyCons1, Value1,
                             KeyCons2, Value2,
                             CompareCons, (I+1)
           >::compare(c0, v0, c1, v1, comp);
  }
}; // compare_ckey_ckey_normal

template <typename KeyCons1, typename Value1,
          typename KeyCons2, typename Value2,
          typename CompareCons, std::size_t I>
struct compare_ckey_ckey
  : std::conditional_t<
      (I < std::tuple_size_v<KeyCons1> && I < std::tuple_size_v<KeyCons2>),
      compare_ckey_ckey_normal  <KeyCons1, Value1,
                                 KeyCons2, Value2,
                                 CompareCons, I>,
      compare_ckey_ckey_terminal<KeyCons1, Value1,
                                 KeyCons2, Value2,
                                 CompareCons>
    >
{ };

template <typename KeyCons, typename Value,
          typename ValCons, typename CompareCons, std::size_t I=0>
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
          typename ValCons, typename CompareCons, std::size_t I>
struct compare_ckey_cval_normal {
  static bool compare(const KeyCons& c, const Value& v, const ValCons& vc,
                      const CompareCons& comp)
  {
    const auto& comp_i = std::get<I>(comp);
    const auto& c_i    = std::get<I>(c)(v);
    const auto& vc_i   = std::get<I>(vc);
    if (comp_i(c_i, vc_i))
      return true;
    if (comp_i(vc_i, c_i))
      return false;
    return compare_ckey_cval<KeyCons, Value,
                             ValCons,
                             CompareCons, (I+1)
           >::compare(c, v, vc, comp);
  } // compare

  static bool compare(const ValCons& vc, const KeyCons& c, const Value& v,
                      const CompareCons& comp)
  {
    const auto& comp_i = std::get<I>(comp);
    const auto& vc_i   = std::get<I>(vc);
    const auto& c_i    = std::get<I>(c)(v);
    if (comp_i(vc_i, c_i))
      return true;
    if (comp_i(c_i, vc_i))
      return false;
    return compare_ckey_cval<KeyCons, Value,
                             ValCons,
                             CompareCons, (I+1)
           >::compare(vc, c, v, comp);
  } // compare
}; // compare_ckey_cval_normal

template <typename KeyCons, typename Value,
          typename ValCons, typename CompareCons, std::size_t I>
struct compare_ckey_cval
  : std::conditional_t<
      (I < std::tuple_size_v<KeyCons> && I < std::tuple_size_v<ValCons>),
      compare_ckey_cval_normal  <KeyCons, Value, ValCons, CompareCons, I>,
      compare_ckey_cval_terminal<KeyCons, Value, ValCons, CompareCons>
    >
{ };

template<typename KeyCons, typename Value, typename HashCons, std::size_t I=0>
struct hash_ckey; /* fwd decl. */

template<typename KeyCons, typename Value, typename HashCons>
struct hash_ckey_terminal {
  static std::size_t hash(const KeyCons&, const Value&, const HashCons&,
                          std::size_t carry)
  { return carry; }
}; // hash_ckey_terminal

namespace {

/* same hashing formula as boost::hash_combine */

inline void hash_combine(std::size_t& seed, std::size_t value) noexcept
{ seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2); }

}

template<typename KeyCons, typename Value, typename HashCons, std::size_t I>
struct hash_ckey_normal {
  static std::size_t hash(const KeyCons& c, const Value& v, const HashCons& h,
                          std::size_t carry = 0)
  {
    hash_combine(carry, std::get<I>(h)(std::get<I>(c)(v)));
    return hash_ckey<KeyCons, Value, HashCons, (I+1)>::hash(c, v, h, carry);
  }
}; // hash_ckey_normal

template<typename KeyCons, typename Value, typename HashCons, std::size_t I>
struct hash_ckey
  : std::conditional_t<
      (I < std::tuple_size_v<KeyCons>),
      hash_ckey_normal  <KeyCons, Value, HashCons, I>,
      hash_ckey_terminal<KeyCons, Value, HashCons>
    >
{ };

template<typename ValCons, typename HashCons, std::size_t I=0>
struct hash_cval; /* fwd decl. */

template<typename ValCons, typename HashCons>
struct hash_cval_terminal {
  static std::size_t hash(const ValCons&, const HashCons&, std::size_t carry)
  { return carry; }
}; // hash_cval_terminal

template<typename ValCons, typename HashCons, std::size_t I>
struct hash_cval_normal {
  static std::size_t hash(const ValCons& vc, const HashCons& h,
                          std::size_t carry = 0)
  {
    hash_combine(carry, std::get<I>(h)(std::get<I>(vc)));
    return hash_cval<ValCons, HashCons, (I+1)>::hash(vc, h, carry);
  }
}; // hash_cval_normal

template<typename ValCons, typename HashCons, std::size_t I>
struct hash_cval
  : std::conditional_t<
      (I < std::tuple_size_v<ValCons>),
      hash_cval_normal  <ValCons, HashCons, I>,
      hash_cval_terminal<ValCons, HashCons>
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
struct composite_key {
public:
  using key_extractor_tuple = std::tuple<KeyFromValueList...>;
  using value_type = Value;
  using result_type = composite_key_result<composite_key>;

private:
  key_extractor_tuple key_ex;

public:
  composite_key() : key_ex() { }

  explicit composite_key(const KeyFromValueList&... keys) : key_ex(keys...) {}

  composite_key(const key_extractor_tuple& x) : key_ex(x) {}

  const key_extractor_tuple& key_extractors() const { return key_ex; }

  key_extractor_tuple&       key_extractors()       { return key_ex; }

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

  static_assert(   std::tuple_size_v<key_extractor_tuple1>
                == std::tuple_size_v<key_extractor_tuple2>);

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

  static_assert(   std::tuple_size_v<key_extractor_tuple>
                == std::tuple_size_v<key_tuple>);

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

  static_assert(   std::tuple_size_v<key_extractor_tuple>
                == std::tuple_size_v<key_tuple>);

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

  static_assert(std::tuple_size_v<key_extractor_tuple>
                == std::tuple_size_v<key_tuple>);

  using operator_tuple =
      detail::generic_operator_equal_tuple<key_extractor_tuple>;

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            operator_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    y,
                    operator_tuple());
}

template<typename CompositeKey, typename... Values>
inline bool operator==(const std::tuple<Values...>& x,
                       const composite_key_result<CompositeKey>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = std::tuple<Values...>;

  static_assert(std::tuple_size_v<key_extractor_tuple>
                == std::tuple_size_v<key_tuple>);

  using operator_tuple =
      detail::generic_operator_equal_tuple<key_extractor_tuple>;

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            operator_tuple
         >::compare(x,
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

template<typename CompositeKey, typename... Values>
inline bool operator<(const std::tuple<Values...>& x,
                      const composite_key_result<CompositeKey>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = std::tuple<Values...>;

  using operator_tuple =
      detail::generic_operator_less_tuple<key_extractor_tuple>;

  return detail::compare_ckey_cval <
          key_extractor_tuple, value_type,
          key_tuple,
          operator_tuple
         >::compare(x,
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
struct composite_key_equal_to {
public:
  using key_eq_tuple = cons_tuple<PredList...>;

private:
  key_eq_tuple _key_eqs;

public:
  composite_key_equal_to() : _key_eqs() { }

  explicit composite_key_equal_to(const PredList&... args)
      : _key_eqs(args...) {}

  composite_key_equal_to(const key_eq_tuple& x) : _key_eqs(x) {}

  const key_eq_tuple& key_eqs() const { return _key_eqs; }
  key_eq_tuple&       key_eqs()       { return _key_eqs; }

  template<typename CompositeKey1, typename CompositeKey2>
  bool operator()(const composite_key_result<CompositeKey1>& x,
                  const composite_key_result<CompositeKey2>& y) const
  {
    using key_extractor_tuple1 = typename CompositeKey1::key_extractor_tuple;
    using value_type1 = typename CompositeKey1::value_type;
    using key_extractor_tuple2 = typename CompositeKey2::key_extractor_tuple;
    using value_type2 = typename CompositeKey2::value_type;

    static_assert(     std::tuple_size_v<key_extractor_tuple1>
                    <= std::tuple_size_v<key_eq_tuple>
                  &&   std::tuple_size_v<key_extractor_tuple1>
                    == std::tuple_size_v<key_extractor_tuple2>);

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

    static_assert(     std::tuple_size_v<key_extractor_tuple>
                    <= std::tuple_size_v<key_eq_tuple>
                  &&   std::tuple_size_v<key_extractor_tuple>
                    == std::tuple_size_v<key_tuple>);

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

    static_assert(     std::tuple_size_v<key_tuple>
                    <= std::tuple_size_v<key_eq_tuple>
                  && std::tuple_size_v<key_tuple>
                    == std::tuple_size_v<key_extractor_tuple>);

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

    static_assert(std::tuple_size_v<key_extractor_tuple>
                <= std::tuple_size_v<key_eq_tuple>
              && std::tuple_size_v<key_extractor_tuple>
                == std::tuple_size_v<key_tuple>);

    return detail::equal_ckey_cval <
              key_extractor_tuple, value_type,
              key_tuple,
              key_eq_tuple
           >::compare(x.composite_key.key_extractors(), x.value,
                      y,
                      key_eqs());
  }

  template<typename CompositeKey, typename... Values>
  bool operator()(const std::tuple<Values...>& x,
                  const composite_key_result<CompositeKey>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = std::tuple<Values...>;

    static_assert(std::tuple_size_v<key_tuple>
            <= std::tuple_size_v<key_eq_tuple>
          && std::tuple_size_v<key_tuple>
            == std::tuple_size_v<key_extractor_tuple>);

    return detail::equal_ckey_cval <
              key_extractor_tuple, value_type,
              key_tuple,
              key_eq_tuple
           >::compare(x,
                      y.composite_key.key_extractors(),
                      y.value,
                      key_eqs());
  }
}; // composite_key_equal_to

/* composite_key_compare */

template <typename... CompareList>
struct composite_key_compare {
public:
  using key_comp_tuple = cons_tuple<CompareList...>;

private:
  key_comp_tuple _key_comps;

public:
  composite_key_compare() : _key_comps() {}

  explicit composite_key_compare(const CompareList&... args)
      : _key_comps(args...) {}

  composite_key_compare(const key_comp_tuple& x) : _key_comps(x) {}

  const key_comp_tuple& key_comps() const { return _key_comps; }
  key_comp_tuple&       key_comps()       { return _key_comps; }

  template<typename CompositeKey1, typename CompositeKey2>
  bool operator()(const composite_key_result<CompositeKey1>& x,
                  const composite_key_result<CompositeKey2>& y)
  const {
    using key_extractor_tuple1 = typename CompositeKey1::key_extractor_tuple;
    using value_type1 = typename CompositeKey1::value_type;
    using key_extractor_tuple2 = typename CompositeKey2::key_extractor_tuple;
    using value_type2 = typename CompositeKey2::value_type;

    static_assert(     std::tuple_size_v<key_extractor_tuple1>
                    <= std::tuple_size_v<key_comp_tuple>
                  ||   std::tuple_size_v<key_extractor_tuple2>
                    <= std::tuple_size_v<key_comp_tuple>);

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

    static_assert(     std::tuple_size_v<key_extractor_tuple>
                    <= std::tuple_size_v<key_comp_tuple>
                  ||   std::tuple_size_v<key_tuple>
                    <= std::tuple_size_v<key_comp_tuple>);

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

    static_assert(     std::tuple_size_v<key_tuple>
                    <= std::tuple_size_v<key_comp_tuple>
                  ||   std::tuple_size_v<key_extractor_tuple>
                    <= std::tuple_size_v<key_comp_tuple>);

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

    static_assert( std::tuple_size_v<key_extractor_tuple>
                <= std::tuple_size_v<key_comp_tuple>
              || std::tuple_size_v<key_tuple>
                <= std::tuple_size_v<key_comp_tuple>);

    return detail::compare_ckey_cval <
              key_extractor_tuple, value_type,
              key_tuple,
              key_comp_tuple
           >::compare(x.composite_key.key_extractors(), x.value,
                      y,
                      key_comps());
  }

  template<typename CompositeKey, typename... Values>
  bool operator()(const std::tuple<Values...>& x,
                  const composite_key_result<CompositeKey>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = std::tuple<Values...>;

    static_assert(   std::tuple_size_v<key_tuple>
                  <= std::tuple_size_v<key_comp_tuple>
                ||   std::tuple_size_v<key_extractor_tuple>
                  <= std::tuple_size_v<key_comp_tuple>);

    return detail::compare_ckey_cval <
              key_extractor_tuple, value_type,
              key_tuple,
              key_comp_tuple
           >::compare(x,
                      y.composite_key.key_extractors(),
                      y.value,
                      key_comps());
  }
}; // composite_key_compare

/* composite_key_hash */

template <typename... HashList>
struct composite_key_hash {
public:
  using key_hasher_tuple = cons_tuple<HashList...>;

private:
  key_hasher_tuple _key_hash_fns;

public:
  composite_key_hash() : _key_hash_fns() { }

  explicit composite_key_hash(const HashList&... args...)
      : _key_hash_fns(args...) {}

  composite_key_hash(const key_hasher_tuple& x) : _key_hash_fns(x) {}

  const key_hasher_tuple& key_hash_functions() const { return _key_hash_fns; }

  key_hasher_tuple&       key_hash_functions()       { return _key_hash_fns; }

  template<typename CompositeKey>
  std::size_t operator()(const composite_key_result<CompositeKey>& x) const {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;

    static_assert(   std::tuple_size_v<key_extractor_tuple>
                  == std::tuple_size_v<key_hasher_tuple>);

    return detail::hash_ckey <
              key_extractor_tuple, value_type,
              key_hasher_tuple
           >::hash(x.composite_key.key_extractors(), x.value,
                   key_hash_functions());
  }

  template<typename... Values>
  std::size_t operator()(const cons_tuple<Values...>& x) const {
    using key_tuple = cons_tuple<Values...>;

    static_assert(   std::tuple_size_v<key_tuple>
                  == std::tuple_size_v<key_hasher_tuple>);

    return detail::hash_cval <
              key_tuple,
              key_hasher_tuple
           >::hash(x, key_hash_functions());
  }

  template<typename... Values>
  std::size_t operator()(const std::tuple<Values...>& x) const {
    using key_tuple = std::tuple<Values...>;

    static_assert(std::tuple_size_v<key_tuple>
              == std::tuple_size_v<key_hasher_tuple>);

    return detail::hash_cval <
              key_tuple,
              key_hasher_tuple
           >::hash(x, key_hash_functions());
  }
}; // composite_key_hash

namespace detail {

template<typename CompositeKeyResult>
struct composite_key_result_equal_to
  : private mp11::mp_apply<composite_key_equal_to,
                           key_list<CompositeKeyResult, key_equal_to>>
{
 private:
  using super = mp11::mp_apply<composite_key_equal_to,
                               key_list<CompositeKeyResult, key_equal_to>>;

 public:
  using first_argument_type  = CompositeKeyResult;
  using second_argument_type = first_argument_type;
  using result_type = bool;

  using super::operator();
}; // composite_key_result_equal_to

template<typename CompositeKeyResult>
struct composite_key_result_less
  : private mp11::mp_apply<composite_key_compare,
                           key_list<CompositeKeyResult, key_less>>
{
 private:
  using super = mp11::mp_apply<composite_key_compare,
                               key_list<CompositeKeyResult, key_less>>;

 public:
  using first_argument_type  = CompositeKeyResult;
  using second_argument_type = first_argument_type;
  using result_type = bool;

  using super::operator();
}; // composite_key_result_less

template<typename CompositeKeyResult>
struct composite_key_result_greater
  : private mp11::mp_apply<composite_key_compare,
                           key_list<CompositeKeyResult, key_greater>>
{
 private:
  using super = mp11::mp_apply<composite_key_compare,
                               key_list<CompositeKeyResult, key_greater>>;

 public:
  using first_argument_type  = CompositeKeyResult;
  using second_argument_type = first_argument_type;
  using result_type = bool;

  using super::operator();
}; // composite_key_result_greater

template<typename CompositeKeyResult>
struct composite_key_result_hash
  : private mp11::mp_apply<composite_key_hash,
                           key_list<CompositeKeyResult, key_hash>>
{
 private:
  using super = mp11::mp_apply<composite_key_hash,
                               key_list<CompositeKeyResult, key_hash>>;

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
