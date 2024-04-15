#pragma once
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

using nlohmann::json;

class User {

public:

    User(std::string nickname, int uid) {
        this->nickname = nickname;
        this->uid = uid;
    }

    tcp::endpoint tcpEndpoint;
    udp::endpoint udpEndpoint;

    std::string nickname;

    //should this be stored????
    char* avatarPicture;

    int uid;

    bool operator==(int _uid) {
        if (_uid == this->uid)
            return true;
        else
            return false;
    }

    json toJson() const {
        json body;
        body["nickname"] = nickname;
        body["address"] = udpEndpoint.address().to_string();
        body["port"] = udpEndpoint.port();
        return body;
    }

};