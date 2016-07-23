#include "../common/index.hpp"

void signal_handler(boost::asio::io_service &ios) {
    CHEF_LOG(info) << "recv signal,> io service stop.";
    ios.stop();
}

class chat_client;
typedef std::shared_ptr<chat_client> chat_client_ptr;

class chat_client
    : public std::enable_shared_from_this<chat_client>
{
public:
    chat_client(boost::asio::io_service &ios, const std::string &host, const std::string &port)
        : ios_(ios)
        , socket_(ios)
        , ok_(true)
    {
        tcp::resolver resolver(ios);
        iter_ = resolver.resolve({host, port});
    }

    ~chat_client() {}
    chat_client(const chat_client &)            = delete;
    chat_client &operator=(const chat_client &) = delete;

    void connect() {
        std::cout << "----- connecting.\n";
        boost::asio::async_connect(
            socket_,
            iter_,
            [this](boost::system::error_code ec, tcp::resolver::iterator) {
                if (ec) {
                    /// maybe error connection_refused
                    std::cout << "----- connect fail, ec: " << ec.message() << "\n";
                    ok_ = false;
                    return;
                }

                std::cout << "----- connect succ.\n";
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

    bool ok() const { return ok_; }

private:
    void do_read_header() {
        boost::asio::async_read(
            socket_,
            boost::asio::buffer(read_msg_.data(), chat_message::HEADER_LENGTH),
            [this](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    if (ec != boost::asio::error::operation_aborted) {
                        std::cout << "----- closed by peer.\n";
                        ok_ = false;
                        socket_.close();
                    }
                    return;
                }

                if (!read_msg_.decode_header()) {
                    std::cout << "----- decode header fail.\n";
                    ok_ = false;
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
                    if (ec != boost::asio::error::operation_aborted) {
                        std::cout << "----- closed by peer.\n";
                        ok_ = false;
                        socket_.close();
                    }
                    return;
                }

                std::cout << "----- recv: ";
                std::cout.write(read_msg_.body(), read_msg_.body_length());
                std::cout << "\n";
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
                    if (ec != boost::asio::error::operation_aborted) {
                        std::cout << "----- closed by peer.\n";
                        ok_ = false;
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
    boost::asio::io_service &ios_;
    tcp::socket              socket_;
    tcp::resolver::iterator  iter_;
    chat_message             read_msg_;
    chat_message_queue       write_msgs_;
    bool                     ok_;
};

int main(int argc, char *argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage client_stdin <host> <port>\n";
            return 1;
        }
        std::string host = argv[1];
        std::string port = argv[2];

        boost::asio::io_service ios;

        boost::asio::signal_set signals(ios);
        signals.add(SIGINT);
        signals.add(SIGTERM);
#if defined(SIGQUIT)
        signals.add(SIGQUIT);
#endif
        signals.async_wait(std::bind(&signal_handler, std::ref(ios)));

        auto c = std::make_shared<chat_client>(ios, host, port);
        c->connect();

        std::thread t([&ios]() { ios.run(); });

        char line[chat_message::MAX_BODY_LENGTH + 1];
        while (std::cin.getline(line, chat_message::MAX_BODY_LENGTH + 1) && c->ok()) {
            chat_message_ptr msg = std::make_shared<chat_message>();
            msg->body_length(std::strlen(line));
            std::memcpy(msg->body(), line, msg->body_length());
            msg->encode_header();
            c->write(msg);
        }
        std::cout << "----- out of input.\n";

        ios.stop();
        t.join();

    } catch(std::exception e) {
        std::cerr << "----- Exception: " << e.what() << "\n";
    }

    std::cout << "----- bye.\n";
    return 0;
}
