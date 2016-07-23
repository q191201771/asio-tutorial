#ifndef _CLIENT_BENCHMARK_CLIENT_HPP_
#define _CLIENT_BENCHMARK_CLIENT_HPP_

#include "../common/index.hpp"

class chat_client;
typedef std::shared_ptr<chat_client> chat_client_ptr;

class chat_client
    : public std::enable_shared_from_this<chat_client>
{
public:
    chat_client(boost::asio::io_service &ios, const std::string &host, const std::string &port)
        : ios_(ios)
        , socket_(ios)
    {
        tcp::resolver resolver(ios);
        iter_ = resolver.resolve({host, port});
    }

    ~chat_client() {
        std::cout << "~chat_client()\n";
    }

    chat_client(const chat_client &)            = delete;
    chat_client &operator=(const chat_client &) = delete;

    void connect() {
        boost::asio::async_connect(
            socket_,
            iter_,
            [this](boost::system::error_code ec, tcp::resolver::iterator) {
                if (ec) {
                    /// maybe error connection_refused
                    std::cout << ec.message() << "\n";
                    return;
                }

                write(MOCK_MESSAGES[0]);
                write(MOCK_MESSAGES[1]);

                do_read_header();
            });
    }

    void write(chat_message_ptr msg) {
        ios_.post(
            [this, msg](){
                bool writing = !write_msgs_.empty();
                write_msgs_.push_back(msg);
                if (!writing) {
                    do_write();
                }
            });
    }

    void close() {
        ios_.post([this](){ socket_.close(); });
    }

public:
    static void init_mock_msg() {
        static std::vector<std::string> MOCK_MSG_BODYS = {"HELLO", "WORLD"};

        for (uint32_t i = 0; i < MOCK_MSG_BODYS.size(); i++) {
            auto mock_msg = std::make_shared<chat_message>();
            mock_msg->body_length(MOCK_MSG_BODYS[i].length());
            std::memcpy(mock_msg->body(), MOCK_MSG_BODYS[i].c_str(), mock_msg->body_length());
            mock_msg->encode_header();
            MOCK_MESSAGES.push_back(mock_msg);
        }
    }

private:
    void do_read_header() {
        boost::asio::async_read(
            socket_,
            boost::asio::buffer(read_msg_.data(), chat_message::HEADER_LENGTH),
            [this](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cout << ec.message() << "\n";
                    if (ec != boost::asio::error::operation_aborted) {
                        socket_.close();
                    }
                    return;
                }

                if (!read_msg_.decode_header()) {
                    socket_.close();
                    return;
                }

                do_read_body();
            });
    }

    void do_read_body() {
        boost::asio::async_read(
            socket_,
            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
            [this](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cout << ec.message() << "\n";
                    if (ec != boost::asio::error::operation_aborted) {
                        socket_.close();
                    }
                    return;
                }

                do_read_header();
            });
    }

    void do_write() {
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(
                write_msgs_.front()->data(),
                write_msgs_.front()->length()),
            [this](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cout << ec.message() << "\n";
                    if (ec != boost::asio::error::operation_aborted) {
                        socket_.close();
                    }
                    return;
                }

                write_msgs_.pop_front();
                if (!write_msgs_.empty()) {
                    do_write();
                }
            });
    }

private:
    static std::vector<chat_message_ptr> MOCK_MESSAGES;

private:
    boost::asio::io_service &ios_;
    tcp::socket              socket_;
    tcp::resolver::iterator  iter_;
    chat_message             read_msg_;
    chat_message_queue       write_msgs_;
};

#endif

