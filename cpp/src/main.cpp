#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/system/error_code.hpp>


#include "listener.h"

namespace ip = boost::asio::ip;
using tcp = ip::tcp;
using std::string;


string usage(const char* name) 
{
	string msg = "Usage: ";
	return msg.append(name).append(" [-h] [-b address] [-p port] [-r root data dir] [-t # threads]\n");
}

string help (const char* name) 
{
  auto msg = usage(name);
  constexpr auto text = R"(
Options:
  -h   Show this text
  -b   Bind to address range. Default = 0.0.0.0
  -p   Listen port.  Default = 8080
  -r   Root of server data dir.  Default = /tmp
  -t   Thread pool size. Default = 1
)";

  return msg.append(text);
}

[[ noreturn ]] void die(const string& prompt) 
{
  std::cerr << prompt << '\n';
  exit(EXIT_FAILURE);
}

struct options {
  ip::address address = ip::address_v4::any();
  uint16_t port = 8080;
  std::shared_ptr<std::string> doc_root = std::make_shared<string>("/tmp");
  int pool_size = 1;
};

std::ostream& operator<<(std::ostream& os, const options& rhs)
{
  os << "address:\t" << rhs.address << '\n'
     << "port:\t\t" << rhs.port << '\n'
     << "doc dir:\t" << rhs.doc_root->c_str() << '\n'
     << "# threads:\t" << rhs.pool_size << '\n';
  return os;
}

options make_options(int argc, char* argv[]);

int main(int argc, char* argv[])
{
  const auto opt = make_options(argc, argv);
  std::cout << "Settings:\n" << opt << '\n';
  boost::asio::io_context ioc{opt.pool_size};
  boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

  signals.async_wait(
      [&ioc](boost::system::error_code const&, int sig) {
        std::cout << "\nCapture "
          << (sig == SIGINT ? "SIGINT" : "SIGTERM")
          << ", stop\n";
        ioc.stop();
    });

  // TODO: verify this bind is sucessful or die
  std::make_shared<listener>(ioc, tcp::endpoint{opt.address, opt.port}, opt.doc_root)->run();

  std::vector<std::thread> workers;
    workers.reserve(opt.pool_size - 1);
    for(auto i = 0; i < opt.pool_size; ++i) {
      workers.emplace_back(
          [&ioc] {
            ioc.run();
          });
    }
    ioc.run();

  // block until all threads exit
  for(auto& t : workers) t.join();

  return EXIT_SUCCESS;
}

options make_options(int argc, char* argv[])
{
  using std::strcmp;
  using std::atoi;
  options opt;
  for (int i=1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0) {
      die(help(argv[0]));
    } else if (strcmp(argv[i], "-b") == 0) {
      ++i;
      if (i < argc) {
        try {
          opt.address = ip::make_address(argv[i]);
        } catch (boost::system::system_error const& e) {
          std::cerr << "Error binding to address " << argv[i] << ".\n";
          std::cerr << e.what() << ": " << e.code() << " - " << e.code().message() << '\n';
          die("Exiting");
        } catch (...) {
          die("Problem binding to address");
        }
      } else {
        std::cerr << usage(argv[0]);
        die("Must specify a bind address for the -b argument");
      }
    } else if (strcmp(argv[i], "-p") == 0) {
      ++i;
      if (i < argc) {
        opt.port = static_cast<unsigned short>(atoi(argv[i]));
      } else {
        std::cerr << usage(argv[0]);
        die("Must specify a port number for the -p argument");
      }
    } else if (strcmp(argv[i], "-r") == 0) {
      ++i;
      if (i < argc) {
        opt.doc_root = std::make_shared<string>(argv[i]);
      } else {
        std::cerr << usage(argv[0]);
        die("Must specify a location for the -r argument");
      }
    } else if (strcmp(argv[i], "-t") == 0) {
      ++i;
      if (i < argc) {
        opt.pool_size = std::max<int>(1, atoi(argv[i]));
      } else {
        std::cerr << usage(argv[0]);
        die("Must specify number of threads for the -t argument");
      }
    } else {
      std::cerr << usage(argv[0]);
      die("unsupported argument");
    }
  }
  const uint32_t max_pool = std::max<int>(1, std::thread::hardware_concurrency()*2);
  if (opt.pool_size > int(max_pool)) {
    std::cerr << "Too many threads set. Setting to " << max_pool << '\n';
    opt.pool_size = max_pool;
  }

  return opt;
}

