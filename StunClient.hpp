#include <iostream>
#include <boost/asio.hpp>
#include <array>
#include <stdint.h>
#include <string>
#include <random>

class StunClient {


private:

	StunClient() {

	}



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
		
		std::array<StunAttribute, 100> attributes;
	};
#pragma pack(pop)
};