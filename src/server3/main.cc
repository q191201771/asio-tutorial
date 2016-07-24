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
        if (!server->start("0.0.0.0", port)) {
            CHEF_LOG(fatal) << "start server fail,bye.";
            return 1;
        }
        CHEF_LOG(info) << "< start server.";

        CHEF_LOG(info) << "> run server.";
        server->run();
        while(server.use_count() != 1) {
            sleep(1);
        }
        /// Oops,I'm chat_server's last maintainer. time to stop server~
        CHEF_LOG(info) << "> stop server.";
        server->stop();
    } catch(std::exception e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    CHEF_LOG(info) << "bye.";
    return 0;
}
