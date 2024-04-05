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

    const auto address = boost::asio::ip::make_address("192.168.0.109");
    int port = 3005;
    int threads = 4;

    boost::asio::io_context ioc{ threads };

    AudioServer *audioServer = new AudioServer();

    //std::shared_ptr<BeastWebSocket> beast = std::make_shared<BeastWebSocket>(ioc, tcp::endpoint(address, port), audioServer);
    //beast->run();
    std::make_shared<BeastWebSocket>(ioc, tcp::endpoint(address, port), audioServer)->run();

    std::vector<std::thread> threadV;
    threadV.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i) {
        threadV.emplace_back([&ioc] {ioc.run(); });
    }

    ioc.run();


    delete audioServer;
    return 0;
}