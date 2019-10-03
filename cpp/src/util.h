#ifndef GEORECTIFY_UTIL_H
#define GEORECTIFY_UTIL_H

#include <iostream>
#include <string>

#include <boost/beast/core.hpp>
#include <boost/system/error_code.hpp>

inline
void fail(const boost::system::error_code& ec, const char* what)
{
  std::cerr << what << ": " << ec.message() << '\n';
}

// Append an HTTP rel-path to a local *nix filesystem path.
inline std::string path_cat(
    boost::beast::string_view base,
    boost::beast::string_view path)
{
    if(base.empty())
        return path.to_string();
    std::string result = base.to_string();
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    return result;
}

#endif

