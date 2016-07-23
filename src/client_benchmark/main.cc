#include "client.hpp"

void signal_handler(boost::asio::io_service &ios) {
    std::cout << "recv signal,> io service stop.\n";
    ios.stop();
}

int main(int argc, char *argv[]) {
    try {
        if (argc != 4) {
            std::cerr << "Usage client_benchmark <host> <port> <num of clients>\n";
            return 1;
        }
        std::string host        = argv[1];
        std::string port        = argv[2];
        uint16_t num_of_threads = std::atoi(argv[3]);

        chef::log::init(chef::log::mode_debug);
        std::cout << "< init log.\n";

        chat_client::init_mock_msg();
        std::cout << "< init mock msg.\n";

        boost::asio::io_service ios;

        boost::asio::signal_set signals(ios);
        signals.add(SIGINT);
        signals.add(SIGTERM);
#if defined(SIGQUIT)
        signals.add(SIGQUIT);
#endif
        signals.async_wait(std::bind(&signal_handler, std::ref(ios)));
        std::cout << "< init signal handler.\n";

        std::vector<chat_client_ptr> clients;

        for (uint16_t i = 0; i < num_of_threads; i++) {
            auto client = std::make_shared<chat_client>(ios, host, port);
            client->connect();
            clients.push_back(client);
        }
        std::cout << "< clients async connect out.\n";

        std::cout << "> io service run.\n";
        std::thread t([&ios]() { ios.run(); });

        t.join();

    } catch(std::exception e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    std::cout << "< bye.\n";
    return 0;
}

