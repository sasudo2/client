//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

//#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include "asio.hpp"
#include "chat_message.hpp"

using asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
    chat_client(asio::io_context& io_context,
        const tcp::resolver::results_type& endpoints)
        : io_context_(io_context),
        socket_(io_context)
    {
        do_connect(endpoints);
    }

    void write(const chat_message& msg)
    {
        asio::post(io_context_,
            [this, msg]()
            {
                bool write_in_progress = !write_msgs_.empty();
                write_msgs_.push_back(msg);
                if (!write_in_progress)
                {
                    do_write();
                }
            });
    }

    void close()
    {
        asio::post(io_context_, [this]() { socket_.close(); });
    }

private:
    void do_connect(const tcp::resolver::results_type& endpoints)
    {
        asio::async_connect(socket_, endpoints,
            [this](std::error_code ec, tcp::endpoint)
            {
                if (!ec)
                {
					std::cout << "Connected to server\n";
                    std::cout << "Enter the name of the sender: ";
                    std::cin >> name;
					std::cout << "Enter the id of the sender: ";
                    std::cin >> sender_id;
					std::cout << "Enter the id of the reciever: ";
                    std::cin >> reciever_id;
                    chat_message msg;
                    char line[] = "";
                    msg.body_length(std::strlen(line));
                    std::memcpy(msg.body(), line, msg.body_length());
                    msg.encode_header(sender_id, reciever_id, 3);
                    this->write(msg);

                    do_read_header();
                }
            });

    }


    void do_read_header()
    {
        asio::async_read(socket_,
            asio::buffer(read_msg_.data(), chat_message::header_length),
            [this](std::error_code ec, std::size_t /*length*/)
            {
                if (!ec && read_msg_.decode_header())
                {
                    do_read_body();
                }
                else
                {
                    socket_.close();
                }
            });
    }

    void do_read_body()
    {
        asio::async_read(socket_,
            asio::buffer(read_msg_.body(), read_msg_.body_length()),
            [this](std::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    std::cout.write(read_msg_.body(), read_msg_.body_length());
                    std::cout << "\n";
                    do_read_header();
                }
                else
                {
                    socket_.close();
                }
            });
    }

    void do_write()
    {
        asio::async_write(socket_,
            asio::buffer(write_msgs_.front().data(),
                write_msgs_.front().length()),
            [this](std::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    write_msgs_.pop_front();
                    if (!write_msgs_.empty())
                    {
                        do_write();
                    }
                }
                else
                {
                    socket_.close();
                }
            });
    }

private:
    asio::io_context& io_context_;
    tcp::socket socket_;
    chat_message read_msg_;
    chat_message_queue write_msgs_;
	std::string name;
    double sender_id;
	double reciever_id;
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: chat_client <host> <port>\n";
            return 1;
        }

        asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(argv[1], argv[2]);
        chat_client c(io_context, endpoints);

        std::thread t([&io_context]() { io_context.run(); });

		// this loop allows user to input a message and send it to the server
        char line[chat_message::max_body_length + 1];
        while (std::cin.getline(line, chat_message::max_body_length + 1))
        {
            chat_message msg;
            msg.body_length(std::strlen(line));
            std::memcpy(msg.body(), line, msg.body_length());
            msg.encode_header();
            c.write(msg);
        }

        c.close();
        t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}