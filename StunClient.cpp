#include "StunClient.hpp"

//this is bad because it returns the address for a socket that isnt binded to it anymore :REDO
std::string StunClient::getExternalAddress(udp::socket *socket, udp::endpoint *ep) {

    udp::endpoint ep = *resolver.resolve(udp::v4(), "stun2.l.google.com", "3478").begin();
    udp::socket sock(_ioc);


    StunRequest sr;

    try {
        boost::system::error_code err;
        sock.send_to(boost::asio::buffer(&sr, sizeof(sr)), ep, NULL, err);
        std::cout << err.message() << std::endl;
    }
    catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
    std::array<char, 128> buffer;

    udp::endpoint sep(boost::asio::ip::make_address("192.168.1.109"), 50687);

    boost::system::error_code er;
    int rcv = sock.receive_from(boost::asio::buffer(buffer), ep, NULL, er);
    std::cout << er.message() << std::endl;

    sock.send_to(boost::asio::buffer(buffer), sep);


    StunResponse* srsp = reinterpret_cast<StunResponse*>(&buffer);

    if (srsp->magicCookie != sr.magicCookie) {
        std::cerr << "magicCookies do not match...\n";
    }

    for (int i = 0; i < 12; i++) {
        std::cout << std::hex << (int)sr.transactionID[i] << " - " << std::hex << (int)srsp->transactionID[i] << std::endl;
    }

    std::string address = "";
    std::string port = "";

    srsp->magicCookie = ntohl(srsp->magicCookie);

    //parse XOR-MAPPED-ADDRESS
    for (int i = 0; i < 4; i++) {
        address += std::to_string(((srsp->magicCookie >> (24 - (i * 8))) & 0xff) ^ ((ntohl(srsp->attribXORMAPPEDADDRESS.address) >> 24 - (i * 8)) & 0xff)) + ".";
    }
    address.pop_back();//get rid of the last dot

    std::cout << std::to_string(ntohl(srsp->attribXORMAPPEDADDRESS.port) & 0xff) << std::endl;
    std::cout << std::to_string((ntohl(srsp->attribXORMAPPEDADDRESS.port) >> 8) & 0xff) << std::endl;




    return address + ":" + port;
}