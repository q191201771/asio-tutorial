#ifndef _COMMON_PARTICIPANT_HPP_
#define _COMMON_PARTICIPANT_HPP_

#include "message.hpp"

class chat_participant;
typedef std::shared_ptr<chat_participant> chat_participant_ptr;

class chat_participant {
public:
    virtual ~chat_participant() {}
    virtual void deliver(chat_message_ptr msg) = 0;
};

#endif

