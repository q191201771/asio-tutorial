#ifndef _COMMON_STATISTICS_HPP_
#define _COMMON_STATISTICS_HPP_

#include <mutex>
#include <atomic>
#include <string>
#include <sstream>
#include <stdint.h>

class statistics {
public:
    static statistics &instance() {
        if (!instance_) {
            std::lock_guard<std::mutex> guard(mutex_);
            if (!instance_) {
                instance_ = new statistics();
            }
        }
        return *instance_;
    }

    void increment_accept()        { ++accept_count_; }
    void increment_connect()       { ++connect_count_; }
    void increment_read()          { ++read_count_; }
    void increment_write()         { ++write_count_; }
    void increment_close_by_self() { ++close_by_self_count_; }
    void increment_close_by_peer() { ++close_by_peer_count_; }

    std::string stringify() {
        std::stringstream ss;
        ss  << "\n-----statistics-----"
            << "\naccept_count: " << accept_count_
            << "\nconnect_count: " << connect_count_
            << "\nread_count: " << read_count_
            << "\nwrite_count: " << write_count_
            << "\nclose_by_self_count: " << close_by_self_count_
            << "\nclose_by_peer_count: " << close_by_peer_count_
            << "\n--------------------\n";
        return ss.str();
    }

private:
    statistics()
        : accept_count_(0)
        , connect_count_(0)
        , read_count_(0)
        , write_count_(0)
        , close_by_self_count_(0)
        , close_by_peer_count_(0)
    {}
    ~statistics();

    static std::mutex  mutex_;
    static statistics *instance_;

private:
    std::atomic<uint64_t> accept_count_;
    std::atomic<uint64_t> connect_count_;
    std::atomic<uint64_t> read_count_;
    std::atomic<uint64_t> write_count_;
    std::atomic<uint64_t> close_by_self_count_;
    std::atomic<uint64_t> close_by_peer_count_;
};

#endif

