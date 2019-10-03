#ifndef GEORECTIFY_SESSION_H
#define GEORECTIFY_SESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;


// Handles an HTTP server connection
class session
  : public boost::asio::coroutine
  , public std::enable_shared_from_this<session>
{
  public:
    session() = delete;

    // Take ownership of the socket
    explicit
      session(tcp::socket socket,
          const std::shared_ptr<const std::string>& doc_root)
      : socket_(std::move(socket))
        , strand_(socket_.get_executor())
        , doc_root_(doc_root)
        , send_(*this)
    { }

    // Start the asynchronous operation
    void run() {
      loop({}, 0, false);
    }

    void loop(boost::system::error_code ec, size_t bytes_transferred, bool close);

  private:
    // A function object used to send an HTTP message.
    struct sender
    {
      session& self_;
      std::shared_ptr<void> res_;

      explicit sender(session& self) : self_(self) { }

      template<bool isRequest, class Body, class Fields>
        void
        operator()(http::message<isRequest, Body, Fields>&& msg) const
        {
          // The lifetime of the message has to extend
          // for the duration of the async operation so
          // we use a shared_ptr to manage it.
          auto sp = std::make_shared<
            http::message<isRequest, Body, Fields>>(std::move(msg));

          // Store a type-erased version of the shared
          // pointer in the class to keep it alive.
          self_.res_ = sp;

          // Write the response
          http::async_write(
              self_.socket_,
              *sp,
              boost::asio::bind_executor(
                self_.strand_,
                std::bind(
                  &session::loop,
                  self_.shared_from_this(),
                  std::placeholders::_1,
                  std::placeholders::_2,
                  sp->need_eof())));
        }
    };

    tcp::socket socket_;
    boost::asio::strand<
      boost::asio::io_context::executor_type> strand_;
    boost::beast::flat_buffer buffer_;
    std::shared_ptr<const std::string> doc_root_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    sender send_;
};

#endif

