#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>
#include <array>
#include <stdint.h>
#include <string>
#include <random>

class StunClient {

private:
#define BINDING_RESPONSE 0x0101



public:

	StunClient() {
	}

	std::string getExternalAddress() {
		using boost::asio::ip::udp;
		
		boost::asio::io_context ioc;
		udp::resolver resolver(ioc);
		udp::endpoint stunServerEndpoint = *resolver.resolve(udp::v4(), "stun2.l.google.com", "3478").begin();
		udp::socket socket(ioc);

		socket.open(udp::v4());

		try {
			StunRequest request;

			socket.send_to(boost::asio::buffer(&request, sizeof(request)), stunServerEndpoint);
			std::array<unsigned char, 64> cresponse;
			int receivedBytes = socket.receive_from(boost::asio::buffer(cresponse), stunServerEndpoint);

			StunResponse* stunResp = reinterpret_cast<StunResponse*>(&cresponse);

			//check response validity

			if (stunResp->messageType != BINDING_RESPONSE) {
				throw std::exception("Stun response type indicates invalid request");
			}

			if (stunResp->transactionID == request.transactionID) {
				throw std::exception("Transaction ids dont match");
			}

			if (stunResp->magicCookie == request.magicCookie) {
				throw std::exception("Magic cookies dont match...?");
			}


			std::string address = "";




		}
		catch(const std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	}

private:

//baisc STUN message formats
#pragma pack(push, 1)
	struct StunRequest {
		StunRequest() {

			//generate random transaction ID
			
			std::random_device device;
			std::mt19937 generator(device);
			std::uniform_int_distribution<unsigned char> distribution(0, 255);
			
			for (int i = 0; i < 12; i++) {
				transactionID[i] = distribution(generator);
			}
		}

		const int16_t messageType = htons(0x0001);
		const int16_t messageLen = htons(0x0000);
		const int32_t magicCookie = htonl(0x2112A442);
		std::array<unsigned char, 12> transactionID = {0};
	};
#pragma pack(pop)


#pragma pack(push, 1)
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
		std::array<unsigned char, 12> transactionID;
		
		StunAttribute att1;
		StunAttribute att2;
	};
#pragma pack(pop)
};