//
// impl/connect.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NET_TS_IMPL_CONNECT_HPP
#define NET_TS_IMPL_CONNECT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <algorithm>
#include <experimental/__net_ts/associated_allocator.hpp>
#include <experimental/__net_ts/associated_executor.hpp>
#include <experimental/__net_ts/detail/bind_handler.hpp>
#include <experimental/__net_ts/detail/handler_alloc_helpers.hpp>
#include <experimental/__net_ts/detail/handler_cont_helpers.hpp>
#include <experimental/__net_ts/detail/handler_invoke_helpers.hpp>
#include <experimental/__net_ts/detail/handler_type_requirements.hpp>
#include <experimental/__net_ts/detail/throw_error.hpp>
#include <experimental/__net_ts/error.hpp>
#include <experimental/__net_ts/post.hpp>

#include <experimental/__net_ts/detail/push_options.hpp>

namespace std {
namespace experimental {
namespace net {
inline namespace v1 {

namespace detail
{
  struct default_connect_condition
  {
    template <typename Endpoint>
    bool operator()(const std::error_code&, const Endpoint&)
    {
      return true;
    }
  };

  template <typename Protocol, typename Iterator>
  inline typename Protocol::endpoint deref_connect_result(
      Iterator iter, std::error_code& ec)
  {
    return ec ? typename Protocol::endpoint() : *iter;
  }

  template <typename T, typename Iterator>
  struct legacy_connect_condition_helper : T
  {
    typedef char (*fallback_func_type)(...);
    operator fallback_func_type() const;
  };

  template <typename R, typename Arg1, typename Arg2, typename Iterator>
  struct legacy_connect_condition_helper<R (*)(Arg1, Arg2), Iterator>
  {
    R operator()(Arg1, Arg2) const;
    char operator()(...) const;
  };

  template <typename T, typename Iterator>
  struct is_legacy_connect_condition
  {
    static char asio_connect_condition_check(char);
    static char (&asio_connect_condition_check(Iterator))[2];

    static const bool value =
      sizeof(asio_connect_condition_check(
        (*static_cast<legacy_connect_condition_helper<T, Iterator>*>(0))(
          *static_cast<const std::error_code*>(0),
          *static_cast<const Iterator*>(0)))) != 1;
  };

  template <typename ConnectCondition, typename Iterator>
  inline Iterator call_connect_condition(ConnectCondition& connect_condition,
      const std::error_code& ec, Iterator next, Iterator end,
      typename enable_if<is_legacy_connect_condition<
        ConnectCondition, Iterator>::value>::type* = 0)
  {
    if (next != end)
      return connect_condition(ec, next);
    return end;
  }

  template <typename ConnectCondition, typename Iterator>
  inline Iterator call_connect_condition(ConnectCondition& connect_condition,
      const std::error_code& ec, Iterator next, Iterator end,
      typename enable_if<!is_legacy_connect_condition<
        ConnectCondition, Iterator>::value>::type* = 0)
  {
    for (;next != end; ++next)
      if (connect_condition(ec, *next))
        return next;
    return end;
  }
}

template <typename Protocol NET_TS_SVC_TPARAM, typename EndpointSequence>
typename Protocol::endpoint connect(
    basic_socket<Protocol NET_TS_SVC_TARG>& s,
    const EndpointSequence& endpoints,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  std::error_code ec;
  typename Protocol::endpoint result = connect(s, endpoints, ec);
  std::experimental::net::v1::detail::throw_error(ec, "connect");
  return result;
}

template <typename Protocol NET_TS_SVC_TPARAM, typename EndpointSequence>
typename Protocol::endpoint connect(
    basic_socket<Protocol NET_TS_SVC_TARG>& s,
    const EndpointSequence& endpoints, std::error_code& ec,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  return detail::deref_connect_result<Protocol>(
      connect(s, endpoints.begin(), endpoints.end(),
        detail::default_connect_condition(), ec), ec);
}

template <typename Protocol NET_TS_SVC_TPARAM, typename Iterator>
Iterator connect(basic_socket<Protocol NET_TS_SVC_TARG>& s,
    Iterator begin, Iterator end)
{
  std::error_code ec;
  Iterator result = connect(s, begin, end, ec);
  std::experimental::net::v1::detail::throw_error(ec, "connect");
  return result;
}

template <typename Protocol NET_TS_SVC_TPARAM, typename Iterator>
inline Iterator connect(basic_socket<Protocol NET_TS_SVC_TARG>& s,
    Iterator begin, Iterator end, std::error_code& ec)
{
  return connect(s, begin, end, detail::default_connect_condition(), ec);
}

template <typename Protocol NET_TS_SVC_TPARAM,
    typename EndpointSequence, typename ConnectCondition>
typename Protocol::endpoint connect(
    basic_socket<Protocol NET_TS_SVC_TARG>& s,
    const EndpointSequence& endpoints, ConnectCondition connect_condition,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  std::error_code ec;
  typename Protocol::endpoint result = connect(
      s, endpoints, connect_condition, ec);
  std::experimental::net::v1::detail::throw_error(ec, "connect");
  return result;
}

template <typename Protocol NET_TS_SVC_TPARAM,
    typename EndpointSequence, typename ConnectCondition>
typename Protocol::endpoint connect(
    basic_socket<Protocol NET_TS_SVC_TARG>& s,
    const EndpointSequence& endpoints, ConnectCondition connect_condition,
    std::error_code& ec,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  return detail::deref_connect_result<Protocol>(
      connect(s, endpoints.begin(), endpoints.end(),
        connect_condition, ec), ec);
}

template <typename Protocol NET_TS_SVC_TPARAM,
    typename Iterator, typename ConnectCondition>
Iterator connect(basic_socket<Protocol NET_TS_SVC_TARG>& s,
    Iterator begin, Iterator end, ConnectCondition connect_condition)
{
  std::error_code ec;
  Iterator result = connect(s, begin, end, connect_condition, ec);
  std::experimental::net::v1::detail::throw_error(ec, "connect");
  return result;
}

template <typename Protocol NET_TS_SVC_TPARAM,
    typename Iterator, typename ConnectCondition>
Iterator connect(basic_socket<Protocol NET_TS_SVC_TARG>& s,
    Iterator begin, Iterator end, ConnectCondition connect_condition,
    std::error_code& ec)
{
  ec = std::error_code();

  for (Iterator iter = begin; iter != end; ++iter)
  {
    iter = (detail::call_connect_condition(connect_condition, ec, iter, end));
    if (iter != end)
    {
      s.close(ec);
      s.connect(*iter, ec);
      if (!ec)
        return iter;
    }
    else
      break;
  }

  if (!ec)
    ec = std::experimental::net::v1::error::not_found;

  return end;
}

namespace detail
{
  // Enable the empty base class optimisation for the connect condition.
  template <typename ConnectCondition>
  class base_from_connect_condition
  {
  protected:
    explicit base_from_connect_condition(
        const ConnectCondition& connect_condition)
      : connect_condition_(connect_condition)
    {
    }

    template <typename Iterator>
    void check_condition(const std::error_code& ec,
        Iterator& iter, Iterator& end)
    {
      iter = detail::call_connect_condition(connect_condition_, ec, iter, end);
    }

  private:
    ConnectCondition connect_condition_;
  };

  // The default_connect_condition implementation is essentially a no-op. This
  // template specialisation lets us eliminate all costs associated with it.
  template <>
  class base_from_connect_condition<default_connect_condition>
  {
  protected:
    explicit base_from_connect_condition(const default_connect_condition&)
    {
    }

    template <typename Iterator>
    void check_condition(const std::error_code&, Iterator&, Iterator&)
    {
    }
  };

  template <typename Protocol NET_TS_SVC_TPARAM,
      typename EndpointSequence, typename ConnectCondition,
      typename RangeConnectHandler>
  class range_connect_op : base_from_connect_condition<ConnectCondition>
  {
  public:
    range_connect_op(basic_socket<Protocol NET_TS_SVC_TARG>& sock,
        const EndpointSequence& endpoints,
        const ConnectCondition& connect_condition,
        RangeConnectHandler& handler)
      : base_from_connect_condition<ConnectCondition>(connect_condition),
        socket_(sock),
        endpoints_(endpoints),
        index_(0),
        start_(0),
        handler_(NET_TS_MOVE_CAST(RangeConnectHandler)(handler))
    {
    }

#if defined(NET_TS_HAS_MOVE)
    range_connect_op(const range_connect_op& other)
      : base_from_connect_condition<ConnectCondition>(other),
        socket_(other.socket_),
        endpoints_(other.endpoints_),
        index_(other.index_),
        start_(other.start_),
        handler_(other.handler_)
    {
    }

    range_connect_op(range_connect_op&& other)
      : base_from_connect_condition<ConnectCondition>(other),
        socket_(other.socket_),
        endpoints_(other.endpoints_),
        index_(other.index_),
        start_(other.start_),
        handler_(NET_TS_MOVE_CAST(RangeConnectHandler)(other.handler_))
    {
    }
#endif // defined(NET_TS_HAS_MOVE)

    void operator()(std::error_code ec, int start = 0)
    {
      typename EndpointSequence::const_iterator begin = endpoints_.begin();
      typename EndpointSequence::const_iterator iter = begin;
      std::advance(iter, index_);
      typename EndpointSequence::const_iterator end = endpoints_.end();

      switch (start_ = start)
      {
        case 1:
        for (;;)
        {
          this->check_condition(ec, iter, end);
          index_ = std::distance(begin, iter);

          if (iter != end)
          {
            socket_.close(ec);
            socket_.async_connect(*iter,
                NET_TS_MOVE_CAST(range_connect_op)(*this));
            return;
          }

          if (start)
          {
            ec = std::experimental::net::v1::error::not_found;
            std::experimental::net::v1::post(socket_.get_executor(),
                detail::bind_handler(
                  NET_TS_MOVE_CAST(range_connect_op)(*this), ec));
            return;
          }

          default:

          if (iter == end)
            break;

          if (!socket_.is_open())
          {
            ec = std::experimental::net::v1::error::operation_aborted;
            break;
          }

          if (!ec)
            break;

          ++iter;
          ++index_;
        }

        handler_(static_cast<const std::error_code&>(ec),
            static_cast<const typename Protocol::endpoint&>(
              ec || iter == end ? typename Protocol::endpoint() : *iter));
      }
    }

  //private:
    basic_socket<Protocol NET_TS_SVC_TARG>& socket_;
    EndpointSequence endpoints_;
    std::size_t index_;
    int start_;
    RangeConnectHandler handler_;
  };

  template <typename Protocol NET_TS_SVC_TPARAM,
      typename EndpointSequence, typename ConnectCondition,
      typename RangeConnectHandler>
  inline void* networking_ts_handler_allocate(std::size_t size,
      range_connect_op<Protocol NET_TS_SVC_TARG, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    return networking_ts_handler_alloc_helpers::allocate(
        size, this_handler->handler_);
  }

  template <typename Protocol NET_TS_SVC_TPARAM,
      typename EndpointSequence, typename ConnectCondition,
      typename RangeConnectHandler>
  inline void networking_ts_handler_deallocate(void* pointer, std::size_t size,
      range_connect_op<Protocol NET_TS_SVC_TARG, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    networking_ts_handler_alloc_helpers::deallocate(
        pointer, size, this_handler->handler_);
  }

  template <typename Protocol NET_TS_SVC_TPARAM,
      typename EndpointSequence, typename ConnectCondition,
      typename RangeConnectHandler>
  inline bool networking_ts_handler_is_continuation(
      range_connect_op<Protocol NET_TS_SVC_TARG, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    return networking_ts_handler_cont_helpers::is_continuation(
        this_handler->handler_);
  }

  template <typename Function, typename Protocol
      NET_TS_SVC_TPARAM, typename EndpointSequence,
      typename ConnectCondition, typename RangeConnectHandler>
  inline void networking_ts_handler_invoke(Function& function,
      range_connect_op<Protocol NET_TS_SVC_TARG, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    networking_ts_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }

  template <typename Function, typename Protocol
      NET_TS_SVC_TPARAM, typename EndpointSequence,
      typename ConnectCondition, typename RangeConnectHandler>
  inline void networking_ts_handler_invoke(const Function& function,
      range_connect_op<Protocol NET_TS_SVC_TARG, EndpointSequence,
        ConnectCondition, RangeConnectHandler>* this_handler)
  {
    networking_ts_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }

  template <typename Protocol NET_TS_SVC_TPARAM, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  class iterator_connect_op : base_from_connect_condition<ConnectCondition>
  {
  public:
    iterator_connect_op(basic_socket<Protocol NET_TS_SVC_TARG>& sock,
        const Iterator& begin, const Iterator& end,
        const ConnectCondition& connect_condition,
        IteratorConnectHandler& handler)
      : base_from_connect_condition<ConnectCondition>(connect_condition),
        socket_(sock),
        iter_(begin),
        end_(end),
        start_(0),
        handler_(NET_TS_MOVE_CAST(IteratorConnectHandler)(handler))
    {
    }

#if defined(NET_TS_HAS_MOVE)
    iterator_connect_op(const iterator_connect_op& other)
      : base_from_connect_condition<ConnectCondition>(other),
        socket_(other.socket_),
        iter_(other.iter_),
        end_(other.end_),
        start_(other.start_),
        handler_(other.handler_)
    {
    }

    iterator_connect_op(iterator_connect_op&& other)
      : base_from_connect_condition<ConnectCondition>(other),
        socket_(other.socket_),
        iter_(other.iter_),
        end_(other.end_),
        start_(other.start_),
        handler_(NET_TS_MOVE_CAST(IteratorConnectHandler)(other.handler_))
    {
    }
#endif // defined(NET_TS_HAS_MOVE)

    void operator()(std::error_code ec, int start = 0)
    {
      switch (start_ = start)
      {
        case 1:
        for (;;)
        {
          this->check_condition(ec, iter_, end_);

          if (iter_ != end_)
          {
            socket_.close(ec);
            socket_.async_connect(*iter_,
                NET_TS_MOVE_CAST(iterator_connect_op)(*this));
            return;
          }

          if (start)
          {
            ec = std::experimental::net::v1::error::not_found;
            std::experimental::net::v1::post(socket_.get_executor(),
                detail::bind_handler(
                  NET_TS_MOVE_CAST(iterator_connect_op)(*this), ec));
            return;
          }

          default:

          if (iter_ == end_)
            break;

          if (!socket_.is_open())
          {
            ec = std::experimental::net::v1::error::operation_aborted;
            break;
          }

          if (!ec)
            break;

          ++iter_;
        }

        handler_(static_cast<const std::error_code&>(ec),
            static_cast<const Iterator&>(iter_));
      }
    }

  //private:
    basic_socket<Protocol NET_TS_SVC_TARG>& socket_;
    Iterator iter_;
    Iterator end_;
    int start_;
    IteratorConnectHandler handler_;
  };

  template <typename Protocol NET_TS_SVC_TPARAM, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline void* networking_ts_handler_allocate(std::size_t size,
      iterator_connect_op<Protocol NET_TS_SVC_TARG, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    return networking_ts_handler_alloc_helpers::allocate(
        size, this_handler->handler_);
  }

  template <typename Protocol NET_TS_SVC_TPARAM, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline void networking_ts_handler_deallocate(void* pointer, std::size_t size,
      iterator_connect_op<Protocol NET_TS_SVC_TARG, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    networking_ts_handler_alloc_helpers::deallocate(
        pointer, size, this_handler->handler_);
  }

  template <typename Protocol NET_TS_SVC_TPARAM, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline bool networking_ts_handler_is_continuation(
      iterator_connect_op<Protocol NET_TS_SVC_TARG, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    return networking_ts_handler_cont_helpers::is_continuation(
        this_handler->handler_);
  }

  template <typename Function, typename Protocol
      NET_TS_SVC_TPARAM, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline void networking_ts_handler_invoke(Function& function,
      iterator_connect_op<Protocol NET_TS_SVC_TARG, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    networking_ts_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }

  template <typename Function, typename Protocol
      NET_TS_SVC_TPARAM, typename Iterator,
      typename ConnectCondition, typename IteratorConnectHandler>
  inline void networking_ts_handler_invoke(const Function& function,
      iterator_connect_op<Protocol NET_TS_SVC_TARG, Iterator,
        ConnectCondition, IteratorConnectHandler>* this_handler)
  {
    networking_ts_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }
} // namespace detail

#if !defined(GENERATING_DOCUMENTATION)

template <typename Protocol NET_TS_SVC_TPARAM,
    typename EndpointSequence, typename ConnectCondition,
    typename RangeConnectHandler, typename Allocator>
struct associated_allocator<
    detail::range_connect_op<Protocol NET_TS_SVC_TARG,
      EndpointSequence, ConnectCondition, RangeConnectHandler>,
    Allocator>
{
  typedef typename associated_allocator<
      RangeConnectHandler, Allocator>::type type;

  static type get(
      const detail::range_connect_op<Protocol NET_TS_SVC_TARG,
        EndpointSequence, ConnectCondition, RangeConnectHandler>& h,
      const Allocator& a = Allocator()) NET_TS_NOEXCEPT
  {
    return associated_allocator<RangeConnectHandler,
        Allocator>::get(h.handler_, a);
  }
};

template <typename Protocol NET_TS_SVC_TPARAM,
    typename EndpointSequence, typename ConnectCondition,
    typename RangeConnectHandler, typename Executor>
struct associated_executor<
    detail::range_connect_op<Protocol NET_TS_SVC_TARG,
      EndpointSequence, ConnectCondition, RangeConnectHandler>,
    Executor>
{
  typedef typename associated_executor<
      RangeConnectHandler, Executor>::type type;

  static type get(
      const detail::range_connect_op<Protocol NET_TS_SVC_TARG,
        EndpointSequence, ConnectCondition, RangeConnectHandler>& h,
      const Executor& ex = Executor()) NET_TS_NOEXCEPT
  {
    return associated_executor<RangeConnectHandler,
        Executor>::get(h.handler_, ex);
  }
};

template <typename Protocol NET_TS_SVC_TPARAM,
    typename Iterator, typename ConnectCondition,
    typename IteratorConnectHandler, typename Allocator>
struct associated_allocator<
    detail::iterator_connect_op<Protocol NET_TS_SVC_TARG, Iterator,
      ConnectCondition, IteratorConnectHandler>,
    Allocator>
{
  typedef typename associated_allocator<
      IteratorConnectHandler, Allocator>::type type;

  static type get(
      const detail::iterator_connect_op<Protocol NET_TS_SVC_TARG,
        Iterator, ConnectCondition, IteratorConnectHandler>& h,
      const Allocator& a = Allocator()) NET_TS_NOEXCEPT
  {
    return associated_allocator<IteratorConnectHandler,
        Allocator>::get(h.handler_, a);
  }
};

template <typename Protocol NET_TS_SVC_TPARAM,
    typename Iterator, typename ConnectCondition,
    typename IteratorConnectHandler, typename Executor>
struct associated_executor<
    detail::iterator_connect_op<Protocol NET_TS_SVC_TARG, Iterator,
      ConnectCondition, IteratorConnectHandler>,
    Executor>
{
  typedef typename associated_executor<
      IteratorConnectHandler, Executor>::type type;

  static type get(
      const detail::iterator_connect_op<Protocol NET_TS_SVC_TARG,
        Iterator, ConnectCondition, IteratorConnectHandler>& h,
      const Executor& ex = Executor()) NET_TS_NOEXCEPT
  {
    return associated_executor<IteratorConnectHandler,
        Executor>::get(h.handler_, ex);
  }
};

#endif // !defined(GENERATING_DOCUMENTATION)

template <typename Protocol NET_TS_SVC_TPARAM,
    typename EndpointSequence, typename RangeConnectHandler>
inline NET_TS_INITFN_RESULT_TYPE(RangeConnectHandler,
    void (std::error_code, typename Protocol::endpoint))
async_connect(basic_socket<Protocol NET_TS_SVC_TARG>& s,
    const EndpointSequence& endpoints,
    NET_TS_MOVE_ARG(RangeConnectHandler) handler,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a RangeConnectHandler.
  NET_TS_RANGE_CONNECT_HANDLER_CHECK(
      RangeConnectHandler, handler, typename Protocol::endpoint) type_check;

  async_completion<RangeConnectHandler,
    void (std::error_code, typename Protocol::endpoint)>
      init(handler);

  detail::range_connect_op<Protocol NET_TS_SVC_TARG, EndpointSequence,
    detail::default_connect_condition,
      NET_TS_HANDLER_TYPE(RangeConnectHandler,
        void (std::error_code, typename Protocol::endpoint))>(s,
          endpoints, detail::default_connect_condition(),
            init.completion_handler)(std::error_code(), 1);

  return init.result.get();
}

template <typename Protocol NET_TS_SVC_TPARAM,
    typename Iterator, typename IteratorConnectHandler>
inline NET_TS_INITFN_RESULT_TYPE(IteratorConnectHandler,
    void (std::error_code, Iterator))
async_connect(basic_socket<Protocol NET_TS_SVC_TARG>& s,
    Iterator begin, Iterator end,
    NET_TS_MOVE_ARG(IteratorConnectHandler) handler)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a IteratorConnectHandler.
  NET_TS_ITERATOR_CONNECT_HANDLER_CHECK(
      IteratorConnectHandler, handler, Iterator) type_check;

  async_completion<IteratorConnectHandler,
    void (std::error_code, Iterator)> init(handler);

  detail::iterator_connect_op<Protocol NET_TS_SVC_TARG, Iterator,
    detail::default_connect_condition, NET_TS_HANDLER_TYPE(
      IteratorConnectHandler, void (std::error_code, Iterator))>(s,
        begin, end, detail::default_connect_condition(),
          init.completion_handler)(std::error_code(), 1);

  return init.result.get();
}

template <typename Protocol NET_TS_SVC_TPARAM, typename EndpointSequence,
    typename ConnectCondition, typename RangeConnectHandler>
inline NET_TS_INITFN_RESULT_TYPE(RangeConnectHandler,
    void (std::error_code, typename Protocol::endpoint))
async_connect(basic_socket<Protocol NET_TS_SVC_TARG>& s,
    const EndpointSequence& endpoints, ConnectCondition connect_condition,
    NET_TS_MOVE_ARG(RangeConnectHandler) handler,
    typename enable_if<is_endpoint_sequence<
        EndpointSequence>::value>::type*)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a RangeConnectHandler.
  NET_TS_RANGE_CONNECT_HANDLER_CHECK(
      RangeConnectHandler, handler, typename Protocol::endpoint) type_check;

  async_completion<RangeConnectHandler,
    void (std::error_code, typename Protocol::endpoint)>
      init(handler);

  detail::range_connect_op<Protocol NET_TS_SVC_TARG, EndpointSequence,
    ConnectCondition, NET_TS_HANDLER_TYPE(RangeConnectHandler,
      void (std::error_code, typename Protocol::endpoint))>(s,
        endpoints, connect_condition, init.completion_handler)(
          std::error_code(), 1);

  return init.result.get();
}

template <typename Protocol NET_TS_SVC_TPARAM, typename Iterator,
    typename ConnectCondition, typename IteratorConnectHandler>
inline NET_TS_INITFN_RESULT_TYPE(IteratorConnectHandler,
    void (std::error_code, Iterator))
async_connect(basic_socket<Protocol NET_TS_SVC_TARG>& s,
    Iterator begin, Iterator end, ConnectCondition connect_condition,
    NET_TS_MOVE_ARG(IteratorConnectHandler) handler)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a IteratorConnectHandler.
  NET_TS_ITERATOR_CONNECT_HANDLER_CHECK(
      IteratorConnectHandler, handler, Iterator) type_check;

  async_completion<IteratorConnectHandler,
    void (std::error_code, Iterator)> init(handler);

  detail::iterator_connect_op<Protocol NET_TS_SVC_TARG, Iterator,
    ConnectCondition, NET_TS_HANDLER_TYPE(
      IteratorConnectHandler, void (std::error_code, Iterator))>(s,
        begin, end, connect_condition, init.completion_handler)(
          std::error_code(), 1);

  return init.result.get();
}

} // inline namespace v1
} // namespace net
} // namespace experimental
} // namespace std

#include <experimental/__net_ts/detail/pop_options.hpp>

#endif // NET_TS_IMPL_CONNECT_HPP
