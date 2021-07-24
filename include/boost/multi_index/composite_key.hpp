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
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/list/at.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
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

/* This user_definable macro limits the number of elements of a composite
 * key; useful for shortening resulting symbol names (MSVC++ 6.0, for
 * instance has problems coping with very long symbol names.)
 * NB: This cannot exceed the maximum number of arguments of
 * cons_tuple, cons_tuple_size.
 */

#if !defined(BOOST_MULTI_INDEX_LIMIT_COMPOSITE_KEY_SIZE)
#define BOOST_MULTI_INDEX_LIMIT_COMPOSITE_KEY_SIZE 10
#endif

/* maximum number of key extractors in a composite key */

#if BOOST_MULTI_INDEX_LIMIT_COMPOSITE_KEY_SIZE<10 /* max length of a cons_tuple */
#define BOOST_MULTI_INDEX_COMPOSITE_KEY_SIZE \
  BOOST_MULTI_INDEX_LIMIT_COMPOSITE_KEY_SIZE
#else
#define BOOST_MULTI_INDEX_COMPOSITE_KEY_SIZE 10
#endif

/* BOOST_PP_ENUM of BOOST_MULTI_INDEX_COMPOSITE_KEY_SIZE elements */

#define BOOST_MULTI_INDEX_CK_ENUM(macro,data)                                \
  BOOST_PP_ENUM(BOOST_MULTI_INDEX_COMPOSITE_KEY_SIZE,macro,data)

/* BOOST_PP_ENUM_PARAMS of BOOST_MULTI_INDEX_COMPOSITE_KEY_SIZE elements */

#define BOOST_MULTI_INDEX_CK_ENUM_PARAMS(param)                              \
  BOOST_PP_ENUM_PARAMS(BOOST_MULTI_INDEX_COMPOSITE_KEY_SIZE,param)

/* if n==0 ->   text0
 * otherwise -> textn=cons_null
 */

#define BOOST_MULTI_INDEX_CK_TEMPLATE_PARM(z,n,text)                         \
  typename BOOST_PP_CAT(text,n) BOOST_PP_EXPR_IF(n,=cons_null)

/* const textn& kn=textn() */

#define BOOST_MULTI_INDEX_CK_CTOR_ARG(z,n,text)                              \
  const BOOST_PP_CAT(text,n)& BOOST_PP_CAT(k,n) = BOOST_PP_CAT(text,n)()

/* typename list(0)<list(1),n>::type */

#define BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N(z,n,list)                  \
  typename BOOST_PP_LIST_AT(list,0)<                           \
    BOOST_PP_LIST_AT(list,1),n                                               \
  >::type

namespace boost::multi_index {

static_assert(BOOST_MULTI_INDEX_COMPOSITE_KEY_SIZE == cons_tuple_size);

namespace detail {

/* n-th key extractor of a composite key */

template<typename CompositeKey, std::size_t N>
struct nth_key_from_value {
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename mp11::mp_if_c <
    (N < cons_size<key_extractor_tuple>::value),
    cons_element<N, key_extractor_tuple>,
    mp11::mp_identity<cons_null>
  >::type                                            type;
}; // nth_key_from_value

/* nth_composite_key_##name<CompositeKey,N>::type yields
 * functor<nth_key_from_value<CompositeKey,N> >, or cons_null
 * if N exceeds the length of the composite key.
 */

#define BOOST_MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(name,functor)         \
template<typename KeyFromValue>                                              \
struct BOOST_PP_CAT(key_,name)                                               \
{                                                                            \
  typedef functor<typename KeyFromValue::result_type> type;                  \
};                                                                           \
                                                                             \
template<>                                                                   \
struct BOOST_PP_CAT(key_,name)<cons_null>                                    \
{                                                                            \
  typedef cons_null type;                                                    \
};                                                                           \
                                                                             \
template<typename CompositeKey,std::size_t  N>                               \
struct BOOST_PP_CAT(nth_composite_key_,name)                                 \
{                                                                            \
  typedef typename nth_key_from_value<CompositeKey,N>::type key_from_value;  \
  typedef typename BOOST_PP_CAT(key_,name)<key_from_value>::type type;       \
};

/* nth_composite_key_equal_to
 * nth_composite_key_less
 * nth_composite_key_greater
 * nth_composite_key_hash
 */

BOOST_MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(equal_to, std::equal_to)
BOOST_MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(less, std::less)
BOOST_MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(greater, std::greater)
BOOST_MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(hash, std::hash)

/* used for defining equality and comparison ops of composite_key_result */

#define BOOST_MULTI_INDEX_CK_IDENTITY_ENUM_MACRO(z,n,text) text

struct generic_operator_equal {
  template<typename T, typename Q>
  bool operator()(const T& x, const Q& y) const
  { return x == y; }
};

typedef mp11::mp_rename <
  mp11::mp_repeat_c<mp11::mp_list<generic_operator_equal>, cons_tuple_size>,
  cons_tuple
>   generic_operator_equal_tuple;

struct generic_operator_less {
  template<typename T, typename Q>
  bool operator()(const T& x, const Q& y) const
  { return x < y; }
};

typedef mp11::mp_fill<generic_operator_equal_tuple, generic_operator_less>
    generic_operator_less_tuple;

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
  typedef CompositeKey                            composite_key_type;
  typedef typename composite_key_type::value_type value_type;

  composite_key_result(const composite_key_type& composite_key_,
                       const value_type& value_)
    : composite_key(composite_key_), value(value_)
  {}

  const composite_key_type& composite_key;
  const value_type&         value;
}; // composite_key_result

/* composite_key */

template <
  typename Value,
  BOOST_MULTI_INDEX_CK_ENUM(BOOST_MULTI_INDEX_CK_TEMPLATE_PARM, KeyFromValue)
>
struct composite_key
  : private cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(KeyFromValue)>
{
private:
  typedef cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(KeyFromValue)> super;

public:
  typedef super                               key_extractor_tuple;
  typedef Value                               value_type;
  typedef composite_key_result<composite_key> result_type;

  explicit composite_key(
    BOOST_MULTI_INDEX_CK_ENUM(BOOST_MULTI_INDEX_CK_CTOR_ARG, KeyFromValue))
    : super(BOOST_MULTI_INDEX_CK_ENUM_PARAMS(k))
  {}

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
  typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
  typedef typename CompositeKey1::value_type          value_type1;
  typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
  typedef typename CompositeKey2::value_type          value_type2;

  static_assert(   cons_size<key_extractor_tuple1>::value
                == cons_size<key_extractor_tuple2>::value);

  return detail::equal_ckey_ckey <
            key_extractor_tuple1, value_type1,
            key_extractor_tuple2, value_type2,
            detail::generic_operator_equal_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    y.composite_key.key_extractors(), y.value,
                    detail::generic_operator_equal_tuple());
}

template <typename CompositeKey, typename... Values>
inline bool operator==(const composite_key_result<CompositeKey>& x,
                       const cons_tuple<Values...>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef cons_tuple<Values...>                          key_tuple;

  static_assert(   cons_size<key_extractor_tuple>::value
                == cons_size<key_tuple>::value);

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            detail::generic_operator_equal_tuple
         >::compare(x.composite_key.key_extractors(), x.value, y,
                    detail::generic_operator_equal_tuple());
}

template <typename CompositeKey, typename... Values>
inline bool operator==(const cons_tuple<Values...>& x,
                       const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef cons_tuple<Values...>                          key_tuple;

  static_assert(   cons_size<key_extractor_tuple>::value
                == cons_size<key_tuple>::value);

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            detail::generic_operator_equal_tuple
         >::compare(x, y.composite_key.key_extractors(), y.value,
                    detail::generic_operator_equal_tuple());
}

template<typename CompositeKey, typename... Values>
inline bool operator==(const composite_key_result<CompositeKey>& x,
                       const std::tuple<Values...>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor < key_tuple >::result_type
                                                     cons_key_tuple;

  static_assert(cons_size<key_extractor_tuple>::value
                == std::tuple_size<key_tuple>::value);

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            cons_key_tuple,
            detail::generic_operator_equal_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    detail::make_cons_stdtuple(y),
                    detail::generic_operator_equal_tuple());
}

template<typename CompositeKey, typename... Values>
inline bool operator==(const std::tuple<Values...>& x,
                       const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor <key_tuple >::result_type
                                                     cons_key_tuple;

  static_assert(cons_size<key_extractor_tuple>::value
                == std::tuple_size<key_tuple>::value);

  return detail::equal_ckey_cval <
            key_extractor_tuple, value_type,
            cons_key_tuple,
            detail::generic_operator_equal_tuple
         >::compare(detail::make_cons_stdtuple(x),
                    y.composite_key.key_extractors(),
                    y.value,
                    detail::generic_operator_equal_tuple());
}

/* < */

template<typename CompositeKey1, typename CompositeKey2>
inline bool operator<(const composite_key_result<CompositeKey1>& x,
                      const composite_key_result<CompositeKey2>& y)
{
  typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
  typedef typename CompositeKey1::value_type          value_type1;
  typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
  typedef typename CompositeKey2::value_type          value_type2;

  return detail::compare_ckey_ckey <
            key_extractor_tuple1, value_type1,
            key_extractor_tuple2, value_type2,
            detail::generic_operator_less_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    y.composite_key.key_extractors(), y.value,
                    detail::generic_operator_less_tuple());
}

template <
  typename CompositeKey,
  BOOST_MULTI_INDEX_CK_ENUM_PARAMS(typename Value)
>
inline bool operator<(
  const composite_key_result<CompositeKey>& x,
  const cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)> key_tuple;

  return detail::compare_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            detail::generic_operator_less_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    y,
                    detail::generic_operator_less_tuple());
}

template <typename CompositeKey, typename... Values>
inline bool operator<(const cons_tuple<Values...>& x,
                      const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef cons_tuple<Values...>                          key_tuple;

  return detail::compare_ckey_cval <
            key_extractor_tuple, value_type,
            key_tuple,
            detail::generic_operator_less_tuple
         >::compare(x, y.composite_key.key_extractors(),
                    y.value,
                    detail::generic_operator_less_tuple());
}

template<typename CompositeKey, typename... Values>
inline bool operator<(const composite_key_result<CompositeKey>& x,
                      const std::tuple<Values...>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor<key_tuple >::result_type
                                                     cons_key_tuple;
  return detail::compare_ckey_cval <
            key_extractor_tuple, value_type,
            cons_key_tuple,
            detail::generic_operator_less_tuple
         >::compare(x.composite_key.key_extractors(), x.value,
                    detail::make_cons_stdtuple(y),
                    detail::generic_operator_less_tuple());
}

template<typename CompositeKey, typename... Values>
inline bool operator<(const std::tuple<Values...>& x,
                      const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor< key_tuple>::result_type
                                                     cons_key_tuple;

  return detail::compare_ckey_cval <
          key_extractor_tuple, value_type,
          cons_key_tuple,
          detail::generic_operator_less_tuple
         >::compare(detail::make_cons_stdtuple(x),
                    y.composite_key.key_extractors(),
                    y.value,
                    detail::generic_operator_less_tuple());
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
  BOOST_MULTI_INDEX_CK_ENUM_PARAMS(typename Value),
  composite_key_result<CompositeKey>,
  cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)>
)

BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  BOOST_MULTI_INDEX_CK_ENUM_PARAMS(typename Value),
  typename CompositeKey,
  cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)>,
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

/* composite_key_equal_to */

template <typename... PredList>
struct composite_key_equal_to
  : private cons_tuple<PredList...>
{
private:
  typedef cons_tuple<PredList...> super;

public:
  typedef super key_eq_tuple;

  composite_key_equal_to() : super() { }

  explicit composite_key_equal_to(const PredList&... args) : super(args...) {}

  composite_key_equal_to(const key_eq_tuple& x) : super(x) {}

  const key_eq_tuple& key_eqs() const { return *this; }
  key_eq_tuple&       key_eqs()       { return *this; }

  template<typename CompositeKey1, typename CompositeKey2>
  bool operator()(const composite_key_result<CompositeKey1>& x,
                  const composite_key_result<CompositeKey2>& y) const
  {
    typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
    typedef typename CompositeKey1::value_type          value_type1;
    typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
    typedef typename CompositeKey2::value_type          value_type2;

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

  template <
    typename CompositeKey,
    BOOST_MULTI_INDEX_CK_ENUM_PARAMS(typename Value)
  >
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)>& y) const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)> key_tuple;

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

  template <
    BOOST_MULTI_INDEX_CK_ENUM_PARAMS(typename Value),
    typename CompositeKey
  >
  bool operator()(
    const cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)>& x,
    const composite_key_result<CompositeKey>& y) const {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)> key_tuple;

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
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<key_tuple >::result_type
                                                       cons_key_tuple;

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
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<key_tuple>::result_type
                                                       cons_key_tuple;

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
  typedef cons_tuple<CompareList...> super;

public:
  typedef super key_comp_tuple;

  composite_key_compare() : super() {}

  explicit composite_key_compare(const CompareList&... args) : super(args...) {}

  composite_key_compare(const key_comp_tuple& x) : super(x) {}

  const key_comp_tuple& key_comps() const { return *this; }
  key_comp_tuple&       key_comps()       { return *this; }

  template<typename CompositeKey1, typename CompositeKey2>
  bool operator()(const composite_key_result<CompositeKey1>& x,
                  const composite_key_result<CompositeKey2>& y)
  const {
    typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
    typedef typename CompositeKey1::value_type          value_type1;
    typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
    typedef typename CompositeKey2::value_type          value_type2;

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

  template <
    typename CompositeKey,
    BOOST_MULTI_INDEX_CK_ENUM_PARAMS(typename Value)
  >
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)>& y) const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)> key_tuple;

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

  template <
    typename CompositeKey,
    BOOST_MULTI_INDEX_CK_ENUM_PARAMS(typename Value)
  >
  bool operator()(
    const cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)>& x,
    const composite_key_result<CompositeKey>& y) const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef cons_tuple<BOOST_MULTI_INDEX_CK_ENUM_PARAMS(Value)> key_tuple;

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
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<key_tuple >::result_type
                                                       cons_key_tuple;

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
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<key_tuple >::result_type
                                                       cons_key_tuple;

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
  typedef cons_tuple<HashList...> super;

public:
  typedef super key_hasher_tuple;

  composite_key_hash() : super() { }

  explicit composite_key_hash(const HashList&... args...) : super(args...) {}

  composite_key_hash(const key_hasher_tuple& x) : super(x) {}

  const key_hasher_tuple& key_hash_functions() const { return *this; }

  key_hasher_tuple&       key_hash_functions()       { return *this; }

  template<typename CompositeKey>
  std::size_t operator()(const composite_key_result<CompositeKey>& x) const {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;

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
    typedef cons_tuple<Values...> key_tuple;

    static_assert(   cons_size<key_tuple>::value
                  == cons_size<key_hasher_tuple>::value);

    return detail::hash_cval <
              key_tuple,
              key_hasher_tuple
           >::hash(x, key_hash_functions());
  }

  template<typename... Values>
  std::size_t operator()(const std::tuple<Values...>& x) const {
    typedef std::tuple<Values...>                key_tuple;
    typedef typename detail::cons_stdtuple_ctor<key_tuple>::result_type
                                                 cons_key_tuple;

    static_assert(std::tuple_size<key_tuple>::value
              == cons_size<key_hasher_tuple>::value);

    return detail::hash_cval <
              cons_key_tuple,
              key_hasher_tuple
           >::hash(detail::make_cons_stdtuple(x), key_hash_functions());
  }
}; // composite_key_hash

namespace detail {

/* Instantiations of the former functors with "natural" basic components:
 * composite_key_result_equal_to uses std::equal_to of the values.
 * composite_key_result_less     uses std::less.
 * composite_key_result_greater  uses std::greater.
 * composite_key_result_hash     uses std::hash.
 */

#define BOOST_MULTI_INDEX_CK_RESULT_EQUAL_TO_SUPER                           \
composite_key_equal_to<                                                      \
    BOOST_MULTI_INDEX_CK_ENUM(                                               \
      BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N,                             \
      /* the argument is a PP list */                                        \
      (detail::nth_composite_key_equal_to,                                   \
        (typename CompositeKeyResult::composite_key_type,                    \
          BOOST_PP_NIL)))                                                    \
  >

template<typename CompositeKeyResult>
struct composite_key_result_equal_to
  : private BOOST_MULTI_INDEX_CK_RESULT_EQUAL_TO_SUPER
{
private:
  typedef BOOST_MULTI_INDEX_CK_RESULT_EQUAL_TO_SUPER super;

public:
  typedef CompositeKeyResult  first_argument_type;
  typedef first_argument_type second_argument_type;
  typedef bool                result_type;

  using super::operator();
}; // composite_key_result_equal_to

#define BOOST_MULTI_INDEX_CK_RESULT_LESS_SUPER                               \
composite_key_compare<                                                       \
    BOOST_MULTI_INDEX_CK_ENUM(                                               \
      BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N,                             \
      /* the argument is a PP list */                                        \
      (detail::nth_composite_key_less,                                       \
        (typename CompositeKeyResult::composite_key_type,                    \
          BOOST_PP_NIL)))                                                    \
  >

template<typename CompositeKeyResult>
struct composite_key_result_less
  : private BOOST_MULTI_INDEX_CK_RESULT_LESS_SUPER
{
private:
  typedef BOOST_MULTI_INDEX_CK_RESULT_LESS_SUPER super;

public:
  typedef CompositeKeyResult  first_argument_type;
  typedef first_argument_type second_argument_type;
  typedef bool                result_type;

  using super::operator();
}; // composite_key_result_less

#define BOOST_MULTI_INDEX_CK_RESULT_GREATER_SUPER                            \
composite_key_compare<                                                       \
    BOOST_MULTI_INDEX_CK_ENUM(                                               \
      BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N,                             \
      /* the argument is a PP list */                                        \
      (detail::nth_composite_key_greater,                                    \
        (typename CompositeKeyResult::composite_key_type,                    \
          BOOST_PP_NIL)))                                                    \
  >

template<typename CompositeKeyResult>
struct composite_key_result_greater
  : private BOOST_MULTI_INDEX_CK_RESULT_GREATER_SUPER
{
private:
  typedef BOOST_MULTI_INDEX_CK_RESULT_GREATER_SUPER super;

public:
  typedef CompositeKeyResult  first_argument_type;
  typedef first_argument_type second_argument_type;
  typedef bool                result_type;

  using super::operator();
}; // composite_key_result_greater

#define BOOST_MULTI_INDEX_CK_RESULT_HASH_SUPER                               \
composite_key_hash<                                                          \
    BOOST_MULTI_INDEX_CK_ENUM(                                               \
      BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N,                             \
      /* the argument is a PP list */                                        \
      (detail::nth_composite_key_hash,                                       \
        (typename CompositeKeyResult::composite_key_type,                    \
          BOOST_PP_NIL)))                                                    \
  >

template<typename CompositeKeyResult>
struct composite_key_result_hash
  : private BOOST_MULTI_INDEX_CK_RESULT_HASH_SUPER
{
private:
  typedef BOOST_MULTI_INDEX_CK_RESULT_HASH_SUPER super;

public:
  typedef CompositeKeyResult argument_type;
  typedef std::size_t        result_type;

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

#undef BOOST_MULTI_INDEX_CK_RESULT_HASH_SUPER
#undef BOOST_MULTI_INDEX_CK_RESULT_GREATER_SUPER
#undef BOOST_MULTI_INDEX_CK_RESULT_LESS_SUPER
#undef BOOST_MULTI_INDEX_CK_RESULT_EQUAL_TO_SUPER
#undef BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS
#undef BOOST_MULTI_INDEX_CK_IDENTITY_ENUM_MACRO
#undef BOOST_MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR
#undef BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N
#undef BOOST_MULTI_INDEX_CK_CTOR_ARG
#undef BOOST_MULTI_INDEX_CK_TEMPLATE_PARM
#undef BOOST_MULTI_INDEX_CK_ENUM_PARAMS
#undef BOOST_MULTI_INDEX_CK_ENUM
#undef BOOST_MULTI_INDEX_COMPOSITE_KEY_SIZE

#endif
