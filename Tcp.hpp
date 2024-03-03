#pragma once
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

using nlohmann::json;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

const int PORT = 3005;

class TcpConnection : public boost::enable_shared_from_this<TcpConnection> {

public:
    typedef boost::shared_ptr<TcpConnection> pointer;

    static pointer create(boost::asio::io_context& ioc, void (TcpServer::*foo)(std::string, std::string&, tcp::endpoint *ep)) {
        return pointer(new TcpConnection(ioc, foo));
    }

    tcp::socket& socket() {
        return _socket;
    }

    void start() {
        boost::asio::async_read(
            _socket,
            boost::asio::buffer(readBuffer),
            boost::asio::transfer_at_least(1),
            boost::bind(&TcpConnection::handleRead, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
        );
    }


private:

    TcpConnection(boost::asio::io_context& ioc, void (TcpServer::* requestParser)(std::string, std::string&, tcp::endpoint *ep)) : _socket(ioc) {
        this->requestParser = requestParser;
    }

    void (*requestParser)(std::string, std::string&, tcp::endpoint*);

    //std::function<void(std::string, std::string&, tcp::endpoint *ep)> requestParser;

    void handleRead(const boost::system::error_code& error, size_t bytesTransferred) {
        if (error) {
            std::cerr << "Error receiving message from client: " << error.message() << std::endl;
        }
        else {

            request = std::string(readBuffer);

            if (request.find("LOGIN") != std::string::npos) {
                lastEp = _socket.remote_endpoint();
            }

            requestParser(request, response, &lastEp);

            boost::asio::async_write(
                _socket,
                boost::asio::buffer(response, response.size()),
                boost::bind(&TcpConnection::handleWrite, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
            );

            std::cout << "server sent " << response << std::endl;
        }
    }

    void handleWrite(const boost::system::error_code& error, size_t bytesTransferred) {
        if (!error) {
            //std::cout << "Sent via socket: " << bytesTransferred << std::endl;
        }
        else {
            std::cerr << "Error writing response" << std::endl;
            _socket.close(); //is this necesarry
        }
    }


    static const int BUFFER_SIZE = 512;
    char readBuffer[BUFFER_SIZE];
    std::string response = "notParsed";
    std::string request = "";

    tcp::socket _socket;
    tcp::endpoint lastEp;
};

class TcpServer {
public:

    TcpServer(boost::asio::io_context& _io_context, std::string address, int port)
        : io_context(_io_context), acceptor(io_context, tcp::endpoint(boost::asio::ip::make_address_v4(address), port))
    {
        startAccept();
    }

    static void asyncServer(std::string address, int port, void (*f)(std::string, std::string&, tcp::endpoint*)) {
        boost::asio::io_context ioc;
        TcpServer server(ioc, address, port);
        ioc.run();
    }

    void notify(std::string notification, int uid) {
        for (TcpConnection::pointer con : connections) {
            con->socket().write_some(boost::asio::buffer(notification));
        }
    }


    //priv functions
private:

    void startAccept() {
        connections.push_back(TcpConnection::create(io_context, foo));

        acceptor.async_accept(connections[connections.size() - 1]->socket(),
            boost::bind(&TcpServer::handleAccept, this, connections[connections.size() - 1], boost::asio::placeholders::error));
    }

    void handleAccept(TcpConnection::pointer newConnection, const boost::system::error_code& error) {
        if (!error) {
            newConnection->start();
        }

        startAccept();
    }


    //priv variables
private:

    std::vector<TcpConnection::pointer> connections;

    TcpConnection::pointer newConnection;

    boost::asio::io_context& io_context;
    tcp::acceptor acceptor;
    std::function<void(std::string, std::string&, tcp::endpoint* ep)> requestParser;

    void foo(std::string request, std::string& response, tcp::endpoint *ep) {

    }


};


