#include "server.hpp"

void signal_handler(boost::asio::io_service &ios) {
    CHEF_LOG(info) << "recv signal,> io service stop.";
    ios.stop();
}

int main(int argc, char *argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: server1 <port>\n";
            return 1;
        }
        uint16_t port = std::atoi(argv[1]);

        chef::log::init(chef::log::mode_debug);
        CHEF_LOG(info) << "< init log.";

        boost::asio::io_service ios;

        boost::asio::signal_set signals(ios);
        signals.add(SIGINT);
        signals.add(SIGTERM);
#if defined(SIGQUIT)
        signals.add(SIGQUIT);
#endif
        signals.async_wait(std::bind(&signal_handler, std::ref(ios)));
        CHEF_LOG(info) << "< init signal handler.";

        auto server = std::make_shared<chat_server>(ios);
        if (!server->start("127.0.0.1", port)) {
            CHEF_LOG(fatal) << "start server fail,bye.";
            return 1;
        }
        CHEF_LOG(info) << "< start server.";

        CHEF_LOG(info) << "> run io service.";
        ios.run();
    } catch(std::exception e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    CHEF_LOG(info) << "< bye.";
    return 0;
}
