#ifndef _SERVER1_ROOM_HPP_
#define _SERVER1_ROOM_HPP_

#include "../common/index.hpp"

class chat_room {
public:
    chat_room()
        : delivered_msg_count_(0)
        , total_participant_count_(0)
    {}

    ~chat_room() {
        CHEF_LOG(debug) << "~chat_room()";
    }

    chat_room(const chat_room &)            = delete;
    chat_room &operator=(const chat_room &) = delete;

    void join(chat_participant_ptr participant) {
        ++total_participant_count_;
        participants_.insert(participant);
        for (auto msg : recent_msgs_) {
            participant->deliver(msg);
        }
    }

    void leave(chat_participant_ptr participant) {
        participants_.erase(participant);
    }

    void deliver(chat_message_ptr msg) {
        recent_msgs_.push_back(msg);
        while(recent_msgs_.size() > MAX_RECENT_MSGS) {
            recent_msgs_.pop_front();
        }

        for (auto participant : participants_) {
            ++delivered_msg_count_;
            participant->deliver(msg);
        }
    }

    void dump() {
        CHEF_LOG(info) << "\n"
            << "-----dump-----\n"
            << "recent msg size:         " << recent_msgs_.size()  << "\n"
            << "online participant size: " << participants_.size() << "\n"
            << "total participant size:  " << total_participant_count_ << "\n"
            << "delivered count:         " << delivered_msg_count_ << "\n"
            << "--------------";
    }

private:
    enum { MAX_RECENT_MSGS = 100 };

    std::set<chat_participant_ptr> participants_;
    chat_message_queue             recent_msgs_;
    uint64_t                       delivered_msg_count_;
    uint64_t                       total_participant_count_;
};

#endif

