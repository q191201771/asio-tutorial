#include "server.hpp"

void signal_handler(boost::asio::io_service &ios) {
    CHEF_LOG(info) << "recv signal,> io service stop.";
    ios.stop();
}

int main(int argc, char *argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: server2 <port> <num of thread>\n";
            return 1;
        }
        uint16_t port           = std::atoi(argv[1]);
        uint16_t num_of_threads = std::atoi(argv[2]);

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
        if (!server->start("0.0.0.0", port)) {
            CHEF_LOG(fatal) << "start server fail,bye.";
            return 1;
        }
        CHEF_LOG(info) << "< start server.";

        std::vector<std::shared_ptr<std::thread> > threads;
        for (int i = 0; i < num_of_threads; i++) {
            auto io_thread = std::make_shared<std::thread>([&ios]() {
                ios.run();
            });
            threads.push_back(io_thread);
        }
        CHEF_LOG(info) << "io threads running.";

        for (int i = 0; i < num_of_threads; i++) {
            threads[i]->join();
            CHEF_LOG(info) << "< thread " << i << " done.";
        }

    } catch(std::exception e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    CHEF_LOG(info) << "bye.";
    return 0;
}
