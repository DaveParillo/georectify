#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <string>
#include <utility>

#include "listener.h"
#include "session.h"
#include "util.h"

using tcp = boost::asio::ip::tcp;

listener::listener(
    boost::asio::io_context& ioc,
    tcp::endpoint endpoint,
    std::shared_ptr<std::string const> const& doc_root)
  : acceptor_(ioc)
  , socket_(ioc)
  , doc_root_(doc_root)
{
  boost::system::error_code ec;

  // Open the acceptor
  acceptor_.open(endpoint.protocol(), ec);
  if(ec) {
    fail(ec, "open");
    return;
  }

  // Allow address reuse
  acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if(ec) {
    fail(ec, "set_option");
    return;
  }

  // Bind to the server address
  acceptor_.bind(endpoint, ec);
  if(ec) {
    fail(ec, "bind");
    return;
  }

  // Start listening for connections
  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
  if(ec) {
    fail(ec, "listen");
    return;
  }
}



#include <boost/asio/yield.hpp>
void
listener::loop(boost::system::error_code ec) {
  reenter(*this) {
    for(;;) {
      yield acceptor_.async_accept(
          socket_,
          std::bind(
            &listener::loop,
            shared_from_this(),
            std::placeholders::_1));
      if(ec) {
        fail(ec, "accept");
      } else {
        // Create the session and run it
        std::make_shared<session>(
            std::move(socket_),
            doc_root_)->run();
      }
    }
  }
}
#include <boost/asio/unyield.hpp>


