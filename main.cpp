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

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>

//PUSH NOTIFICATIONS HNNNNNNGGGGGGGHHHHHHHHHWAPOGMWAQPOIGFMNOP)IQ@MNE)POI@M!#)!#@!JM$_O!@K$_@!K_#$K!@<+_)$K!@

#pragma pack()
struct StunRequest {

    StunRequest() {
        srand(time(NULL));
        for (int i = 0; i < tid.size(); i++) {
            tid[i] = rand() % 256;
        }
    }

    template<typename archive> void serialize(archive& ar, const unsigned) {
        ar& messageType;
        ar& messageLen;
        ar& magicCookie;
        ar& tid;
    }

    const int16_t messageType = htons(0x0001);
    const int16_t messageLen = htons(0x0000);
    const int32_t magicCookie = htonl(0x2112A442);
    std::array<uint8_t, 12> tid;
};

int main() {
    /*WebSocketServer server;
    server.run();*/
    _net::io_context _ioc;
    
    udp::socket sock(_ioc);
    udp::endpoint ep;
    try {
        udp::resolver resolver(_ioc);
        ep = *resolver.resolve("192.168.1.110", "3478");
    }
    catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, floor(pow(2, 96)) - 1);
    auto tid = dist(rng);

    StunRequest sr;

    try {
        sock.send_to(_net::buffer(&sr, sizeof(StunRequest)), ep);
    }
    catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
    char buffer[512]{ 0 };

    int rcv = sock.receive(_net::buffer(buffer));
    std::cout << rcv;
    exit(0);
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
}