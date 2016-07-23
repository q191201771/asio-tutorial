#ifndef _SERVER2_SERVER_HPP_
#define _SERVER2_SERVER_HPP_

#include "../common/index.hpp"
#include "room.hpp"
#include "session.hpp"

#define CHECK_ERROR_CODE(ec) do { if (ec) { CHEF_LOG(fatal) << ec.message(); return false; } } while(0);

class chat_server
    : public std::enable_shared_from_this<chat_server>
{
public:
    chat_server(boost::asio::io_service &ios)
        : ios_(ios)
        , acceptor_(ios)
        , socket_(ios)
        , timer_(ios)
        , timer_tick_(0)
    {
    }

    ~chat_server() {
        CHEF_LOG(debug) << "~chat_server";
    }

    chat_server(const chat_server &)            = delete;
    chat_server &operator=(const chat_server &) = delete;

    /// non-block func
    bool start(const std::string &host, uint16_t port) {
        boost::system::error_code ec;
        boost::asio::ip::address addr = boost::asio::ip::address::from_string(host, ec);
        CHECK_ERROR_CODE(ec);
        acceptor_.open(tcp::v4(), ec);
        CHECK_ERROR_CODE(ec);
        acceptor_.set_option(tcp::acceptor::reuse_address(true));
        tcp::endpoint ep(addr, port);
        acceptor_.bind(ep, ec);
        CHECK_ERROR_CODE(ec);
        acceptor_.listen(1024, ec);
        CHECK_ERROR_CODE(ec);

        do_timer();
        do_accept();
        return true;
    }

private:
    void do_timer() {
        timer_.expires_from_now(boost::posix_time::seconds(2));
        timer_.async_wait(boost::bind(
            &chat_server::timer_handler,
            shared_from_this(),
            _1));
    }

    void timer_handler(const boost::system::error_code &ec) {
        if (ec) {
            CHEF_LOG(fatal) << "timer_handler ec: " << ec.message();
            return;
        }

        ++timer_tick_;
        room_.dump();
        CHEF_LOG(info) << statistics::instance().stringify();

        do_timer();
    }

    void do_accept() {
        auto self(shared_from_this());
        acceptor_.async_accept(
            socket_,
            [this, self](boost::system::error_code ec) {
                /// maybe error::no_descriptors
                if (ec) {
                    CHEF_LOG(fatal) << "accept handler ec: " << ec.message();
                    room_.dump();
                } else {
                    statistics::instance().increment_accept();

                    auto session = std::make_shared<chat_session>(
                        std::move(socket_),
                        room_,
                        std::make_shared<boost::asio::deadline_timer>(ios_));
                    session->start();
                }

                do_accept();
            });
    }

private:
    boost::asio::io_service    &ios_;
    tcp::acceptor               acceptor_;
    tcp::socket                 socket_;
    chat_room                   room_;
    boost::asio::deadline_timer timer_;
    uint64_t                    timer_tick_;
};

#endif

