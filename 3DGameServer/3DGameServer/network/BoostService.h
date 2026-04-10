#pragma once
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

class BoostService : public std::enable_shared_from_this<BoostService>
{
public:
    using Ptr = std::shared_ptr<BoostService>;

    static Ptr Create(boost::asio::io_context& io)
    {
        return std::make_shared<BoostService>(io);
    }

    tcp::socket& Socket() { return socket_; }

    // 开始异步读取
    void Start()
    {
        DoRead();
    }

    explicit BoostService(boost::asio::io_context& io)
        : socket_(io)
    {
    }

private:
    void DoRead()
    {
        auto self = shared_from_this();
        socket_.async_read_some(boost::asio::buffer(buffer_),
            [this, self](const boost::system::error_code& ec, std::size_t bytes)
            {
                if (!ec)
                {
                    std::string msg(buffer_.data(), bytes);
                    std::cout << "收到消息: " << msg << std::endl;

                    // 回显给客户端
                    DoWrite(msg);
                }
            });
    }

    void DoWrite(const std::string& msg)
    {
        auto self = shared_from_this();
        boost::asio::async_write(socket_, boost::asio::buffer(msg),
            [this, self](const boost::system::error_code& ec, std::size_t)
            {
                if (!ec)
                {
                    DoRead(); // 继续接收下一条消息
                }
            });
    }

    tcp::socket socket_;
    std::array<char, 1024> buffer_;
};

class TcpServer
{
public:
    TcpServer(boost::asio::io_context& io, short port)
        : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port))
    {
        StartAccept();
    }

private:
    void StartAccept()
    {
        auto new_conn = BoostService::Create(io_);
        acceptor_.async_accept(new_conn->Socket(),
            [this, new_conn](const boost::system::error_code& ec)
            {
                if (!ec)
                {
                    std::cout << "客户端已连接" << std::endl;
                    new_conn->Start();
                }
                StartAccept(); // 继续监听下一个连接
            });
    }

    boost::asio::io_context& io_;
    tcp::acceptor acceptor_;
};