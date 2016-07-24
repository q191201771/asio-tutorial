#ifndef _SERVER2_SERVER_HPP_
#define _SERVER2_SERVER_HPP_

#include "../common/index.hpp"
#include "room.hpp"
#include "session.hpp"
#include "io_service_pool.hpp"

#define CHECK_ERROR_CODE(ec) do { if (ec) { CHEF_LOG(fatal) << ec.message(); return false; } } while(0);

class chat_server
    : public std::enable_shared_from_this<chat_server>
{
public:
    chat_server(uint16_t num_of_threads)
        : io_service_pool_(num_of_threads)
        , signals_(io_service_pool_.fetch(0))
        , acceptor_(io_service_pool_.fetch(0))
        , timer_(io_service_pool_.fetch(0))
        , timer_tick_(0)
        , new_session_()
    {
    }

    ~chat_server() {
        CHEF_LOG(info) << "~chat_server";
    }

    chat_server(const chat_server &)            = delete;
    chat_server &operator=(const chat_server &) = delete;

    /// non-block func
    bool start(const std::string &host, uint16_t port) {
        signals_.add(SIGINT);
        signals_.add(SIGTERM);
#if defined(SIGQUIT)
        signals_.add(SIGQUIT);
#endif
        signals_.async_wait(std::bind(&chat_server::signal_handler, shared_from_this()));
        CHEF_LOG(info) << "< init signal handler.";

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

    /// non-block func
    void run() {
        io_service_pool_.run();
    }

    void stop() {
        io_service_pool_.stop();
        io_service_pool_.join();
    }

private:
    void signal_handler() {
        CHEF_LOG(info) << "recv signal,> stop acceptor and timer.";
        acceptor_.close();
        timer_.cancel();
    }

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
        boost::asio::io_service &ios = io_service_pool_.fetch();
        new_session_.reset(new chat_session(
            ios,
            room_));

        auto self(shared_from_this());
        acceptor_.async_accept(
            new_session_->socket(),
            [this, self](boost::system::error_code ec) {
                /// maybe error bad_descriptor or no_descriptors
                if (ec) {
                    if (ec == boost::asio::error::bad_descriptor) {
                        return;
                    }
                    CHEF_LOG(fatal) << "accept handler ec: " << ec.message();
                    room_.dump();
                } else {
                    statistics::instance().increment_accept();

                    new_session_->start();
                }

                do_accept();
            });
    }

private:
    io_service_pool             io_service_pool_;
    boost::asio::signal_set     signals_;
    tcp::acceptor               acceptor_;
    boost::asio::deadline_timer timer_;
    uint64_t                    timer_tick_;
    chat_room                   room_;
    chat_session_ptr            new_session_;
};

#endif

