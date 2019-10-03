#ifndef GEORECTIFY_LISTENER_H
#define GEORECTIFY_LISTENER_H

#include <algorithm>
#include <memory>
#include <string>

#include <boost/asio/coroutine.hpp>
#include <boost/asio/ip/tcp.hpp>

class listener
  : public boost::asio::coroutine
  , public std::enable_shared_from_this<listener>
{
  using tcp = boost::asio::ip::tcp;
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  std::shared_ptr<std::string const> doc_root_;
  bool alive_ = false;

  public:
  listener(
      boost::asio::io_context& ioc,
      tcp::endpoint endpoint,
      std::shared_ptr<std::string const> const& doc_root);

  bool alive() const {return alive_;};

  void run() {
    if(acceptor_.is_open() && alive_) loop();
  }

  void loop(boost::system::error_code ec = {});
};

#endif

