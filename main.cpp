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

#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))

#pragma pack(push, 1)
struct StunRequest {

    StunRequest() {
        srand(time(NULL));
        for (int i = 0; i < tid.size(); i++) {
            tid[i] = rand() % 256;
        }
    }

    const int16_t messageType = htons(0x0001);
    const int16_t messageLen = htons(0x0000);
    const int32_t magicCookie = htonl(0x2112A442);
    std::array<uint8_t, 12> tid;
};


struct StunAttribute {
    int16_t type;
    int16_t len;
    int32_t value;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct StunResponse {
    int16_t messageType;
    int16_t messageLen;
    int32_t magicCookie;
    std::array<uint8_t, 12> tid;

    std::array<StunAttribute,1000> attribs;
};
#pragma pack(pop)

int main() {
    /*WebSocketServer server;
    server.run();*/
    _net::io_context _ioc;
    udp::resolver resolver(_ioc);
    //udp::endpoint ep = *resolver.resolve("stun2.l.google.com", "19305");
    //udp::endpoint ep(boost::asio::ip::make_address("192.168.1.109"), 65469);
    udp::endpoint ep = *resolver.resolve(udp::v4(), "stun2.l.google.com", "3478").begin();
    udp::socket sock(_ioc);
    sock.open(udp::v4());
    
    StunRequest sr;

    uint16_t data[] = { 0x0001, 0x0000, 0x2112, 0xA442,
        0x79,
        0xc3,
        0x0a,
        0x1e,
        0x65,
        0x63,
        0x7e,
        0x07,
        0x8d,
        0x4e,
        0xc5,
        0x41,
        0x20,
        0x4f,
        0xfb,
        0x34,
        0x03,
        0xd6,
        0x4a,
        0x60,
        0x0d,
        0x09,
        0x73,
        0x05
    };

    try {
        boost::system::error_code err;
        sock.send_to(_net::buffer(&sr, sizeof(sr)), ep, NULL, err);
        std::cout << err.message() << std::endl;
    }
    catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << "here???\n";
    std::array<char, 128> buffer;

    udp::endpoint sep;

    boost::system::error_code er;
    int rcv = sock.receive_from(_net::buffer(buffer), ep, NULL, er);
    std::cout << er.message() << std::endl;

    udp::endpoint packetSenderEndpoint(boost::asio::ip::make_address("192.168.1.109"), 65469);

    sock.send_to(_net::buffer(buffer), packetSenderEndpoint);

    StunResponse* srsp = reinterpret_cast<StunResponse*>(&buffer);

    /*std::cout << std::hex << srsp->magicCookie << std::endl;
    std::cout << std::hex << srsp->messageType << std::endl;
    std::cout << std::hex << srsp->messageLen << std::endl;*/


    if (srsp->magicCookie != sr.magicCookie) {
        std::cerr << "magicCookies do not match...\n";
    }
    else {
        std::cerr << "magic cookies good\n";
    }
    std::cout << "Transaction ID comparsion: <req.tid> - <res.tid>\n";
    for (int i = 0; i < 12; i++) {
        std::cout << std::hex << (int)sr.tid[i] << " - " << std::hex << (int)srsp->tid[i] << std::endl;
    }

    std::cout << "Attribs: \n";
    std::cout << "0x0001 type:" << std::hex << srsp->attribs[0].type << std::endl;
    std::cout << "0x0001 len:" << std::hex << srsp->attribs[0].len << std::endl;
    std::cout << "0x0001 value:" << std::hex << srsp->attribs[0].value << std::endl;


    _ioc.run();

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