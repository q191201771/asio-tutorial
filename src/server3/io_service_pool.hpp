#ifndef _SERVER3_IO_SERVICE_POOL_HPP_
#define _SERVER3_IO_SERVICE_POOL_HPP_

#include "../common/index.hpp"

class io_service_pool {
public:
    io_service_pool(uint32_t size)
        : size_(size)
        , index_(0)
    {
        assert(size > 0);
        for (uint32_t i = 0; i < size; i++) {
            auto ios = std::make_shared<boost::asio::io_service>();
            auto work = std::make_shared<boost::asio::io_service::work>(*ios);
            io_service_pool_.push_back(ios);
            io_work_pool_.push_back(work);
        }
    }
    ~io_service_pool() {
        CHEF_LOG(info) << "~io_service_pool()";
    }

    io_service_pool(const io_service_pool &)            = delete;
    io_service_pool &operator=(const io_service_pool &) = delete;

    /// non-block func
    void run() {
        for (uint32_t i = 0; i < size_; i++) {
            auto ios = io_service_pool_[i];
            auto t = std::make_shared<std::thread>([ios]() {
                ios->run();
            });
            thread_pool_.push_back(t);
        }
        CHEF_LOG(info) << "io threads running.";
    }

    void join() {
        uint32_t size = thread_pool_.size();
        for (uint32_t i = 0; i < size; i++) {
            thread_pool_[i]->join();
            CHEF_LOG(info) << "< thread " << i << " done.";
        }
    }

    void stop() {
        uint32_t size = thread_pool_.size();
        for (uint32_t i = 0; i < size; i++) {
            io_service_pool_[i]->stop();
        }
    }

    /// round-robin
    boost::asio::io_service &fetch() {
        if (++index_ == size_) {
            index_ = 0;
        }
        boost::asio::io_service &ios = *(io_service_pool_[index_]);
        return ios;
    }

    boost::asio::io_service &fetch(uint32_t index) {
        assert(index <= size_);
        boost::asio::io_service &ios = *(io_service_pool_[index]);
        return ios;
    }

private:
    typedef std::shared_ptr<boost::asio::io_service>       io_service_ptr;
    typedef std::shared_ptr<boost::asio::io_service::work> io_work_ptr;
    typedef std::shared_ptr<std::thread>                   thread_ptr;

    uint32_t                    size_;
    std::vector<io_service_ptr> io_service_pool_;

    /// ensure ios'run() function will not exit while work is underway
    std::vector<io_work_ptr>    io_work_pool_;

    std::vector<thread_ptr>     thread_pool_;

    uint32_t                    index_;
};

#endif

