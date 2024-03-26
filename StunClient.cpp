#include "StunClient.hpp"

//this is bad because it returns the address for a socket that isnt binded to it anymore :REDO
std::tuple<std::string, std::string> StunClient::getExternalAddress(udp::socket *socket, udp::endpoint *ep) {

    StunRequest sr;

    try {
        boost::system::error_code err;
        socket->send_to(boost::asio::buffer(&sr, sizeof(sr)), *ep, NULL, err);
        std::cout << err.message() << std::endl;
    }
    catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
    std::array<char, 128> buffer;

    udp::endpoint sep(boost::asio::ip::make_address("192.168.1.109"), 50687);

    boost::system::error_code er;

    udp::endpoint remoteEp;

    int rcv = socket->receive_from(boost::asio::buffer(buffer), remoteEp, NULL, er);
    std::cout << er.message() << std::endl;

    socket->send_to(boost::asio::buffer(buffer), sep);


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

    port = std::to_string((ntohl(srsp->attribXORMAPPEDADDRESS.port) & 0xffFF) ^ (srsp->magicCookie >> 16));

    return std::make_tuple(address, port);
}