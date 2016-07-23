#ifndef _COMMON_COMMON_HPP_
#define _COMMON_COMMON_HPP_

#include <boost/asio.hpp>
using boost::asio::ip::tcp;
typedef std::shared_ptr<boost::asio::deadline_timer> timer_ptr;
typedef std::shared_ptr<tcp::socket>         socket_ptr;

#include <memory>
#include <thread>
#include <mutex>
#include <functional>
#include <iostream>
#include <list>
#include <set>
#include <assert.h>

//#define PRINT(x) do { std::cout << __FUNCTION__ << ":" << __LINE__ << " " << x << "\n"; } while(0);

#endif

