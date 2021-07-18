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
#include <boost/mp11/function.hpp>
#include <boost/mp11/utility.hpp>
#include <boost/tuple/tuple.hpp>
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
 * boost::tuple. In Boost 1.32, the limit is 10.
 */

#if !defined(BOOST_MULTI_INDEX_LIMIT_COMPOSITE_KEY_SIZE)
#define BOOST_MULTI_INDEX_LIMIT_COMPOSITE_KEY_SIZE 10
#endif

/* maximum number of key extractors in a composite key */

#if BOOST_MULTI_INDEX_LIMIT_COMPOSITE_KEY_SIZE<10 /* max length of a boost::tuple */
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
 * otherwise -> textn=boost::tuples::null_type
 */

#define BOOST_MULTI_INDEX_CK_TEMPLATE_PARM(z,n,text)                         \
  typename BOOST_PP_CAT(text,n) BOOST_PP_EXPR_IF(n,=boost::tuples::null_type)

/* const textn& kn=textn() */

#define BOOST_MULTI_INDEX_CK_CTOR_ARG(z,n,text)                              \
  const BOOST_PP_CAT(text,n)& BOOST_PP_CAT(k,n) = BOOST_PP_CAT(text,n)()

/* typename list(0)<list(1),n>::type */

#define BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N(z,n,list)                  \
  typename BOOST_PP_LIST_AT(list,0)<                           \
    BOOST_PP_LIST_AT(list,1),n                                               \
  >::type

namespace boost::multi_index{

namespace detail{

/* n-th key extractor of a composite key */

template<typename CompositeKey,int N>
struct nth_key_from_value
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename mp11::mp_if_c<
    N<boost::tuples::length<key_extractor_tuple>::value,
    boost::tuples::element<N,key_extractor_tuple>,
    mp11::mp_identity<boost::tuples::null_type>
  >::type                                            type;
};

/* nth_composite_key_##name<CompositeKey,N>::type yields
 * functor<nth_key_from_value<CompositeKey,N> >, or boost::tuples::null_type
 * if N exceeds the length of the composite key.
 */

/* nth_composite_key_equal_to
 * nth_composite_key_less
 * nth_composite_key_greater
 * nth_composite_key_hash
 */

template<typename KeyFromValue>
struct key_equal_to {
  typedef std::equal_to<typename KeyFromValue::result_type> type;
};

template<>
struct key_equal_to<boost::tuples::null_type> {
  typedef boost::tuples::null_type type;
};

template<typename CompositeKey,int N>
struct nth_composite_key_equal_to {
  typedef typename nth_key_from_value<CompositeKey,N>::type key_from_value;
  typedef typename key_equal_to<key_from_value>::type type;
};

template<typename KeyFromValue>
struct key_less {
  typedef std::less<typename KeyFromValue::result_type> type;
};

template<> struct key_less<boost::tuples::null_type> {
  typedef boost::tuples::null_type type;
};

template<typename CompositeKey,int N>
struct nth_composite_key_less {
  typedef typename nth_key_from_value<CompositeKey,N>::type key_from_value;
  typedef typename key_less<key_from_value>::type type;
};

template<typename KeyFromValue>
struct key_greater {
  typedef std::greater<typename KeyFromValue::result_type> type;
};

template<> struct key_greater<boost::tuples::null_type> {
  typedef boost::tuples::null_type type;
};

template<typename CompositeKey,int N>
struct nth_composite_key_greater {
  typedef typename nth_key_from_value<CompositeKey,N>::type key_from_value;
  typedef typename key_greater<key_from_value>::type type;
};

template<typename KeyFromValue>
struct key_hash {
  typedef std::hash<typename KeyFromValue::result_type> type;
};

template<> struct key_hash<boost::tuples::null_type> {
  typedef boost::tuples::null_type type;
};

template<typename CompositeKey,int N>
struct nth_composite_key_hash {
  typedef typename nth_key_from_value<CompositeKey,N>::type key_from_value;
  typedef typename key_hash<key_from_value>::type type;
};

/* used for defining equality and comparison ops of composite_key_result */

#define BOOST_MULTI_INDEX_CK_IDENTITY_ENUM_MACRO(z,n,text) text

struct generic_operator_equal
{
  template<typename T,typename Q>
  bool operator()(const T& x,const Q& y)const{return x==y;}
};

typedef boost::tuple<
  detail::generic_operator_equal,
  detail::generic_operator_equal,
  detail::generic_operator_equal,
  detail::generic_operator_equal,
  detail::generic_operator_equal,
  detail::generic_operator_equal,
  detail::generic_operator_equal,
  detail::generic_operator_equal,
  detail::generic_operator_equal,
  detail::generic_operator_equal
> generic_operator_equal_tuple;

struct generic_operator_less
{
  template<typename T,typename Q>
  bool operator()(const T& x,const Q& y)const{return x<y;}
};

typedef boost::tuple<
  detail::generic_operator_less,
  detail::generic_operator_less,
  detail::generic_operator_less,
  detail::generic_operator_less,
  detail::generic_operator_less,
  detail::generic_operator_less,
  detail::generic_operator_less,
  detail::generic_operator_less,
  detail::generic_operator_less,
  detail::generic_operator_less
> generic_operator_less_tuple;

/* Metaprogramming machinery for implementing equality, comparison and
 * hashing operations of composite_key_result.
 *
 * equal_* checks for equality between composite_key_results and
 * between those and tuples, accepting a boost::tuple of basic equality functors.
 * compare_* does lexicographical comparison.
 * hash_* computes a combination of elementwise hash values.
 */

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename EqualCons
>
struct equal_ckey_ckey; /* fwd decl. */

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename EqualCons
>
struct equal_ckey_ckey_terminal
{
  static bool compare(
    const KeyCons1&,const Value1&,
    const KeyCons2&,const Value2&,
    const EqualCons&)
  {
    return true;
  }
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename EqualCons
>
struct equal_ckey_ckey_normal
{
  static bool compare(
    const KeyCons1& c0,const Value1& v0,
    const KeyCons2& c1,const Value2& v1,
    const EqualCons& eq)
  {
    if(!eq.get_head()(c0.get_head()(v0),c1.get_head()(v1)))return false;
    return equal_ckey_ckey<
      typename KeyCons1::tail_type,Value1,
      typename KeyCons2::tail_type,Value2,
      typename EqualCons::tail_type
    >::compare(c0.get_tail(),v0,c1.get_tail(),v1,eq.get_tail());
  }
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename EqualCons
>
struct equal_ckey_ckey:
  mp11::mp_if<
    mp11::mp_or<
      std::is_same<KeyCons1,boost::tuples::null_type>,
      std::is_same<KeyCons2,boost::tuples::null_type>
    >,
    equal_ckey_ckey_terminal<KeyCons1,Value1,KeyCons2,Value2,EqualCons>,
    equal_ckey_ckey_normal<KeyCons1,Value1,KeyCons2,Value2,EqualCons>
  >
{
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename EqualCons
>
struct equal_ckey_cval; /* fwd decl. */

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename EqualCons
>
struct equal_ckey_cval_terminal
{
  static bool compare(
    const KeyCons&,const Value&,const ValCons&,const EqualCons&)
  {
    return true;
  }

  static bool compare(
    const ValCons&,const KeyCons&,const Value&,const EqualCons&)
  {
    return true;
  }
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename EqualCons
>
struct equal_ckey_cval_normal
{
  static bool compare(
    const KeyCons& c,const Value& v,const ValCons& vc,
    const EqualCons& eq)
  {
    if(!eq.get_head()(c.get_head()(v),vc.get_head()))return false;
    return equal_ckey_cval<
      typename KeyCons::tail_type,Value,
      typename ValCons::tail_type,
      typename EqualCons::tail_type
    >::compare(c.get_tail(),v,vc.get_tail(),eq.get_tail());
  }

  static bool compare(
    const ValCons& vc,const KeyCons& c,const Value& v,
    const EqualCons& eq)
  {
    if(!eq.get_head()(vc.get_head(),c.get_head()(v)))return false;
    return equal_ckey_cval<
      typename KeyCons::tail_type,Value,
      typename ValCons::tail_type,
      typename EqualCons::tail_type
    >::compare(vc.get_tail(),c.get_tail(),v,eq.get_tail());
  }
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename EqualCons
>
struct equal_ckey_cval:
  mp11::mp_if<
    mp11::mp_or<
      std::is_same<KeyCons,boost::tuples::null_type>,
      std::is_same<ValCons,boost::tuples::null_type>
    >,
    equal_ckey_cval_terminal<KeyCons,Value,ValCons,EqualCons>,
    equal_ckey_cval_normal<KeyCons,Value,ValCons,EqualCons>
  >
{
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename CompareCons
>
struct compare_ckey_ckey; /* fwd decl. */

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename CompareCons
>
struct compare_ckey_ckey_terminal
{
  static bool compare(
    const KeyCons1&,const Value1&,
    const KeyCons2&,const Value2&,
    const CompareCons&)
  {
    return false;
  }
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename CompareCons
>
struct compare_ckey_ckey_normal
{
  static bool compare(
    const KeyCons1& c0,const Value1& v0,
    const KeyCons2& c1,const Value2& v1,
    const CompareCons& comp)
  {
    if(comp.get_head()(c0.get_head()(v0),c1.get_head()(v1)))return true;
    if(comp.get_head()(c1.get_head()(v1),c0.get_head()(v0)))return false;
    return compare_ckey_ckey<
      typename KeyCons1::tail_type,Value1,
      typename KeyCons2::tail_type,Value2,
      typename CompareCons::tail_type
    >::compare(c0.get_tail(),v0,c1.get_tail(),v1,comp.get_tail());
  }
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename CompareCons
>
struct compare_ckey_ckey:
  mp11::mp_if<
    mp11::mp_or<
      std::is_same<KeyCons1,boost::tuples::null_type>,
      std::is_same<KeyCons2,boost::tuples::null_type>
    >,
    compare_ckey_ckey_terminal<KeyCons1,Value1,KeyCons2,Value2,CompareCons>,
    compare_ckey_ckey_normal<KeyCons1,Value1,KeyCons2,Value2,CompareCons>
  >
{
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename CompareCons
>
struct compare_ckey_cval; /* fwd decl. */

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename CompareCons
>
struct compare_ckey_cval_terminal
{
  static bool compare(
    const KeyCons&,const Value&,const ValCons&,const CompareCons&)
  {
    return false;
  }

  static bool compare(
    const ValCons&,const KeyCons&,const Value&,const CompareCons&)
  {
    return false;
  }
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename CompareCons
>
struct compare_ckey_cval_normal
{
  static bool compare(
    const KeyCons& c,const Value& v,const ValCons& vc,
    const CompareCons& comp)
  {
    if(comp.get_head()(c.get_head()(v),vc.get_head()))return true;
    if(comp.get_head()(vc.get_head(),c.get_head()(v)))return false;
    return compare_ckey_cval<
      typename KeyCons::tail_type,Value,
      typename ValCons::tail_type,
      typename CompareCons::tail_type
    >::compare(c.get_tail(),v,vc.get_tail(),comp.get_tail());
  }

  static bool compare(
    const ValCons& vc,const KeyCons& c,const Value& v,
    const CompareCons& comp)
  {
    if(comp.get_head()(vc.get_head(),c.get_head()(v)))return true;
    if(comp.get_head()(c.get_head()(v),vc.get_head()))return false;
    return compare_ckey_cval<
      typename KeyCons::tail_type,Value,
      typename ValCons::tail_type,
      typename CompareCons::tail_type
    >::compare(vc.get_tail(),c.get_tail(),v,comp.get_tail());
  }
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename CompareCons
>
struct compare_ckey_cval:
  mp11::mp_if<
    mp11::mp_or<
      std::is_same<KeyCons,boost::tuples::null_type>,
      std::is_same<ValCons,boost::tuples::null_type>
    >,
    compare_ckey_cval_terminal<KeyCons,Value,ValCons,CompareCons>,
    compare_ckey_cval_normal<KeyCons,Value,ValCons,CompareCons>
  >
{
};

template<typename KeyCons,typename Value,typename HashCons>
struct hash_ckey; /* fwd decl. */

template<typename KeyCons,typename Value,typename HashCons>
struct hash_ckey_terminal
{
  static std::size_t hash(
    const KeyCons&,const Value&,const HashCons&,std::size_t carry)
  {
    return carry;
  }
};

template<typename KeyCons,typename Value,typename HashCons>
struct hash_ckey_normal
{
  static std::size_t hash(
    const KeyCons& c,const Value& v,const HashCons& h,std::size_t carry=0)
  {
    /* same hashing formula as boost::hash_combine */

    carry^=h.get_head()(c.get_head()(v))+0x9e3779b9+(carry<<6)+(carry>>2);
    return hash_ckey<
      typename KeyCons::tail_type,Value,
      typename HashCons::tail_type
    >::hash(c.get_tail(),v,h.get_tail(),carry);
  }
};

template<typename KeyCons,typename Value,typename HashCons>
struct hash_ckey:
  mp11::mp_if<
    std::is_same<KeyCons,boost::tuples::null_type>,
    hash_ckey_terminal<KeyCons,Value,HashCons>,
    hash_ckey_normal<KeyCons,Value,HashCons>
  >
{
};

template<typename ValCons,typename HashCons>
struct hash_cval; /* fwd decl. */

template<typename ValCons,typename HashCons>
struct hash_cval_terminal
{
  static std::size_t hash(const ValCons&,const HashCons&,std::size_t carry)
  {
    return carry;
  }
};

template<typename ValCons,typename HashCons>
struct hash_cval_normal
{
  static std::size_t hash(
    const ValCons& vc,const HashCons& h,std::size_t carry=0)
  {
    carry^=h.get_head()(vc.get_head())+0x9e3779b9+(carry<<6)+(carry>>2);
    return hash_cval<
      typename ValCons::tail_type,
      typename HashCons::tail_type
    >::hash(vc.get_tail(),h.get_tail(),carry);
  }
};

template<typename ValCons,typename HashCons>
struct hash_cval:
  mp11::mp_if<
    std::is_same<ValCons,boost::tuples::null_type>,
    hash_cval_terminal<ValCons,HashCons>,
    hash_cval_normal<ValCons,HashCons>
  >
{
};

} // detail

/* composite_key_result */

template<typename CompositeKey>
struct composite_key_result
{
  typedef CompositeKey                            composite_key_type;
  typedef typename composite_key_type::value_type value_type;

  composite_key_result(
    const composite_key_type& composite_key_,const value_type& value_):
    composite_key(composite_key_),value(value_)
  {}

  const composite_key_type& composite_key;
  const value_type&         value;
};

/* composite_key */

template<
  typename Value,
  typename KeyFromValue0,
  typename KeyFromValue1=boost::tuples::null_type,
  typename KeyFromValue2=boost::tuples::null_type,
  typename KeyFromValue3=boost::tuples::null_type,
  typename KeyFromValue4=boost::tuples::null_type,
  typename KeyFromValue5=boost::tuples::null_type,
  typename KeyFromValue6=boost::tuples::null_type,
  typename KeyFromValue7=boost::tuples::null_type,
  typename KeyFromValue8=boost::tuples::null_type,
  typename KeyFromValue9=boost::tuples::null_type
>
struct composite_key:
  private boost::tuple<
    KeyFromValue0, KeyFromValue1, KeyFromValue2, KeyFromValue3, KeyFromValue4,
    KeyFromValue5, KeyFromValue6, KeyFromValue7, KeyFromValue8, KeyFromValue9
  >
{
private:
  typedef boost::tuple<
    KeyFromValue0, KeyFromValue1, KeyFromValue2, KeyFromValue3, KeyFromValue4,
    KeyFromValue5, KeyFromValue6, KeyFromValue7, KeyFromValue8, KeyFromValue9
  > super;

public:
  typedef super                               key_extractor_tuple;
  typedef Value                               value_type;
  typedef composite_key_result<composite_key> result_type;

  composite_key(const KeyFromValue0& k0 = KeyFromValue0(),
                const KeyFromValue1& k1 = KeyFromValue1(),
                const KeyFromValue2& k2 = KeyFromValue2(),
                const KeyFromValue3& k3 = KeyFromValue3(),
                const KeyFromValue4& k4 = KeyFromValue4(),
                const KeyFromValue5& k5 = KeyFromValue5(),
                const KeyFromValue6& k6 = KeyFromValue6(),
                const KeyFromValue7& k7 = KeyFromValue7(),
                const KeyFromValue8& k8 = KeyFromValue8(),
                const KeyFromValue9& k9 = KeyFromValue9())
      : super(k0, k1, k2, k3, k4, k5, k6, k7, k8, k9)
  {}

  composite_key(const key_extractor_tuple& x):super(x){}

  const key_extractor_tuple& key_extractors()const{return *this;}
  key_extractor_tuple&       key_extractors(){return *this;}

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<const ChainedPtr&,const value_type&>,result_type>
  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  result_type operator()(const value_type& x)const
  {
    return result_type(*this,x);
  }

  result_type operator()(const std::reference_wrapper<const value_type>& x)const
  {
    return result_type(*this,x.get());
  }

  result_type operator()(const std::reference_wrapper<value_type>& x)const
  {
    return result_type(*this,x.get());
  }
};

/* comparison operators */

/* == */

template<typename CompositeKey1,typename CompositeKey2>
inline bool operator==(
  const composite_key_result<CompositeKey1>& x,
  const composite_key_result<CompositeKey2>& y)
{
  typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
  typedef typename CompositeKey1::value_type          value_type1;
  typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
  typedef typename CompositeKey2::value_type          value_type2;

  static_assert(
    boost::tuples::length<key_extractor_tuple1>::value==
    boost::tuples::length<key_extractor_tuple2>::value);

  return detail::equal_ckey_ckey<
    key_extractor_tuple1,value_type1,
    key_extractor_tuple2,value_type2,
    detail::generic_operator_equal_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    y.composite_key.key_extractors(),y.value,
    detail::generic_operator_equal_tuple());
}

template<
  typename CompositeKey,
  typename Value0, typename Value1,
  typename Value2, typename Value3,
  typename Value4, typename Value5,
  typename Value6, typename Value7,
  typename Value8, typename Value9
>
inline bool operator==(
  const composite_key_result<CompositeKey>& x,
  const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                     Value5, Value6, Value7, Value8, Value9>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef boost::tuple<Value0, Value1, Value2, Value3, Value4,
                       Value5, Value6, Value7, Value8, Value9>  key_tuple;

  static_assert(
    boost::tuples::length<key_extractor_tuple>::value==
    boost::tuples::length<key_tuple>::value);

  return detail::equal_ckey_cval<
    key_extractor_tuple,value_type,
    key_tuple,detail::generic_operator_equal_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    y,detail::generic_operator_equal_tuple());
}

template <
  typename Value0, typename Value1,
  typename Value2, typename Value3,
  typename Value4, typename Value5,
  typename Value6, typename Value7,
  typename Value8, typename Value9,
  typename CompositeKey
>
inline bool operator==(
  const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                     Value5, Value6, Value7, Value8, Value9>& x,
  const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef boost::tuple<Value0, Value1, Value2, Value3, Value4,
                       Value5, Value6, Value7, Value8, Value9>  key_tuple;

  static_assert(
    boost::tuples::length<key_extractor_tuple>::value==
    boost::tuples::length<key_tuple>::value);

  return detail::equal_ckey_cval<
    key_extractor_tuple,value_type,
    key_tuple,detail::generic_operator_equal_tuple
  >::compare(
    x,y.composite_key.key_extractors(),
    y.value,detail::generic_operator_equal_tuple());
}

template<typename CompositeKey,typename... Values>
inline bool operator==(
  const composite_key_result<CompositeKey>& x,
  const std::tuple<Values...>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor<
    key_tuple>::result_type                          cons_key_tuple;

  static_assert(
    static_cast<std::size_t>(boost::tuples::length<key_extractor_tuple>::value)==
    std::tuple_size<key_tuple>::value);

  return detail::equal_ckey_cval<
    key_extractor_tuple,value_type,
    cons_key_tuple,detail::generic_operator_equal_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    detail::make_cons_stdtuple(y),detail::generic_operator_equal_tuple());
}

template<typename CompositeKey,typename... Values>
inline bool operator==(
  const std::tuple<Values...>& x,
  const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor<
    key_tuple>::result_type                          cons_key_tuple;

  static_assert(
    static_cast<std::size_t>(boost::tuples::length<key_extractor_tuple>::value)==
    std::tuple_size<key_tuple>::value);

  return detail::equal_ckey_cval<
    key_extractor_tuple,value_type,
    cons_key_tuple,detail::generic_operator_equal_tuple
  >::compare(
    detail::make_cons_stdtuple(x),y.composite_key.key_extractors(),
    y.value,detail::generic_operator_equal_tuple());
}

/* < */

template<typename CompositeKey1,typename CompositeKey2>
inline bool operator<(
  const composite_key_result<CompositeKey1>& x,
  const composite_key_result<CompositeKey2>& y)
{
  typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
  typedef typename CompositeKey1::value_type          value_type1;
  typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
  typedef typename CompositeKey2::value_type          value_type2;

  return detail::compare_ckey_ckey<
   key_extractor_tuple1,value_type1,
   key_extractor_tuple2,value_type2,
   detail::generic_operator_less_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    y.composite_key.key_extractors(),y.value,
    detail::generic_operator_less_tuple());
}

template <
  typename CompositeKey,
  typename Value0, typename Value1,
  typename Value2, typename Value3,
  typename Value4, typename Value5,
  typename Value6, typename Value7,
  typename Value8, typename Value9
>
inline bool operator<(const composite_key_result<CompositeKey>& x,
                const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                                   Value5, Value6, Value7, Value8, Value9>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef boost::tuple<Value0, Value1, Value2, Value3, Value4,
                       Value5, Value6, Value7, Value8, Value9>  key_tuple;

  return detail::compare_ckey_cval<
    key_extractor_tuple,value_type,
    key_tuple,detail::generic_operator_less_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    y,detail::generic_operator_less_tuple());
}

template <
  typename Value0, typename Value1,
  typename Value2, typename Value3,
  typename Value4, typename Value5,
  typename Value6, typename Value7,
  typename Value8, typename Value9,
  typename CompositeKey
>
inline bool operator<(
    const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                       Value5, Value6, Value7, Value8, Value9>& x,
  const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef boost::tuple<Value0, Value1, Value2, Value3, Value4,
                       Value5, Value6, Value7, Value8, Value9>  key_tuple;

  return detail::compare_ckey_cval<
    key_extractor_tuple,value_type,
    key_tuple,detail::generic_operator_less_tuple
  >::compare(
    x,y.composite_key.key_extractors(),
    y.value,detail::generic_operator_less_tuple());
}

template<typename CompositeKey,typename... Values>
inline bool operator<(
  const composite_key_result<CompositeKey>& x,
  const std::tuple<Values...>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor<
    key_tuple>::result_type                          cons_key_tuple;

  return detail::compare_ckey_cval<
    key_extractor_tuple,value_type,
    cons_key_tuple,detail::generic_operator_less_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    detail::make_cons_stdtuple(y),detail::generic_operator_less_tuple());
}

template<typename CompositeKey,typename... Values>
inline bool operator<(
  const std::tuple<Values...>& x,
  const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor<
    key_tuple>::result_type                          cons_key_tuple;

  return detail::compare_ckey_cval<
    key_extractor_tuple,value_type,
    cons_key_tuple,detail::generic_operator_less_tuple
  >::compare(
    detail::make_cons_stdtuple(x),y.composite_key.key_extractors(),
    y.value,detail::generic_operator_less_tuple());
}

/* rest of comparison operators */

#define BOOST_MULTI_INDEX_CK_COMPLETE_COMP_OPS(t1,t2,a1,a2)                  \
template<t1,t2> inline bool operator!=(const a1& x,const a2& y)              \
{                                                                            \
  return !(x==y);                                                            \
}                                                                            \
                                                                             \
template<t1,t2> inline bool operator>(const a1& x,const a2& y)               \
{                                                                            \
  return y<x;                                                                \
}                                                                            \
                                                                             \
template<t1,t2> inline bool operator>=(const a1& x,const a2& y)              \
{                                                                            \
  return !(x<y);                                                             \
}                                                                            \
                                                                             \
template<t1,t2> inline bool operator<=(const a1& x,const a2& y)              \
{                                                                            \
  return !(y<x);                                                             \
}

template<typename CompositeKey1, typename CompositeKey2>
inline bool operator!=(const composite_key_result<CompositeKey1>& x,
                       const composite_key_result<CompositeKey2>& y)
{ return !(x==y); }

template<typename CompositeKey1, typename CompositeKey2>
inline bool operator>(const composite_key_result<CompositeKey1>& x,
                      const composite_key_result<CompositeKey2>& y)
{ return y<x; }

template<typename CompositeKey1, typename CompositeKey2>
inline bool operator>=(const composite_key_result<CompositeKey1>& x,
                        const composite_key_result<CompositeKey2>& y)
{ return !(x<y); }

template<typename CompositeKey1, typename CompositeKey2>
inline bool operator<=(const composite_key_result<CompositeKey1>& x,
                       const composite_key_result<CompositeKey2>& y)
{ return !(y<x); }

template<typename CompositeKey,
         typename Value0, typename Value1,
         typename Value2, typename Value3,
         typename Value4, typename Value5,
         typename Value6, typename Value7,
         typename Value8, typename Value9>
inline
bool operator!=(const composite_key_result<CompositeKey>& x,
                const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                                   Value5, Value6, Value7, Value8, Value9>& y)
{ return !(x==y); }

template<typename CompositeKey,
         typename Value0, typename Value1,
         typename Value2, typename Value3,
         typename Value4, typename Value5,
         typename Value6, typename Value7,
         typename Value8, typename Value9>
inline bool operator>(const composite_key_result<CompositeKey>& x,
               const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                                  Value5, Value6, Value7, Value8, Value9>& y)
{ return y<x; }

template<typename CompositeKey,
         typename Value0, typename Value1,
         typename Value2, typename Value3,
         typename Value4, typename Value5,
         typename Value6, typename Value7,
         typename Value8, typename Value9>
inline bool operator>=(const composite_key_result<CompositeKey>& x,
                const boost::tuple< Value0, Value1, Value2, Value3, Value4,
                                    Value5, Value6, Value7, Value8, Value9>& y)
{ return !(x<y); }

template<typename CompositeKey,
         typename Value0, typename Value1,
         typename Value2, typename Value3,
         typename Value4, typename Value5,
         typename Value6, typename Value7,
         typename Value8, typename Value9>
inline bool operator<=(const composite_key_result<CompositeKey>& x,
                const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                                   Value5, Value6, Value7, Value8, Value9>& y)
{ return !(y<x); }

template<typename Value0, typename Value1,
         typename Value2, typename Value3,
         typename Value4, typename Value5,
         typename Value6, typename Value7,
         typename Value8, typename Value9,
         typename CompositeKey>
inline bool operator!=(
              const boost::tuple< Value0, Value1, Value2, Value3, Value4,
                                  Value5, Value6, Value7, Value8, Value9>& x,
                       const composite_key_result<CompositeKey>& y)
{ return !(x==y); }

template<typename Value0, typename Value1,
         typename Value2, typename Value3,
         typename Value4, typename Value5,
         typename Value6, typename Value7,
         typename Value8, typename Value9,
         typename CompositeKey>
inline bool operator>(
               const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                                  Value5, Value6, Value7, Value8, Value9>& x,
        const composite_key_result<CompositeKey>& y)
{ return y<x; }

template<typename Value0, typename Value1,
         typename Value2, typename Value3,
         typename Value4, typename Value5,
         typename Value6, typename Value7,
         typename Value8, typename Value9,
         typename CompositeKey>
inline bool operator>=(
                const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                                   Value5, Value6, Value7, Value8, Value9>& x,
        const composite_key_result<CompositeKey>& y)
{ return !(x<y); }

template<typename Value0, typename Value1,
         typename Value2, typename Value3,
         typename Value4, typename Value5,
         typename Value6, typename Value7,
         typename Value8, typename Value9,
         typename CompositeKey>
inline bool operator<=(
                const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                                   Value5, Value6, Value7, Value8, Value9>& x,
        const composite_key_result<CompositeKey>& y)
{ return !(y<x); }

template<typename CompositeKey, typename... Values>
inline bool operator!=(const composite_key_result<CompositeKey>& x,
                       const std::tuple<Values...>& y)
{ return !(x==y); }

template<typename CompositeKey, typename... Values>
inline bool operator>(const composite_key_result<CompositeKey>& x,
                      const std::tuple<Values...>& y)
{ return y<x; }

template<typename CompositeKey, typename... Values>
inline bool operator>=(const composite_key_result<CompositeKey>& x,
                       const std::tuple<Values...>& y)
{ return !(x<y); }

template<typename CompositeKey, typename... Values>
inline bool operator<=(const composite_key_result<CompositeKey>& x,
                       const std::tuple<Values...>& y)
{ return !(y<x); }

template<typename CompositeKey, typename... Values>
inline bool operator!=(const std::tuple<Values...>& x,
                       const composite_key_result<CompositeKey>& y)
{ return !(x==y); }

template<typename CompositeKey, typename... Values>
inline bool operator>(const std::tuple<Values...>& x,
                      const composite_key_result<CompositeKey>& y)
{ return y<x; }

template<typename CompositeKey, typename... Values>
inline bool operator>=(const std::tuple<Values...>& x,
                       const composite_key_result<CompositeKey>& y)
{ return !(x<y); }

template<typename CompositeKey, typename... Values>
inline bool operator<=(const std::tuple<Values...>& x,
                       const composite_key_result<CompositeKey>& y)
{ return !(y<x); }

/* composite_key_equal_to */

template <typename Pred0,
          typename Pred1=boost::tuples::null_type,
          typename Pred2=boost::tuples::null_type,
          typename Pred3=boost::tuples::null_type,
          typename Pred4=boost::tuples::null_type,
          typename Pred5=boost::tuples::null_type,
          typename Pred6=boost::tuples::null_type,
          typename Pred7=boost::tuples::null_type,
          typename Pred8=boost::tuples::null_type,
          typename Pred9=boost::tuples::null_type
>
struct composite_key_equal_to:
  private boost::tuple<Pred0, Pred1, Pred2, Pred3, Pred4,
                       Pred5, Pred6, Pred7, Pred8, Pred9>
{
private:
  typedef boost::tuple<Pred0, Pred1, Pred2, Pred3, Pred4,
                       Pred5, Pred6, Pred7, Pred8, Pred9> super;

public:
  typedef super key_eq_tuple;

  composite_key_equal_to(const Pred0& k0 = Pred0(),
                         const Pred1& k1 = Pred1(),
                         const Pred2& k2 = Pred2(),
                         const Pred3& k3 = Pred3(),
                         const Pred4& k4 = Pred4(),
                         const Pred5& k5 = Pred5(),
                         const Pred6& k6 = Pred6(),
                         const Pred7& k7 = Pred7(),
                         const Pred8& k8 = Pred8(),
                         const Pred9& k9 = Pred9())
      : super(k0, k1, k2, k3, k4, k5, k6, k7, k8, k9)
  {}

  composite_key_equal_to(const key_eq_tuple& x):super(x){}

  const key_eq_tuple& key_eqs()const{return *this;}
  key_eq_tuple&       key_eqs(){return *this;}

  template<typename CompositeKey1,typename CompositeKey2>
  bool operator()(
    const composite_key_result<CompositeKey1> & x,
    const composite_key_result<CompositeKey2> & y)const
  {
    typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
    typedef typename CompositeKey1::value_type          value_type1;
    typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
    typedef typename CompositeKey2::value_type          value_type2;

    static_assert(
      boost::tuples::length<key_extractor_tuple1>::value<=
      boost::tuples::length<key_eq_tuple>::value&&
      boost::tuples::length<key_extractor_tuple1>::value==
      boost::tuples::length<key_extractor_tuple2>::value);

    return detail::equal_ckey_ckey<
      key_extractor_tuple1,value_type1,
      key_extractor_tuple2,value_type2,
      key_eq_tuple
    >::compare(
      x.composite_key.key_extractors(),x.value,
      y.composite_key.key_extractors(),y.value,
      key_eqs());
  }

  template<
    typename CompositeKey,
    typename Value0, typename Value1,
    typename Value2, typename Value3,
    typename Value4, typename Value5,
    typename Value6, typename Value7,
    typename Value8, typename Value9
  >
  bool operator()(const composite_key_result<CompositeKey>& x,
           const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                              Value5, Value6, Value7, Value8, Value9>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef boost::tuple<Value0, Value1, Value2, Value3, Value4,
                         Value5, Value6, Value7, Value8, Value9> key_tuple;

    static_assert(
      boost::tuples::length<key_extractor_tuple>::value<=
      boost::tuples::length<key_eq_tuple>::value&&
      boost::tuples::length<key_extractor_tuple>::value==
      boost::tuples::length<key_tuple>::value);

    return detail::equal_ckey_cval<
      key_extractor_tuple,value_type,
      key_tuple,key_eq_tuple
    >::compare(x.composite_key.key_extractors(),x.value,y,key_eqs());
  }

  template<
    typename Value0, typename Value1,
    typename Value2, typename Value3,
    typename Value4, typename Value5,
    typename Value6, typename Value7,
    typename Value8, typename Value9,
    typename CompositeKey
  >
  bool operator()(
           const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                              Value5, Value6, Value7, Value8, Value9>& x,
                  const composite_key_result<CompositeKey>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef boost::tuple<Value0, Value1, Value2, Value3, Value4,
                         Value5, Value6, Value7, Value8, Value9> key_tuple;

    static_assert(
      boost::tuples::length<key_tuple>::value<=
      boost::tuples::length<key_eq_tuple>::value&&
      boost::tuples::length<key_tuple>::value==
      boost::tuples::length<key_extractor_tuple>::value);

    return detail::equal_ckey_cval<
      key_extractor_tuple,value_type,
      key_tuple,key_eq_tuple
    >::compare(x,y.composite_key.key_extractors(),y.value,key_eqs());
  }

  template<typename CompositeKey,typename... Values>
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const std::tuple<Values...>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                          cons_key_tuple;

    static_assert(
      boost::tuples::length<key_extractor_tuple>::value<=
      boost::tuples::length<key_eq_tuple>::value&&
      static_cast<std::size_t>(boost::tuples::length<key_extractor_tuple>::value)==
      std::tuple_size<key_tuple>::value);

    return detail::equal_ckey_cval<
      key_extractor_tuple,value_type,
      cons_key_tuple,key_eq_tuple
    >::compare(
      x.composite_key.key_extractors(),x.value,
      detail::make_cons_stdtuple(y),key_eqs());
  }

  template<typename CompositeKey,typename... Values>
  bool operator()(
    const std::tuple<Values...>& x,
    const composite_key_result<CompositeKey>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                          cons_key_tuple;

    static_assert(
      std::tuple_size<key_tuple>::value<=
      static_cast<std::size_t>(boost::tuples::length<key_eq_tuple>::value)&&
      std::tuple_size<key_tuple>::value==
      static_cast<std::size_t>(boost::tuples::length<key_extractor_tuple>::value));

    return detail::equal_ckey_cval<
      key_extractor_tuple,value_type,
      cons_key_tuple,key_eq_tuple
    >::compare(
      detail::make_cons_stdtuple(x),y.composite_key.key_extractors(),
      y.value,key_eqs());
  }
};

/* composite_key_compare */

template<
  typename Compare0,
  typename Compare1=boost::tuples::null_type,
  typename Compare2=boost::tuples::null_type,
  typename Compare3=boost::tuples::null_type,
  typename Compare4=boost::tuples::null_type,
  typename Compare5=boost::tuples::null_type,
  typename Compare6=boost::tuples::null_type,
  typename Compare7=boost::tuples::null_type,
  typename Compare8=boost::tuples::null_type,
  typename Compare9=boost::tuples::null_type
>
struct composite_key_compare:
  private boost::tuple<Compare0, Compare1, Compare2, Compare3, Compare4,
                       Compare5, Compare6, Compare7, Compare8, Compare9>
{
private:
  typedef boost::tuple<Compare0, Compare1, Compare2, Compare3, Compare4,
                       Compare5, Compare6, Compare7, Compare8, Compare9> super;

public:
  typedef super key_comp_tuple;

  composite_key_compare(const Compare0& k0 = Compare0(),
                        const Compare1& k1 = Compare1(),
                        const Compare2& k2 = Compare2(),
                        const Compare3& k3 = Compare3(),
                        const Compare4& k4 = Compare4(),
                        const Compare5& k5 = Compare5(),
                        const Compare6& k6 = Compare6(),
                        const Compare7& k7 = Compare7(),
                        const Compare8& k8 = Compare8(),
                        const Compare9& k9 = Compare9())
      : super(k0, k1, k2, k3, k4, k5, k6, k7, k8, k9)
  {}

  composite_key_compare(const key_comp_tuple& x):super(x){}

  const key_comp_tuple& key_comps()const{return *this;}
  key_comp_tuple&       key_comps(){return *this;}

  template<typename CompositeKey1,typename CompositeKey2>
  bool operator()(
    const composite_key_result<CompositeKey1> & x,
    const composite_key_result<CompositeKey2> & y)const
  {
    typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
    typedef typename CompositeKey1::value_type          value_type1;
    typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
    typedef typename CompositeKey2::value_type          value_type2;

    static_assert(
      boost::tuples::length<key_extractor_tuple1>::value<=
      boost::tuples::length<key_comp_tuple>::value||
      boost::tuples::length<key_extractor_tuple2>::value<=
      boost::tuples::length<key_comp_tuple>::value);

    return detail::compare_ckey_ckey<
      key_extractor_tuple1,value_type1,
      key_extractor_tuple2,value_type2,
      key_comp_tuple
    >::compare(
      x.composite_key.key_extractors(),x.value,
      y.composite_key.key_extractors(),y.value,
      key_comps());
  }

  template<typename CompositeKey,typename Value>
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const Value& y)const
  {
    return operator()(x,boost::make_tuple(std::cref(y)));
  }

  template<typename CompositeKey,
           typename Value0, typename Value1,
           typename Value2, typename Value3,
           typename Value4, typename Value5,
           typename Value6, typename Value7,
           typename Value8, typename Value9
  >
  bool operator()(const composite_key_result<CompositeKey>& x,
           const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                              Value5, Value6, Value7, Value8, Value9>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef boost::tuple<Value0, Value1, Value2, Value3, Value4,
                         Value5, Value6, Value7, Value8, Value9> key_tuple;

    static_assert(
      boost::tuples::length<key_extractor_tuple>::value<=
      boost::tuples::length<key_comp_tuple>::value||
      boost::tuples::length<key_tuple>::value<=
      boost::tuples::length<key_comp_tuple>::value);

    return detail::compare_ckey_cval<
      key_extractor_tuple,value_type,
      key_tuple,key_comp_tuple
    >::compare(x.composite_key.key_extractors(),x.value,y,key_comps());
  }

  template<typename Value,typename CompositeKey>
  bool operator()(
    const Value& x,
    const composite_key_result<CompositeKey>& y)const
  {
    return operator()(boost::make_tuple(std::cref(x)),y);
  }

  template<
    typename Value0, typename Value1,
    typename Value2, typename Value3,
    typename Value4, typename Value5,
    typename Value6, typename Value7,
    typename Value8, typename Value9,
    typename CompositeKey
  >
  bool operator()(
           const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                              Value5, Value6, Value7, Value8, Value9>& x,
                  const composite_key_result<CompositeKey>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef boost::tuple<Value0, Value1, Value2, Value3, Value4,
                         Value5, Value6, Value7, Value8, Value9> key_tuple;

    static_assert(
      boost::tuples::length<key_tuple>::value<=
      boost::tuples::length<key_comp_tuple>::value||
      boost::tuples::length<key_extractor_tuple>::value<=
      boost::tuples::length<key_comp_tuple>::value);

    return detail::compare_ckey_cval<
      key_extractor_tuple,value_type,
      key_tuple,key_comp_tuple
    >::compare(x,y.composite_key.key_extractors(),y.value,key_comps());
  }

  template<typename CompositeKey,typename... Values>
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const std::tuple<Values...>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                          cons_key_tuple;

    static_assert(
      boost::tuples::length<key_extractor_tuple>::value<=
      boost::tuples::length<key_comp_tuple>::value||
      std::tuple_size<key_tuple>::value<=
      static_cast<std::size_t>(boost::tuples::length<key_comp_tuple>::value));

    return detail::compare_ckey_cval<
      key_extractor_tuple,value_type,
      cons_key_tuple,key_comp_tuple
    >::compare(
      x.composite_key.key_extractors(),x.value,
      detail::make_cons_stdtuple(y),key_comps());
  }

  template<typename CompositeKey,typename... Values>
  bool operator()(
    const std::tuple<Values...>& x,
    const composite_key_result<CompositeKey>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                          cons_key_tuple;

    static_assert(
      std::tuple_size<key_tuple>::value<=
      static_cast<std::size_t>(boost::tuples::length<key_comp_tuple>::value)||
      boost::tuples::length<key_extractor_tuple>::value<=
      boost::tuples::length<key_comp_tuple>::value);

    return detail::compare_ckey_cval<
      key_extractor_tuple,value_type,
      cons_key_tuple,key_comp_tuple
    >::compare(
      detail::make_cons_stdtuple(x),y.composite_key.key_extractors(),
      y.value,key_comps());
  }
};

/* composite_key_hash */

template<
  typename Hash0,
  typename Hash1=boost::tuples::null_type,
  typename Hash2=boost::tuples::null_type,
  typename Hash3=boost::tuples::null_type,
  typename Hash4=boost::tuples::null_type,
  typename Hash5=boost::tuples::null_type,
  typename Hash6=boost::tuples::null_type,
  typename Hash7=boost::tuples::null_type,
  typename Hash8=boost::tuples::null_type,
  typename Hash9=boost::tuples::null_type
>
struct composite_key_hash:
  private boost::tuple<Hash0, Hash1, Hash2, Hash3, Hash4,
                       Hash5, Hash6, Hash7, Hash8, Hash9>
{
private:
  typedef boost::tuple<Hash0, Hash1, Hash2, Hash3, Hash4,
                       Hash5, Hash6, Hash7, Hash8, Hash9> super;

public:
  typedef super key_hasher_tuple;

  composite_key_hash(const Hash0& k0 = Hash0(),
                     const Hash1& k1 = Hash1(),
                     const Hash2& k2 = Hash2(),
                     const Hash3& k3 = Hash3(),
                     const Hash4& k4 = Hash4(),
                     const Hash5& k5 = Hash5(),
                     const Hash6& k6 = Hash6(),
                     const Hash7& k7 = Hash7(),
                     const Hash8& k8 = Hash8(),
                     const Hash9& k9 = Hash9())
      : super(k0, k1, k2, k3, k4, k5, k6, k7, k8, k9)
  {}

  composite_key_hash(const key_hasher_tuple& x):super(x){}

  const key_hasher_tuple& key_hash_functions()const{return *this;}
  key_hasher_tuple&       key_hash_functions(){return *this;}

  template<typename CompositeKey>
  std::size_t operator()(const composite_key_result<CompositeKey> & x)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;

    static_assert(
      boost::tuples::length<key_extractor_tuple>::value==
      boost::tuples::length<key_hasher_tuple>::value);

    return detail::hash_ckey<
      key_extractor_tuple,value_type,
      key_hasher_tuple
    >::hash(x.composite_key.key_extractors(),x.value,key_hash_functions());
  }

  template<typename Value0, typename Value1,
           typename Value2, typename Value3,
           typename Value4, typename Value5,
           typename Value6, typename Value7,
           typename Value8, typename Value9>
  std::size_t operator()(
                  const boost::tuple<Value0, Value1, Value2, Value3, Value4,
                                     Value5, Value6, Value7, Value8, Value9>& x)
  const
  {
    typedef boost::tuple<Value0, Value1, Value2, Value3, Value4,
                         Value5, Value6, Value7, Value8, Value9> key_tuple;

    static_assert(
      boost::tuples::length<key_tuple>::value==
      boost::tuples::length<key_hasher_tuple>::value);

    return detail::hash_cval<
      key_tuple,key_hasher_tuple
    >::hash(x,key_hash_functions());
  }

  template<typename... Values>
  std::size_t operator()(const std::tuple<Values...>& x)const
  {
    typedef std::tuple<Values...>                key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                    cons_key_tuple;

    static_assert(
      std::tuple_size<key_tuple>::value==
      static_cast<std::size_t>(boost::tuples::length<key_hasher_tuple>::value));

    return detail::hash_cval<
      cons_key_tuple,key_hasher_tuple
    >::hash(detail::make_cons_stdtuple(x),key_hash_functions());
  }
};

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
        (typename CompositeKeyResult::composite_key_type,      \
          BOOST_PP_NIL)))                                                    \
  >

template<typename CompositeKeyResult>
struct composite_key_result_equal_to:
private
  composite_key_equal_to<
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,0>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,1>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,2>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,3>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,4>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,5>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,6>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,7>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,8>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,9>::type
  >
{
private:
  typedef composite_key_equal_to<
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,0>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,1>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,2>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,3>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,4>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,5>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,6>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,7>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,8>::type,
    typename detail::nth_composite_key_equal_to<
      typename CompositeKeyResult::composite_key_type,9>::type
  > super;

public:
  typedef CompositeKeyResult  first_argument_type;
  typedef first_argument_type second_argument_type;
  typedef bool                result_type;

  using super::operator();
};

#define BOOST_MULTI_INDEX_CK_RESULT_LESS_SUPER                               \
composite_key_compare<                                                       \
    BOOST_MULTI_INDEX_CK_ENUM(                                               \
      BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N,                             \
      /* the argument is a PP list */                                        \
      (detail::nth_composite_key_less,                                       \
        (typename CompositeKeyResult::composite_key_type,      \
          BOOST_PP_NIL)))                                                    \
  >

template<typename CompositeKeyResult>
struct composite_key_result_less:
private
  composite_key_compare<
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,0>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,1>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,2>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,3>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,4>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,5>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,6>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,7>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,8>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,9>::type
  >
{
private:
  typedef composite_key_compare<
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,0>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,1>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,2>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,3>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,4>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,5>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,6>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,7>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,8>::type,
    typename detail::nth_composite_key_less<
      typename CompositeKeyResult::composite_key_type,9>::type
  > super;

public:
  typedef CompositeKeyResult  first_argument_type;
  typedef first_argument_type second_argument_type;
  typedef bool                result_type;

  using super::operator();
};

#define BOOST_MULTI_INDEX_CK_RESULT_GREATER_SUPER                            \
composite_key_compare<                                                       \
    BOOST_MULTI_INDEX_CK_ENUM(                                               \
      BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N,                             \
      /* the argument is a PP list */                                        \
      (detail::nth_composite_key_greater,                                    \
        (typename CompositeKeyResult::composite_key_type,      \
          BOOST_PP_NIL)))                                                    \
  >

template<typename CompositeKeyResult>
struct composite_key_result_greater:
private
  composite_key_compare<
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,0>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,1>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,2>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,3>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,4>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,5>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,6>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,7>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,8>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,9>::type
  >
{
private:
  typedef composite_key_compare<
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,0>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,1>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,2>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,3>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,4>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,5>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,6>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,7>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,8>::type,
    typename detail::nth_composite_key_greater<
      typename CompositeKeyResult::composite_key_type,9>::type
  > super;

public:
  typedef CompositeKeyResult  first_argument_type;
  typedef first_argument_type second_argument_type;
  typedef bool                result_type;

  using super::operator();
};

#define BOOST_MULTI_INDEX_CK_RESULT_HASH_SUPER                               \
composite_key_hash<                                                          \
    BOOST_MULTI_INDEX_CK_ENUM(                                               \
      BOOST_MULTI_INDEX_CK_APPLY_METAFUNCTION_N,                             \
      /* the argument is a PP list */                                        \
      (detail::nth_composite_key_hash,                                       \
        (typename CompositeKeyResult::composite_key_type,      \
          BOOST_PP_NIL)))                                                    \
  >

template<typename CompositeKeyResult>
struct composite_key_result_hash:
private
composite_key_hash<
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,0>::type,
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,1>::type,
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,2>::type,
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,3>::type,
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,4>::type,
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,5>::type,
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,6>::type,
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,7>::type,
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,8>::type,
  typename detail::nth_composite_key_hash<
    typename CompositeKeyResult::composite_key_type,9>::type
>
{
private:
  typedef composite_key_hash<
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,0>::type,
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,1>::type,
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,2>::type,
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,3>::type,
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,4>::type,
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,5>::type,
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,6>::type,
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,7>::type,
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,8>::type,
    typename detail::nth_composite_key_hash<
      typename CompositeKeyResult::composite_key_type,9>::type
  > super;

public:
  typedef CompositeKeyResult argument_type;
  typedef std::size_t        result_type;

  using super::operator();
};

} // boost::multi_index

/* Specializations of std::equal_to, std::less, std::greater and std::hash
 * for composite_key_results enabling interoperation with tuples of values.
 */

namespace std{

template<typename CompositeKey>
struct equal_to<boost::multi_index::composite_key_result<CompositeKey> >:
  boost::multi_index::composite_key_result_equal_to<
    boost::multi_index::composite_key_result<CompositeKey>
  >
{
};

template<typename CompositeKey>
struct less<boost::multi_index::composite_key_result<CompositeKey> >:
  boost::multi_index::composite_key_result_less<
    boost::multi_index::composite_key_result<CompositeKey>
  >
{
};

template<typename CompositeKey>
struct greater<boost::multi_index::composite_key_result<CompositeKey> >:
  boost::multi_index::composite_key_result_greater<
    boost::multi_index::composite_key_result<CompositeKey>
  >
{
};

template<typename CompositeKey>
struct hash<boost::multi_index::composite_key_result<CompositeKey> >:
  boost::multi_index::composite_key_result_hash<
    boost::multi_index::composite_key_result<CompositeKey>
  >
{
};

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
