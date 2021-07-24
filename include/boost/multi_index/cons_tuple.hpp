//  tuple.hpp - Boost Tuple Library --------------------------------------

// Copyright (C) 1999, 2000 Jaakko Jarvi (jaakko.jarvi@cs.utu.fi)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

// -----------------------------------------------------------------

#ifndef BOOST_MULTI_INDEX_CONS_TUPLE_HPP
#define BOOST_MULTI_INDEX_CONS_TUPLE_HPP
#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <cstddef>

namespace boost::multi_index {

constexpr std::size_t cons_tuple_size = 10;

struct cons_null { };

namespace detail {

// a helper function to provide a const cons_null type temporary
inline const cons_null cnull() { return cons_null(); }

} // detail

template <class HT, class TT> struct cons;

template <
  class T0 = cons_null, class T1 = cons_null, class T2 = cons_null,
  class T3 = cons_null, class T4 = cons_null, class T5 = cons_null,
  class T6 = cons_null, class T7 = cons_null, class T8 = cons_null,
  class T9 = cons_null>
class cons_tuple;

template<class T> struct cons_size;

namespace detail {

// -- generate error template, referencing to non-existing members of this
// template is used to produce compilation errors intentionally
template<class T> class generate_error;

template<int N>
struct drop_front {
  template<class Tuple>
  struct apply {
    typedef typename drop_front<N-1>::template apply<Tuple> next;
    typedef typename next::type::tail_type type;
    static const type& call(const Tuple& tup) {
      return next::call(tup).tail;
    }
  };
}; // drop_front<N>

template<>
struct drop_front<0> {
  template<class Tuple>
  struct apply {
    typedef Tuple type;
    static const type& call(const Tuple& tup) {
      return tup;
    }
  };
}; // drop_front<0>

} // detail

// -cons type accessors ----------------------------------------
// typename cons_element<N,T>::type gets the type of the
// Nth element ot T, first element is at index 0
// -------------------------------------------------------

template<int N, class T>
struct cons_element {
  typedef typename detail::drop_front<N>::template
      apply<T>::type::head_type type;
}; // cons_element<N, T>

template<int N, class T>
struct cons_element<N, const T> {
private:
  typedef typename detail::drop_front<N>::template
      apply<T>::type::head_type unqualified_type;
public:
  typedef std::add_const_t<unqualified_type> type;
}; // cons_element<N, const T>

// -get function templates -----------------------------------------------
// Usage: get<N>(aTuple)

// -- some traits classes for get functions

// Access traits lifted from detail namespace to be part of the interface,
// (Joel de Guzman's suggestion). Rationale: get functions are part of the
// interface, so should the way to express their return types be.

template <class T> struct access_traits {
  typedef const T& const_type;
  typedef T& non_const_type;
  typedef const std::remove_cv_t<T>& parameter_type;

// Used as the cons_tuple constructors parameter types
// Rationale: non-reference cons_tuple element types can be cv-qualified.
// It should be possible to initialize such types with temporaries,
// and when binding temporaries to references, the reference must
// be non-volatile and const. 8.5.3. (5)
}; // access_traits<T>

template <class T> struct access_traits<T&> {
  typedef T& const_type;
  typedef T& non_const_type;
  typedef T& parameter_type;
}; // access_traits<T&>

// get function for non-const cons-lists, returns a reference to the element

template<int N, class HT, class TT>
inline typename access_traits<
                  typename cons_element<N, cons<HT, TT> >::type
                >::non_const_type
get(cons<HT, TT>& c) {
  typedef typename detail::drop_front<N>::template
      apply<cons<HT, TT> > impl;
  typedef typename impl::type cons_element;
  return const_cast<cons_element&>(impl::call(c)).head;
} // get non-const

// get function for const cons-lists, returns a const reference to
// the element. If the element is a reference, returns the reference
// as such (that is, can return a non-const reference).
template<int N, class HT, class TT>
inline typename access_traits<
                  typename cons_element<N, cons<HT, TT> >::type
                >::const_type
get(const cons<HT, TT>& c) {
  typedef typename detail::drop_front<N>::template
      apply<cons<HT, TT> > impl;
  return impl::call(c).head;
} // get const

// -- the cons template  --------------------------------------------------
namespace detail {

//  These helper templates wrap void types and plain function types.
//  The reationale is to allow one to write cons_tuple types with those types
//  as elements, even though it is not possible to instantiate such object.
//  E.g: typedef cons_tuple<void> some_type; // ok
//  but: some_type x; // fails

template <class T> class non_storeable_type {
  non_storeable_type();
};

template <class T> struct wrap_non_storeable_type {
  typedef std::conditional_t<
    std::is_function_v<T>, non_storeable_type<T>, T
  > type;
};
template <> struct wrap_non_storeable_type<void> {
  typedef non_storeable_type<void> type;
};

} // detail

template <class HT, class TT>
struct cons {

  typedef HT head_type;
  typedef TT tail_type;

  typedef typename
      detail::wrap_non_storeable_type<head_type>::type stored_head_type;

  stored_head_type head;
  tail_type tail;

  typename access_traits<stored_head_type>::non_const_type
  get_head() { return head; }

  typename access_traits<tail_type>::non_const_type
  get_tail() { return tail; }

  typename access_traits<stored_head_type>::const_type
  get_head() const { return head; }

  typename access_traits<tail_type>::const_type
  get_tail() const { return tail; }

  cons() : head(), tail() {}
  // cons() : head(detail::default_arg<HT>::f()), tail() {}

  // The argument for head is not strictly needed, but it prevents
  // array type elements. This is good, since array type elements
  // cannot be supported properly in any case (no assignment,
  // copy works only if the tails are exactly the same type, ...)

  cons(typename access_traits<stored_head_type>::parameter_type h,
       const tail_type& t)
    : head(h), tail(t) {}

  template <class T1, class T2, class T3, class T4, class T5,
            class T6, class T7, class T8, class T9, class T10>
  cons(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
       T6& t6, T7& t7, T8& t8, T9& t9, T10& t10)
    : head(t1)
    , tail(t2, t3, t4, t5, t6, t7, t8, t9, t10, detail::cnull())
    {}

  template <class T2, class T3, class T4, class T5,
            class T6, class T7, class T8, class T9, class T10>
  cons(const cons_null& /*t1*/, T2& t2, T3& t3, T4& t4, T5& t5,
        T6& t6, T7& t7, T8& t8, T9& t9, T10& t10)
    : head()
    , tail(t2, t3, t4, t5, t6, t7, t8, t9, t10, detail::cnull())
    {}

  cons( const cons& u ) : head(u.head), tail(u.tail) {}

  template <class HT2, class TT2>
  cons( const cons<HT2, TT2>& u ) : head(u.head), tail(u.tail) {}

  template <class HT2, class TT2>
  cons& operator=( const cons<HT2, TT2>& u ) {
    head = u.head;
    tail = u.tail;
    return *this;
  }

  // must define assignment operator explicitly, implicit version is
  // illformed if HT is a reference (12.8. (12))
  cons& operator=(const cons& u) {
    head = u.head;
    tail = u.tail;
    return *this;
  }

  template <class T1, class T2>
  cons& operator=( const std::pair<T1, T2>& u ) {
    static_assert(cons_size<cons>::value == 2); // check length = 2
    head = u.first;
    tail.head = u.second;
    return *this;
  }

  // get member functions (non-const and const)
  template <int N>
  typename access_traits<
             typename cons_element<N, cons<HT, TT> >::type
           >::non_const_type
  get() {
    return boost::multi_index::get<N>(*this); // delegate to non-member get
  }

  template <int N>
  typename access_traits<
             typename cons_element<N, cons<HT, TT> >::type
           >::const_type
  get() const {
    return boost::multi_index::get<N>(*this); // delegate to non-member get
  }
}; // cons<class HT, class TT>

template <class HT>
struct cons<HT, cons_null> {

  typedef HT head_type;
  typedef cons_null tail_type;
  typedef cons<HT, cons_null> self_type;

  typedef typename
    detail::wrap_non_storeable_type<head_type>::type stored_head_type;
  stored_head_type head;

  typename access_traits<stored_head_type>::non_const_type
  get_head() { return head; }

  cons_null get_tail() { return cons_null(); }

  typename access_traits<stored_head_type>::const_type
  get_head() const { return head; }

  const cons_null get_tail() const { return cons_null(); }

  //  cons() : head(detail::default_arg<HT>::f()) {}
  cons() : head() {}

  cons(typename access_traits<stored_head_type>::parameter_type h,
       const cons_null& = cons_null())
    : head (h) {}

  template<class T1>
  cons(T1& t1, const cons_null&, const cons_null&, const cons_null&,
       const cons_null&, const cons_null&, const cons_null&,
       const cons_null&, const cons_null&, const cons_null&)
  : head (t1) {}

  cons(const cons_null&,
       const cons_null&, const cons_null&, const cons_null&,
       const cons_null&, const cons_null&, const cons_null&,
       const cons_null&, const cons_null&, const cons_null&)
  : head () {}

  cons(const cons& u) : head(u.head) {}

  template <class HT2>
  cons(const cons<HT2, cons_null>& u) : head(u.head) {}

  template <class HT2>
  cons& operator=(const cons<HT2, cons_null>& u ) {
    head = u.head;
    return *this;
  }

  // Must define assignment operator explicitely. Implicit version
  // is illformed if HT is a reference.
  cons& operator=(const cons& u) { head = u.head; return *this; }

  template <int N>
  typename access_traits<
             typename cons_element<N, self_type>::type
            >::non_const_type
  get() {
    return boost::multi_index::get<N>(*this);
  }

  template <int N>
  typename access_traits<
             typename cons_element<N, self_type>::type
           >::const_type
  get() const {
    return boost::multi_index::get<N>(*this);
  }

}; // cons<HT, cons_null>

// templates for finding out the length of the cons_tuple -------------------

template<class T>
struct cons_size
  : std::integral_constant<std::size_t,
         1 + cons_size<typename T::tail_type>::value>
{ };

template<>
struct cons_size<cons_tuple<>> : std::integral_constant<std::size_t, 0>
{ };

template<>
struct cons_size<cons_tuple<> const> : std::integral_constant<std::size_t, 0>
{ };

template<>
struct cons_size<cons_null> : std::integral_constant<std::size_t, 0>
{ };

template<>
struct cons_size<cons_null const> : std::integral_constant<std::size_t, 0>
{ };

namespace detail {

// Tuple to cons mapper --------------------------------------------------
template <class T0, class T1, class T2, class T3, class T4,
          class T5, class T6, class T7, class T8, class T9>
struct map_tuple_to_cons {
  typedef cons<T0,
               typename map_tuple_to_cons<T1, T2, T3, T4, T5,
                                          T6, T7, T8, T9, cons_null>::type
              > type;
};

// The empty cons_tuple is a cons_null
template <>
struct map_tuple_to_cons<cons_null, cons_null, cons_null, cons_null, cons_null,
                         cons_null, cons_null, cons_null, cons_null, cons_null>
{
  typedef cons_null type;
};

} // detail

// -------------------------------------------------------------------
// -- cons_tuple ------------------------------------------------------
template <class T0, class T1, class T2, class T3, class T4,
          class T5, class T6, class T7, class T8, class T9>

class cons_tuple :
  public detail::map_tuple_to_cons<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type
{
public:
  typedef typename
    detail::map_tuple_to_cons<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type
        inherited;
  typedef typename inherited::head_type head_type;
  typedef typename inherited::tail_type tail_type;

// access_traits<T>::parameter_type takes non-reference types as const T&
  cons_tuple() : inherited() {}

  explicit
  cons_tuple(typename access_traits<T0>::parameter_type t0,
             typename access_traits<T1>::parameter_type t1 = cons_null(),
             typename access_traits<T2>::parameter_type t2 = cons_null(),
             typename access_traits<T3>::parameter_type t3 = cons_null(),
             typename access_traits<T4>::parameter_type t4 = cons_null(),
             typename access_traits<T5>::parameter_type t5 = cons_null(),
             typename access_traits<T6>::parameter_type t6 = cons_null(),
             typename access_traits<T7>::parameter_type t7 = cons_null(),
             typename access_traits<T8>::parameter_type t8 = cons_null(),
             typename access_traits<T9>::parameter_type t9 = cons_null())
    : inherited(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9) {}


  template<class U1, class U2>
  cons_tuple(const cons<U1, U2>& p) : inherited(p) {}

  template <class U1, class U2>
  cons_tuple& operator=(const cons<U1, U2>& k) {
    inherited::operator=(k);
    return *this;
  }

  template <class U1, class U2>
  cons_tuple& operator=(const std::pair<U1, U2>& k) {
    static_assert(cons_size<cons_tuple>::value == 2);// check_length = 2
    this->head = k.first;
    this->tail.head = k.second;
    return *this;
  }

}; // cons_tuple

// The empty cons_tuple
template <>
class cons_tuple<cons_null, cons_null, cons_null, cons_null, cons_null,
                 cons_null, cons_null, cons_null, cons_null, cons_null>
  : public cons_null
{
public:
  typedef cons_null inherited;
}; // cons_tuple<>

// Swallows any assignment   (by Doug Gregor)
namespace detail {

struct swallow_assign;

typedef void (detail::swallow_assign::*ignore_t)();

struct swallow_assign {
  swallow_assign(ignore_t(*)(ignore_t)) {}
  template<typename T>
  swallow_assign const& operator=(const T&) const { return *this; }
}; // swallow_assign

} // detail

// "ignore" allows cons_tuple positions to be ignored when using "tie".
inline detail::ignore_t ignore(detail::ignore_t) { return 0; }

// ---------------------------------------------------------------------------
// The call_traits for make_cons_tuple
// Honours the reference_wrapper class.

// Must be instantiated with plain or const plain types (not with references)

// from template<class T> foo(const T& t) : make_tuple_traits<const T>::type
// from template<class T> foo(T& t) : make_tuple_traits<T>::type

// Conversions:
// T -> T,
// references -> compile_time_error
// reference_wrapper<T> -> T&
// const reference_wrapper<T> -> T&
// array -> const ref array


template<class T>
struct make_tuple_traits {
  typedef T type;

  // commented away, see below  (JJ)
  //  typedef typename IF<
  //  std::is_function_v<T>,
  //  T&,
  //  T>::type type;

}; // make_tuple_traits

// The is_function test was there originally for plain function types,
// which can't be stored as such (we must either store them as references or
// pointers). Such a type could be formed if make_cons_tuple was called with a
// reference to a function.
// But this would mean that a const qualified function type was formed in
// the make_cons_tuple function and hence make_cons_tuple can't take a function
// reference as a parameter, and thus T can't be a function type.
// So is_function test was removed.
// (14.8.3. says that type deduction fails if a cv-qualified function type
// is created. (It only applies for the case of explicitly specifying template
// args, though?)) (JJ)

template<class T>
struct make_tuple_traits<T&> {
  typedef typename
     detail::generate_error<T&>::
       do_not_use_with_reference_type error;
}; // make_tuple_traits<T>

// Arrays can't be stored as plain types; convert them to references.
// All arrays are converted to const. This is because make_cons_tuple takes its
// parameters as const T& and thus the knowledge of the potential
// non-constness of actual argument is lost.
template<class T, int n>  struct make_tuple_traits <T[n]> {
  typedef const T (&type)[n];
};

template<class T, int n>
struct make_tuple_traits<const T[n]> {
  typedef const T (&type)[n];
};

template<class T, int n>  struct make_tuple_traits<volatile T[n]> {
  typedef const volatile T (&type)[n];
};

template<class T, int n>
struct make_tuple_traits<const volatile T[n]> {
  typedef const volatile T (&type)[n];
};

template<class T>
struct make_tuple_traits<std::reference_wrapper<T> >{
  typedef T& type;
};

template<class T>
struct make_tuple_traits<const std::reference_wrapper<T> >{
  typedef T& type;
};

template<>
struct make_tuple_traits<detail::ignore_t(detail::ignore_t)> {
  typedef detail::swallow_assign type;
};

namespace detail {

// A helper traits to make the make_cons_tuple functions shorter
// (Vesa Karvonen's suggestion)
template <
  class T0 = cons_null, class T1 = cons_null, class T2 = cons_null,
  class T3 = cons_null, class T4 = cons_null, class T5 = cons_null,
  class T6 = cons_null, class T7 = cons_null, class T8 = cons_null,
  class T9 = cons_null
>
struct make_tuple_mapper {
  typedef
    cons_tuple<typename make_tuple_traits<T0>::type,
               typename make_tuple_traits<T1>::type,
               typename make_tuple_traits<T2>::type,
               typename make_tuple_traits<T3>::type,
               typename make_tuple_traits<T4>::type,
               typename make_tuple_traits<T5>::type,
               typename make_tuple_traits<T6>::type,
               typename make_tuple_traits<T7>::type,
               typename make_tuple_traits<T8>::type,
               typename make_tuple_traits<T9>::type> type;
}; // make_tuple_mapper

} // detail

// -make_cons_tuple function templates -----------------------------------
inline cons_tuple<> make_cons_tuple() {
  return cons_tuple<>();
}

template<class T0>
inline typename detail::make_tuple_mapper<T0>::type
make_cons_tuple(const T0& t0) {
  typedef typename detail::make_tuple_mapper<T0>::type t;
  return t(t0);
}

template<class T0, class T1>
inline typename detail::make_tuple_mapper<T0, T1>::type
make_cons_tuple(const T0& t0, const T1& t1) {
  typedef typename detail::make_tuple_mapper<T0, T1>::type t;
  return t(t0, t1);
}

template<class T0, class T1, class T2>
inline typename detail::make_tuple_mapper<T0, T1, T2>::type
make_cons_tuple(const T0& t0, const T1& t1, const T2& t2) {
  typedef typename detail::make_tuple_mapper<T0, T1, T2>::type t;
  return t(t0, t1, t2);
}

template<class T0, class T1, class T2, class T3>
inline typename detail::make_tuple_mapper<T0, T1, T2, T3>::type
make_cons_tuple(const T0& t0, const T1& t1, const T2& t2, const T3& t3) {
  typedef typename detail::make_tuple_mapper<T0, T1, T2, T3>::type t;
  return t(t0, t1, t2, t3);
}

template<class T0, class T1, class T2, class T3, class T4>
inline typename detail::make_tuple_mapper<T0, T1, T2, T3, T4>::type
make_cons_tuple(const T0& t0, const T1& t1, const T2& t2, const T3& t3,
                  const T4& t4) {
  typedef typename detail::make_tuple_mapper<T0, T1, T2, T3, T4>::type t;
  return t(t0, t1, t2, t3, t4);
}

template<class T0, class T1, class T2, class T3, class T4, class T5>
inline typename detail::make_tuple_mapper<T0, T1, T2, T3, T4, T5>::type
make_cons_tuple(const T0& t0, const T1& t1, const T2& t2, const T3& t3,
                  const T4& t4, const T5& t5) {
  typedef typename detail::make_tuple_mapper<T0, T1, T2, T3, T4, T5>::type t;
  return t(t0, t1, t2, t3, t4, t5);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline typename detail::make_tuple_mapper<T0, T1, T2, T3, T4, T5, T6>::type
make_cons_tuple(const T0& t0, const T1& t1, const T2& t2, const T3& t3,
                  const T4& t4, const T5& t5, const T6& t6) {
  typedef typename detail::make_tuple_mapper
           <T0, T1, T2, T3, T4, T5, T6>::type t;
  return t(t0, t1, t2, t3, t4, t5, t6);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
         class T7>
inline typename detail::make_tuple_mapper<T0, T1, T2, T3, T4, T5, T6, T7>::type
make_cons_tuple(const T0& t0, const T1& t1, const T2& t2, const T3& t3,
                  const T4& t4, const T5& t5, const T6& t6, const T7& t7) {
  typedef typename detail::make_tuple_mapper
           <T0, T1, T2, T3, T4, T5, T6, T7>::type t;
  return t(t0, t1, t2, t3, t4, t5, t6, t7);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
         class T7, class T8>
inline typename detail::make_tuple_mapper
  <T0, T1, T2, T3, T4, T5, T6, T7, T8>::type
make_cons_tuple(const T0& t0, const T1& t1, const T2& t2, const T3& t3,
                  const T4& t4, const T5& t5, const T6& t6, const T7& t7,
                  const T8& t8) {
  typedef typename detail::make_tuple_mapper
           <T0, T1, T2, T3, T4, T5, T6, T7, T8>::type t;
  return t(t0, t1, t2, t3, t4, t5, t6, t7, t8);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
         class T7, class T8, class T9>
inline typename detail::make_tuple_mapper
  <T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type
make_cons_tuple(const T0& t0, const T1& t1, const T2& t2, const T3& t3,
                  const T4& t4, const T5& t5, const T6& t6, const T7& t7,
                  const T8& t8, const T9& t9) {
  typedef typename detail::make_tuple_mapper
           <T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type t;
  return t(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9);
}

namespace detail {

template<class T>
struct tie_traits {
  typedef T& type;
};

template<>
struct tie_traits<ignore_t(ignore_t)> {
  typedef swallow_assign type;
};

template<>
struct tie_traits<void> {
  typedef cons_null type;
};

template <
  class T0 = void, class T1 = void, class T2 = void,
  class T3 = void, class T4 = void, class T5 = void,
  class T6 = void, class T7 = void, class T8 = void,
  class T9 = void
>
struct tie_mapper {
  typedef
    cons_tuple<typename tie_traits<T0>::type,
               typename tie_traits<T1>::type,
               typename tie_traits<T2>::type,
               typename tie_traits<T3>::type,
               typename tie_traits<T4>::type,
               typename tie_traits<T5>::type,
               typename tie_traits<T6>::type,
               typename tie_traits<T7>::type,
               typename tie_traits<T8>::type,
               typename tie_traits<T9>::type> type;
}; // tie_mapper

} // detail

// Tie function templates -------------------------------------------------
template<class T0>
inline typename detail::tie_mapper<T0>::type
tie(T0& t0) {
  typedef typename detail::tie_mapper<T0>::type t;
  return t(t0);
}

template<class T0, class T1>
inline typename detail::tie_mapper<T0, T1>::type
tie(T0& t0, T1& t1) {
  typedef typename detail::tie_mapper<T0, T1>::type t;
  return t(t0, t1);
}

template<class T0, class T1, class T2>
inline typename detail::tie_mapper<T0, T1, T2>::type
tie(T0& t0, T1& t1, T2& t2) {
  typedef typename detail::tie_mapper<T0, T1, T2>::type t;
  return t(t0, t1, t2);
}

template<class T0, class T1, class T2, class T3>
inline typename detail::tie_mapper<T0, T1, T2, T3>::type
tie(T0& t0, T1& t1, T2& t2, T3& t3) {
  typedef typename detail::tie_mapper<T0, T1, T2, T3>::type t;
  return t(t0, t1, t2, t3);
}

template<class T0, class T1, class T2, class T3, class T4>
inline typename detail::tie_mapper<T0, T1, T2, T3, T4>::type
tie(T0& t0, T1& t1, T2& t2, T3& t3, T4& t4) {
  typedef typename detail::tie_mapper<T0, T1, T2, T3, T4>::type t;
  return t(t0, t1, t2, t3, t4);
}

template<class T0, class T1, class T2, class T3, class T4, class T5>
inline typename detail::tie_mapper<T0, T1, T2, T3, T4, T5>::type
tie(T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5) {
  typedef typename detail::tie_mapper<T0, T1, T2, T3, T4, T5>::type t;
  return t(t0, t1, t2, t3, t4, t5);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline typename detail::tie_mapper<T0, T1, T2, T3, T4, T5, T6>::type
tie(T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6) {
  typedef typename detail::tie_mapper
           <T0, T1, T2, T3, T4, T5, T6>::type t;
  return t(t0, t1, t2, t3, t4, t5, t6);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
         class T7>
inline typename detail::tie_mapper<T0, T1, T2, T3, T4, T5, T6, T7>::type
tie(T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7) {
  typedef typename detail::tie_mapper
           <T0, T1, T2, T3, T4, T5, T6, T7>::type t;
  return t(t0, t1, t2, t3, t4, t5, t6, t7);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
         class T7, class T8>
inline typename detail::tie_mapper
  <T0, T1, T2, T3, T4, T5, T6, T7, T8>::type
tie(T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7, T8& t8) {
  typedef typename detail::tie_mapper
           <T0, T1, T2, T3, T4, T5, T6, T7, T8>::type t;
  return t(t0, t1, t2, t3, t4, t5, t6, t7, t8);
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
         class T7, class T8, class T9>
inline typename detail::tie_mapper
  <T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type
tie(T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7, T8& t8,
    T9& t9)
{
  typedef typename detail::tie_mapper
           <T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type t;
  return t(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9);
}

template <class T0, class T1, class T2, class T3, class T4,
          class T5, class T6, class T7, class T8, class T9>
void swap(cons_tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>& lhs,
          cons_tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>& rhs);
inline void swap(cons_null&, cons_null&) {}
template<class HH>
inline void swap(cons<HH, cons_null>& lhs, cons<HH, cons_null>& rhs) {
  std::swap(lhs.head, rhs.head);
}

template<class HH, class TT>
inline void swap(cons<HH, TT>& lhs, cons<HH, TT>& rhs) {
  std::swap(lhs.head, rhs.head);
  boost::multi_index::swap(lhs.tail, rhs.tail);
}

template <class T0, class T1, class T2, class T3, class T4,
          class T5, class T6, class T7, class T8, class T9>
inline void swap(cons_tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>& lhs,
          cons_tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>& rhs) {
  typedef cons_tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> tuple_type;
  typedef typename tuple_type::inherited base;
  boost::multi_index::swap(static_cast<base&>(lhs), static_cast<base&>(rhs));
}

} // boost::multi_index

#endif
