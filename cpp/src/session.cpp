#include <boost/beast/http.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <string>
#include <utility>

#include "handler.h"
#include "session.h"
#include "util.h"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

#include <boost/asio/yield.hpp>

void session::loop(
    boost::system::error_code ec,
    size_t bytes_transferred,
    bool close)
{
  boost::ignore_unused(bytes_transferred);
  reenter(*this)
  {
    for(;;)
    {
      // Make the request empty before reading,
      // otherwise the operation behavior is undefined.
      req_ = {};

      // Read a request
      yield http::async_read(socket_, buffer_, req_,
          boost::asio::bind_executor(
            strand_,
            std::bind(
              &session::loop,
              shared_from_this(),
              std::placeholders::_1,
              std::placeholders::_2,
              false)));
      // The remote host closed the connection
      if(ec == http::error::end_of_stream) {
        break;
      }
      if(ec) {
        return fail(ec, "read");
      }

      // Send the response
      yield handle_request(*doc_root_, std::move(req_), send_);
      if(ec) {
        return fail(ec, "write");
      }

      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      if(close)
      {
        break;
      }

      // We're done with the response so delete it
      res_ = nullptr;
    }

    socket_.shutdown(tcp::socket::shutdown_send, ec);
  }
}

#include <boost/asio/unyield.hpp>


