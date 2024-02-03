#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <functional>
#include <string>
#include "nlohmann/json.hpp"

using nlohmann::json;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;


class User {
    tcp::endpoint tcpEndpoint;
    udp::endpoint udpEndpoint;
};

class Room {
public:

    Room(User &host) {

    }

};
