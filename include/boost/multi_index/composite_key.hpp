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
  using key_result_type =
                        mp11::mp_transform<result_type_of, key_extractor_list>;
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

template<typename KeyTuple1, typename Value1,
         typename KeyTuple2, typename Value2,
         typename EqualTuple, std::size_t I=0>
struct equal_ckey_ckey;

template<typename KeyTuple1, typename Value1,
         typename KeyTuple2, typename Value2,
         typename EqualTuple, std::size_t I>
struct equal_ckey_ckey_normal {
  static bool compare(const KeyTuple1& kt1, const Value1& v1,
                      const KeyTuple2& kt2, const Value2& v2,
                      const EqualTuple& eq) {
    if (!std::get<I>(eq)(std::get<I>(kt1)(v1), std::get<I>(kt2)(v2)))
      return false;
    return equal_ckey_ckey<KeyTuple1, Value1, KeyTuple2, Value2,
                           EqualTuple, I+1>
        ::compare(kt1, v1, kt2, v2, eq);
  }
}; // equal_ckey_ckey_normal

template<typename KeyTuple1, typename Value1,
         typename KeyTuple2, typename Value2,
         typename EqualTuple>
struct equal_ckey_ckey_terminal {
  static bool compare(const KeyTuple1&, const Value1&,
                      const KeyTuple2&, const Value2&,
                      const EqualTuple&)
  { return true; }
}; // equal_ckey_ckey_terminal

template<typename KeyTuple1, typename Value1,
         typename KeyTuple2, typename Value2,
         typename EqualTuple, std::size_t I>
struct equal_ckey_ckey
  : std::conditional_t<(   I < std::tuple_size_v<KeyTuple1>
                        && I < std::tuple_size_v<KeyTuple2>),
        equal_ckey_ckey_normal<  KeyTuple1, Value1,
                                 KeyTuple2, Value2, EqualTuple, I>,
        equal_ckey_ckey_terminal<KeyTuple1, Value1,
                                 KeyTuple2, Value2, EqualTuple>
    >
{ };

template<typename KeyXTuple, typename Value,
         typename KeyTuple, typename EqualTuple, std::size_t I=0>
struct equal_ckey_cval;

template<typename KeyXTuple, typename Value,
         typename KeyTuple, typename EqualTuple,
         std::size_t I>
struct equal_ckey_cval_normal {
  static bool compare(const KeyXTuple& kxt, const Value& v,
                      const KeyTuple& kt, const EqualTuple& eq) {
    if (!std::get<I>(eq)(std::get<I>(kxt)(v), std::get<I>(kt)))
      return false;
    return equal_ckey_cval<KeyXTuple, Value, KeyTuple, EqualTuple, I+1>
        ::compare(kxt, v, kt, eq);
  }
}; // equal_ckey_cval_normal

template<typename KeyXTuple, typename Value,
         typename KeyTuple, typename EqualTuple>
struct equal_ckey_cval_terminal {
  static bool compare(const KeyXTuple&, const Value&,
                      const KeyTuple, const EqualTuple&)
  { return true; }
}; // equal_ckey_cval_terminal

template<typename KeyXTuple, typename Value,
         typename KeyTuple, typename EqualTuple, std::size_t I>
struct equal_ckey_cval
  : std::conditional_t<(   I < std::tuple_size_v<KeyXTuple>
                        && I < std::tuple_size_v<KeyTuple>),
            equal_ckey_cval_normal<  KeyXTuple, Value, KeyTuple, EqualTuple, I>,
            equal_ckey_cval_terminal<KeyXTuple, Value, KeyTuple, EqualTuple>
    >
{ };

template<typename KeyTuple1, typename Value1,
         typename KeyTuple2, typename Value2,
         typename CompareTuple, std::size_t I=0>
struct compare_ckey_ckey;

template<typename KeyTuple1, typename Value1,
         typename KeyTuple2, typename Value2,
         typename CompareTuple, std::size_t I>
struct compare_ckey_ckey_normal {
  static bool compare(const KeyTuple1& kt1, const Value1& v1,
                      const KeyTuple2& kt2, const Value2& v2,
                      const CompareTuple& ct) {
    const auto& lhs  = std::get<I>(kt1)(v1);
    const auto& rhs  = std::get<I>(kt2)(v2);
    const auto& comp = std::get<I>(ct);
    if (comp(lhs, rhs))
      return true;
    if (comp(rhs, lhs))
      return false;
    return compare_ckey_ckey<KeyTuple1, Value1, KeyTuple2, Value2,
                             CompareTuple, I+1>
        ::compare(kt1, v1, kt2, v2, ct);
  }
}; // compare_ckey_ckey_normal

template<typename KeyTuple1, typename Value1,
         typename KeyTuple2, typename Value2,
         typename CompareTuple>
struct compare_ckey_ckey_terminal {
  static bool compare(const KeyTuple1&, const Value1&,
                      const KeyTuple2&, const Value2&,
                      const CompareTuple&)
  { return false; }
}; // compare_ckey_ckey_terminal

template<typename KeyTuple1, typename Value1,
         typename KeyTuple2, typename Value2,
         typename CompareTuple, std::size_t I>
struct compare_ckey_ckey
  : std::conditional_t<(   I < std::tuple_size_v<KeyTuple1>
                        && I < std::tuple_size_v<KeyTuple2>),
      compare_ckey_ckey_normal<  KeyTuple1, Value1,
                                 KeyTuple2, Value2,
                                 CompareTuple, I>,
      compare_ckey_ckey_terminal<KeyTuple1, Value1,
                                 KeyTuple2, Value2,
                                 CompareTuple>
    >
{ };

template<typename KeyXTuple, typename Value,
         typename KeyTuple,  typename CompareTuple, std::size_t I=0>
struct compare_ckey_cval;

template<typename KeyXTuple, typename Value,
         typename KeyTuple,  typename CompareTuple, std::size_t I>
struct compare_ckey_cval_normal {
  static bool compare(const KeyXTuple& kxt, const Value& v,
                      const KeyTuple& kt, const CompareTuple& ct)
  {
    const auto& lhs  = std::get<I>(kxt)(v);
    const auto& rhs  = std::get<I>(kt);
    const auto& comp = std::get<I>(ct);
    if (comp(lhs, rhs))
      return true;
    if (comp(rhs, lhs))
      return false;
    return compare_ckey_cval<KeyXTuple, Value, KeyTuple, CompareTuple, I+1>
        ::compare(kxt, v, kt, ct);
  }
}; // compare_ckey_cval_normal

template<typename KeyXTuple, typename Value,
         typename KeyTuple,  typename CompareTuple>
struct compare_ckey_cval_terminal {
  static bool compare(const KeyXTuple&, const Value,
                      const KeyTuple&,  const CompareTuple&)
  { return false; }
}; // compare_ckey_cval_terminal

template<typename KeyXTuple, typename Value,
         typename KeyTuple,  typename CompareTuple, std::size_t I>
struct compare_ckey_cval
  : std::conditional_t<(   I < std::tuple_size_v<KeyXTuple>
                        && I < std::tuple_size_v<KeyTuple>),
      compare_ckey_cval_normal<  KeyXTuple, Value, KeyTuple, CompareTuple, I>,
      compare_ckey_cval_terminal<KeyXTuple, Value, KeyTuple, CompareTuple>
    >
{ };

template<typename KeyTuple,
         typename KeyXTuple, typename Value,
         typename CompareTuple, std::size_t I=0>
struct compare_cval_ckey;

template<typename KeyTuple,
         typename KeyXTuple, typename Value,
         typename CompareTuple, std::size_t I>
struct compare_cval_ckey_normal {
  static bool compare(const KeyTuple& kt,
                      const KeyXTuple& kxt, const Value& v,
                      const CompareTuple& ct) {
    const auto& lhs  = std::get<I>(kt);
    const auto& rhs  = std::get<I>(kxt)(v);
    const auto& comp = std::get<I>(ct);
    if (comp(lhs, rhs))
      return true;
    if (comp(rhs, lhs))
      return false;
    return compare_cval_ckey<KeyTuple, KeyXTuple, Value, CompareTuple, I+1>
        ::compare(kt, kxt, v, ct);
  }
}; // compare_cval_ckey_normal

template<typename KeyTuple,
         typename KeyXTuple, typename Value,
         typename CompareTuple>
struct compare_cval_ckey_terminal {
  static bool compare(const KeyTuple&,
                      const KeyXTuple&, const Value&,
                      const CompareTuple&)
  { return false; }
}; // compare_cval_ckey_terminal

template<typename KeyTuple,
         typename KeyXTuple, typename Value,
         typename CompareTuple, std::size_t I>
struct compare_cval_ckey
  : std::conditional_t<(   I < std::tuple_size_v<KeyTuple>
                        && I < std::tuple_size_v<KeyXTuple>),
      compare_cval_ckey_normal<  KeyTuple, KeyXTuple, Value, CompareTuple, I>,
      compare_cval_ckey_terminal<KeyTuple, KeyXTuple, Value, CompareTuple>
    >
{ };

template<typename KeyTuple, typename Value, typename HashTuple, std::size_t I=0>
struct hash_ckey;

template<typename KeyTuple, typename Value, typename HashTuple, std::size_t I>
struct hash_ckey_normal {
  static
  std::size_t hash(const KeyTuple& kt, const Value& v, const HashTuple& h,
                   std::size_t carry=0)
  {
    // same hashing formula as boost::hash_combine
    carry ^= std::get<I>(h)(std::get<I>(kt)(v))
             + 0x9e3779b9 + (carry << 6) + (carry >> 2);

    return hash_ckey<KeyTuple, Value, HashTuple, I+1>
        ::hash(kt, v, h, carry);
  }
}; // hash_ckey_normal

template<typename KeyTuple, typename Value, typename HashTuple>
struct hash_ckey_terminal {
  static
  std::size_t hash(const KeyTuple&, const Value&, const HashTuple&,
                   std::size_t carry=0)
  { return carry; }
}; // hash_ckey_terminal

template<typename KeyTuple, typename Value, typename HashTuple, std::size_t I>
struct hash_ckey
  : std::conditional_t<(I < std::tuple_size_v<KeyTuple>),
      hash_ckey_normal  <KeyTuple, Value, HashTuple, I>,
      hash_ckey_terminal<KeyTuple, Value, HashTuple>
    >
{ };

template<typename ValTuple, typename HashTuple, std::size_t I=0>
struct hash_cval;

template<typename ValTuple, typename HashTuple, std::size_t I>
struct hash_cval_normal {
  static
  std::size_t hash(const ValTuple& vt, const HashTuple& h, std::size_t carry=0)
  {
    // same hashing formula as boost::hash_combine
    carry ^= std::get<I>(h)(std::get<I>(vt)) + 0x9e3779b9 + (carry << 6) + (carry >> 2);
    return hash_cval<ValTuple, HashTuple, I+1>
        ::hash(vt, h, carry);
  }
}; // hash_cval_normal

template<typename ValTuple, typename HashTuple>
struct hash_cval_terminal {
  static
  std::size_t hash(const ValTuple&, const HashTuple&, std::size_t carry=0)
  { return carry; }
}; // hash_cval_terminal

template<typename ValTuple, typename HashTuple, std::size_t I>
struct hash_cval
  : std::conditional_t<(I < std::tuple_size_v<ValTuple>),
      hash_cval_normal  <ValTuple, HashTuple, I>,
      hash_cval_terminal<ValTuple, HashTuple>
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
  { }

  const composite_key_type& composite_key;
  const value_type&         value;
}; // composite_key_result

/* composite_key */

template<typename Value, typename... KeyFromValueList>
struct composite_key
  : private std::tuple<KeyFromValueList...>
{
private:
  using super = std::tuple<KeyFromValueList...>;

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

  static_assert(   std::tuple_size_v<key_extractor_tuple1>
                == std::tuple_size_v<key_extractor_tuple2>);

  using equal_tuple =
      detail::generic_operator_equal_tuple<key_extractor_tuple1>;

  using comparer = detail::equal_ckey_ckey<key_extractor_tuple1, value_type1,
                                           key_extractor_tuple2, value_type2,
                                           equal_tuple>;

 return comparer::compare(x.composite_key.key_extractors(), x.value,
                          y.composite_key.key_extractors(), y.value,
                          equal_tuple());
} // ==

template<typename CompositeKey, typename... Values>
inline bool operator==(const composite_key_result<CompositeKey>& x,
                       const std::tuple<Values...>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = std::tuple<Values...>;

  static_assert(   std::tuple_size_v<key_extractor_tuple>
                == std::tuple_size_v<key_tuple>);

  using equal_tuple = detail::generic_operator_equal_tuple<key_extractor_tuple>;

  using comparer = detail::equal_ckey_cval<key_extractor_tuple, value_type,
                                           key_tuple, equal_tuple>;
  return comparer::compare(x.composite_key.key_extractors(), x.value, y,
                           equal_tuple());
} // ==

template<typename CompositeKey, typename... Values>
inline bool operator==(const std::tuple<Values...>& x,
                       const composite_key_result<CompositeKey>& y)
{ return operator==(y, x); }

template<typename CompositeKey1, typename CompositeKey2>
inline bool operator<(const composite_key_result<CompositeKey1>& x,
                      const composite_key_result<CompositeKey2>& y)
{
  using key_extractor_tuple1 = typename CompositeKey1::key_extractor_tuple;
  using value_type1 = typename CompositeKey1::value_type;
  using key_extractor_tuple2 = typename CompositeKey2::key_extractor_tuple;
  using value_type2 = typename CompositeKey2::value_type;

  using less_tuple = detail::generic_operator_less_tuple<key_extractor_tuple1>;

  using comparer = detail::compare_ckey_ckey<key_extractor_tuple1, value_type1,
                                             key_extractor_tuple2, value_type2,
                                             less_tuple>;

  return comparer::compare(x.composite_key.key_extractors(), x.value,
                           y.composite_key.key_extractors(), y.value,
                           less_tuple());
} // <

template<typename CompositeKey, typename... Values>
inline bool operator<(const composite_key_result<CompositeKey>& x,
                      const std::tuple<Values...>& y)
{
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;
  using key_tuple = std::tuple<Values...>;

  using less_tuple = detail::generic_operator_less_tuple<key_extractor_tuple>;

  using comparer = detail::compare_ckey_cval<key_extractor_tuple, value_type,
                                             key_tuple,
                                             less_tuple>;

  return comparer::compare(x.composite_key.key_extractors(), x.value, y,
                           less_tuple());
} // <

template<typename CompositeKey, typename... Values>
inline bool operator<(const std::tuple<Values...>& x,
                      const composite_key_result<CompositeKey>& y)
{
  using key_tuple = std::tuple<Values...>;
  using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
  using value_type = typename CompositeKey::value_type;

  using less_tuple = detail::generic_operator_less_tuple<key_extractor_tuple>;

  using comparer = detail::compare_cval_ckey<key_tuple,
                                             key_extractor_tuple, value_type,
                                             less_tuple>;

  return comparer::compare(x, y.composite_key.key_extractors(), y.value,
                           less_tuple());
} // <

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

template<typename... PredList>
struct composite_key_equal_to
  : private std::tuple<PredList...>
{
private:
  using super = std::tuple<PredList...>;

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

    static_assert(     std::tuple_size_v<key_extractor_tuple1>
                    <= std::tuple_size_v<key_eq_tuple>
                  &&   std::tuple_size_v<key_extractor_tuple1>
                    == std::tuple_size_v<key_extractor_tuple2>);

    using comparer = detail::equal_ckey_ckey<key_extractor_tuple1, value_type1,
                                             key_extractor_tuple2, value_type2,
                                             key_eq_tuple>;

    return comparer::compare(x.composite_key.key_extractors(), x.value,
                             y.composite_key.key_extractors(), y.value,
                             key_eqs());
  }

  template<typename CompositeKey, typename... Values>
  bool operator()(const composite_key_result<CompositeKey>& x,
                  const std::tuple<Values...>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = std::tuple<Values...>;

    static_assert(     std::tuple_size_v<key_extractor_tuple>
                    <= std::tuple_size_v<key_eq_tuple>
                  &&   std::tuple_size_v<key_extractor_tuple>
                    == std::tuple_size_v<key_tuple>);

    using comparer = detail::equal_ckey_cval<key_extractor_tuple, value_type,
                                             key_tuple, key_eq_tuple>;
    return comparer::compare(x.composite_key.key_extractors(), x.value, y,
                             key_eqs());
  }

  template<typename CompositeKey, typename... Values>
  bool operator()(const std::tuple<Values...>& x,
                  const composite_key_result<CompositeKey>& y) const
  { return operator()(y, x); }

}; // composite_key_equal_to

/* composite_key_compare */

template<typename... CompareList>
struct composite_key_compare
  : private std::tuple<CompareList...>
{
private:
  using super = std::tuple<CompareList...>;

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

    static_assert(     std::tuple_size_v<key_extractor_tuple1>
                    <= std::tuple_size_v<key_comp_tuple>
                  ||   std::tuple_size_v<key_extractor_tuple2>
                    <= std::tuple_size_v<key_comp_tuple>);

    using comparer = detail::compare_ckey_ckey<
                        key_extractor_tuple1, value_type1,
                        key_extractor_tuple2, value_type2,
                        key_comp_tuple>;

    return comparer::compare(x.composite_key.key_extractors(), x.value,
                             y.composite_key.key_extractors(), y.value,
                             key_comps());
  }

  template<typename CompositeKey, typename Value>
  bool operator()(const composite_key_result<CompositeKey>& x, const Value& y)
    const
  { return operator()(x, std::make_tuple(std::cref(y))); }

  template<typename Value, typename CompositeKey>
  bool operator()(const Value& x, const composite_key_result<CompositeKey>& y)
    const
  { return operator()(std::make_tuple(std::cref(x)), y); }

  template<typename CompositeKey, typename... Values>
  bool operator()(const composite_key_result<CompositeKey>& x,
                  const std::tuple<Values...>& y) const
  {
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;
    using key_tuple = std::tuple<Values...>;

    static_assert(   std::tuple_size_v<key_extractor_tuple>
                  <= std::tuple_size_v<key_comp_tuple>
                ||   std::tuple_size_v<key_tuple>
                  <= std::tuple_size_v<key_comp_tuple>);

    using comparer = detail::compare_ckey_cval<key_extractor_tuple, value_type,
                                               key_tuple, key_comp_tuple>;
    return comparer::compare(x.composite_key.key_extractors(), x.value, y,
                             key_comps());
  }

  template<typename CompositeKey, typename... Values>
  bool operator()(const std::tuple<Values...>& x,
                  const composite_key_result<CompositeKey>& y) const
  {
    using key_tuple = std::tuple<Values...>;
    using key_extractor_tuple = typename CompositeKey::key_extractor_tuple;
    using value_type = typename CompositeKey::value_type;

    static_assert(   std::tuple_size_v<key_tuple>
                  <= std::tuple_size_v<key_comp_tuple>
                ||   std::tuple_size_v<key_extractor_tuple>
                  <= std::tuple_size_v<key_comp_tuple>);

    using comparer = detail::compare_cval_ckey<key_tuple,
                                               key_extractor_tuple, value_type,
                                               key_comp_tuple>;
    return comparer::compare(x, y.composite_key.key_extractors(), y.value,
                             key_comps());
  }

}; // composite_key_compare

/* composite_key_hash */

template<typename... HashList>
struct composite_key_hash
  : private std::tuple<HashList...>
{
private:
  using super = std::tuple<HashList...>;

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

    static_assert(   std::tuple_size_v<key_extractor_tuple>
                  == std::tuple_size_v<key_hasher_tuple>);

    using hasher = detail::hash_ckey<key_extractor_tuple, value_type,
                                     key_hasher_tuple>;
    return hasher::hash(x.composite_key.key_extractors(), x.value,
                        key_hash_functions());
  }

  template<typename... Values>
  std::size_t operator()(const std::tuple<Values...>& x) const {
    using key_tuple = std::tuple<Values...>;

    static_assert(  std::tuple_size_v<key_tuple>
                 == std::tuple_size_v<key_hasher_tuple>);

    using hasher = detail::hash_cval<key_tuple, key_hasher_tuple>;
    return hasher::hash(x, key_hash_functions());
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
