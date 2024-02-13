#pragma once
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class User {

public:

    User(std::string nickname, int uid) {
        this->nickname = nickname;
        this->uid = uid;
    }

    tcp::endpoint tcpEndpoint;
    udp::endpoint udpEndpoint;

    std::string nickname;

    int uid;

    bool operator==(int _uid) {
        if (_uid == this->uid)
            return true;
        else
            return false;
    }
};