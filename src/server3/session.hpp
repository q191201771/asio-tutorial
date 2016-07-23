#ifndef _SERVER2_SESSION_HPP_
#define _SERVER2_SESSION_HPP_

#include "../common/index.hpp"
#include "room.hpp"

static const bool CONFIG_CLEAR_IDLE_SESSION      = true;
static const uint64_t CONFIG_ACTIVE_DURATION_SEC = 10;

class chat_session;
typedef std::shared_ptr<chat_session> chat_session_ptr;

class chat_session
    : public chat_participant
    , public std::enable_shared_from_this<chat_session>
{
public:
    enum session_status {
        STATUS_CONNECTED = 0,
        STATUS_ERROR     = 1,
        STATUS_CLOSED    = 2,
    };

    chat_session(boost::asio::io_service &ios, chat_room &room)
        : ios_(ios)
        , socket_(ios)
        , timer_(ios)
        , room_(room)
        , active_ts_(0)
        , status_(STATUS_CONNECTED)
        , close_by_peer_(false)
    {
        mark_active();
    }

    ~chat_session() {
        if (close_by_peer_) {
            statistics::instance().increment_close_by_peer();
        } else {
            statistics::instance().increment_close_by_self();
        }
    }

    chat_session(const chat_session &)            = delete;
    chat_session &operator=(const chat_session &) = delete;

    void start() {
        room_.join(shared_from_this());

        do_timer();
        do_read_header();
    }

    void deliver(chat_message_ptr msg) {
        std::lock_guard<std::mutex> guard(mutex_);
        bool writing = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!writing) {
            do_write();
        }
    }

    tcp::socket &socket() { return socket_; }

private:
    void do_read_header() {
        std::lock_guard<std::mutex> guard(mutex_);
        mark_active();
        auto self(shared_from_this());
        boost::asio::async_read(
            socket_,
            boost::asio::buffer(read_msg_.data(), read_msg_.HEADER_LENGTH),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    if (ec != boost::asio::error::operation_aborted) {
                        /// maybe error connection_reset or eof
                        status(STATUS_ERROR);
                    }
                    return;
                }

                bool valid_msg = false;
                {
                    std::lock_guard<std::mutex> valid_guard(mutex_);
                    valid_msg = read_msg_.decode_header();
                }
                if (!valid_msg) {
                    close();
                    return;
                }

                do_read_body();
            });
    }

    void do_read_body() {
        std::lock_guard<std::mutex> guard(mutex_);
        mark_active();
        auto self(shared_from_this());
        boost::asio::async_read(
            socket_,
            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    if (ec != boost::asio::error::operation_aborted) {
                        /// maybe error connection_reset or eof
                        status(STATUS_ERROR);
                    }
                    return;
                }

                statistics::instance().increment_read();

                room_.deliver(read_msg_.clone());
                do_read_header();
            });
    }

    void do_write() {
        auto self(shared_from_this());
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(
                write_msgs_.front()->data(),
                write_msgs_.front()->length()),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    if (ec != boost::asio::error::operation_aborted) {
                        /// maybe error connection_reset or bad_descriptor or broken_pipe
                        status(STATUS_ERROR);
                    }
                    return;
                }

                statistics::instance().increment_write();

                std::lock_guard<std::mutex> guard(mutex_);
                write_msgs_.pop_front();
                if (!write_msgs_.empty()) {
                do_write();
                }
            });
    }

    void do_timer() {
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait(boost::bind(
            &chat_session::timer_handler,
            shared_from_this(),
            _1));
    }

    void timer_handler(const boost::system::error_code &ec) {
        if (ec) {
            CHEF_LOG(fatal) << "timer_handler ec: " << ec.message();
            return;
        }

        if (status() == STATUS_ERROR) {
            close_by_peer_ = true;

            close();
        } else if (status() == STATUS_CLOSED) {
            room_.leave(shared_from_this());
            return;
        }

        if (CONFIG_CLEAR_IDLE_SESSION) {
            auto now = std::time(0);
            if (!valid(now)) {
                close();
            }
        }

        do_timer();
    }

private:
    void close() {
        if (status_.exchange(STATUS_CLOSED) != STATUS_CLOSED) {
            std::lock_guard<std::mutex> guard(mutex_);
            socket_.close();
        }
    }

    void mark_active() {
        active_ts_ = std::time(0);
    }

    bool valid(uint64_t now) {
        return (now - active_ts_) < CONFIG_ACTIVE_DURATION_SEC;
    }

    session_status status() const { return status_.load(); }
    void status(session_status s) { status_.store(s); }

private:
    std::mutex                   mutex_;
    boost::asio::io_service     &ios_;
    tcp::socket                  socket_;
    boost::asio::deadline_timer  timer_;
    chat_room                   &room_;
    chat_message                 read_msg_;
    chat_message_queue           write_msgs_;
    uint64_t                     active_ts_;
    std::atomic<session_status>  status_;
    bool                         close_by_peer_;
};

#endif

