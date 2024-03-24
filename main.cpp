#include "Request.hpp"
#include "nlohmann/json.hpp"
#include "AudioServer.hpp"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <stdint.h>
#include "Database.hpp"
#include "WebSocketServer.hpp"
#include <random>
#include <math.h>
#include <array>
#include "StunClient.hpp"

using boost::asio::ip::udp;

int main() {

    StunClient stc;

    boost::asio::io_context ioc;
    udp::resolver resolver(ioc);
    udp::endpoint ep = *resolver.resolve("stun2.l.google.com", "3478").begin();
    udp::endpoint localEndpoint(boost::asio::ip::make_address("192.168.1.109"), 3007);

    udp::socket socket(ioc, localEndpoint);

    std::cout << std::get<0>(stc.getExternalAddress(&socket, &ep));

    getchar();

    exit(0);
    /*
    const auto address = boost::asio::ip::make_address("192.168.1.109");
    int port = 3005;
    int threads = 1;

    boost::asio::io_context ioc{ threads };

    AudioServer *audioServer = new AudioServer();

    std::shared_ptr<BeastWebSocket> beast = std::make_shared<BeastWebSocket>(ioc, tcp::endpoint(address, port), audioServer);
    beast->run();

    std::vector<std::thread> threadV;
    threadV.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i) {
        threadV.emplace_back([&ioc] {ioc.run(); });
    }

    ioc.run();


    std::cout << "Closing server...";
    delete audioServer;

    return 0;
    */
}