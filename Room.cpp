#include "Room.hpp"


json Room::toJson() {
	json data;

	for (const User& user : users) {
		json::array_t arr;
		arr.push_back(user.nickname);
		data["users"] = arr;
	}
	return data;
}