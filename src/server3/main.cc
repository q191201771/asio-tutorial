//#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

#include "server.hpp"

int main(int argc, char *argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: server3 <port> <num of threads>\n";
            return 1;
        }
        uint16_t port           = std::atoi(argv[1]);
        uint16_t num_of_threads = std::atoi(argv[2]);

        chef::log::init(chef::log::mode_debug);
        CHEF_LOG(info) << "< init log.";

        auto server = std::make_shared<chat_server>(num_of_threads);
        if (!server->start("127.0.0.1", port)) {
            CHEF_LOG(fatal) << "start server fail,bye.";
            return 1;
        }
        CHEF_LOG(info) << "< start server.";

        CHEF_LOG(info) << "> run server.";
        server->run();
    } catch(std::exception e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    CHEF_LOG(info) << "bye.";
    return 0;
}
