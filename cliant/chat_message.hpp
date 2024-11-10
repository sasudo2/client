//
// chat_message.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

class chat_message
{
public:
    static constexpr std::size_t header_length = 24;
    static constexpr std::size_t max_body_length = 512;
    static constexpr int array_of_msg_id[] = { 1, 2, 3, 4, 5 };
    // 1 = login, 2 = logout, 3 = assigne_reciever, 4 = send_message, 5 = to be determined

    chat_message()
        : body_length_(0)
    {
    }

    const char* data() const
    {
        return data_;
    }

    char* data()
    {
        return data_;
    }

    std::size_t length() const
    {
        return header_length + body_length_;
    }

    const char* body() const
    {
        return data_ + header_length;
    }

    char* body()
    {
        return data_ + header_length;
    }

    std::size_t body_length() const
    {
        return body_length_;
    }

    void body_length(std::size_t new_length)
    {
        body_length_ = new_length;
        if (body_length_ > max_body_length)
            body_length_ = max_body_length;
    }

    bool decode_header()
    {
        char header[header_length + 1] = "";
        std::strncat(header, data_, header_length);
        body_length_ = std::atoi(header);
        if (body_length_ > max_body_length)
        {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    void encode_header(double sender_id, double reciever_id, int _msg_id)
    {
        char header[header_length + 1] = "";
        std::sprintf(header, "%4d%4d%10.2f%10.2f", static_cast<int>(body_length_), _msg_id, sender_id, reciever_id);
        std::memcpy(data_, header, header_length);
    }

private:
    int msg_id;
    char data_[header_length + max_body_length];
    std::size_t body_length_;
};

#endif // CHAT_MESSAGE_HPP