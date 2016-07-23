#ifndef _COMMON_MESSAGE_HPP_
#define _COMMON_MESSAGE_HPP_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <memory>

class chat_message;
typedef std::shared_ptr<chat_message> chat_message_ptr;
typedef std::deque<chat_message_ptr> chat_message_queue;

class chat_message {
public:
    enum { HEADER_LENGTH = 4 };
    enum { MAX_BODY_LENGTH = 512 };

    chat_message()
        : body_length_(0)
    {}

    chat_message(const chat_message &)            = delete;
    chat_message &operator=(const chat_message &) = delete;

    chat_message_ptr clone() const {
        auto obj = std::make_shared<chat_message>();
        obj->body_length(this->body_length_);
        memcpy(obj->data_, data_, HEADER_LENGTH + MAX_BODY_LENGTH);
        return obj;
    }

    const char *data() const { return data_; }
    char *data() { return data_; }

    std::size_t length() const {
        return HEADER_LENGTH + body_length_;
    }

    const char *body() const {
        return data_ + HEADER_LENGTH;
    }
    char *body() {
        return data_ + HEADER_LENGTH;
    }

    std::size_t body_length() const {
        return body_length_;
    }
    void body_length(std::size_t new_length) {
        body_length_ = new_length > MAX_BODY_LENGTH
            ? MAX_BODY_LENGTH
            : new_length;
    }

    bool decode_header() {
        char header[HEADER_LENGTH + 1] = "";
        std::strncat(header, data_, HEADER_LENGTH);
        body_length_ = std::atoi(header);
        if (body_length_ > MAX_BODY_LENGTH) {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    void encode_header() {
        char header[HEADER_LENGTH + 1] = "";
        std::sprintf(header, "%4d", static_cast<int>(body_length_));
        std::memcpy(data_, header, HEADER_LENGTH);
    }

private:
    char data_[HEADER_LENGTH + MAX_BODY_LENGTH];
    std::size_t body_length_;
};

#endif

