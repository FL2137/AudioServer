#include <iostream>
#include <string>
#include "nlohmann/json.hpp"

using nlohmann::json;

struct request_t {
	std::string type;
	int uid;
};

class Request {

public:
	Request(std::string reqestStr) {
		requestJson = parseToJson(reqestStr);
	}

	static json parseToJson(std::string str) {
		if (json::accept(str)) {
			return json::parse(str);
		}
		else {
			return json("");
		}
	}

private:

	std::string jsonStr;
	json requestJson;
};