#include "AudioServer.hpp"

bool AudioServer::setUserEndpoint(int uid, std::string address, std::string port) {
	std::vector<User>::iterator iter;
	if ((iter = std::find(loggedUsers.begin(), loggedUsers.end(), uid)) != loggedUsers.end()) {
		User user = *iter;

		(*iter).udpEndpoint = udp::endpoint(boost::asio::ip::make_address(address), std::stoi(port));

		//std::cout << "User " << (*iter).uid << " " << (*iter).udpEndpoint.address() << ":" << (*iter).udpEndpoint.port() << std::endl;
		return true;
	}

	lastError = "User not found";
	return false;
}

int AudioServer::createRoom(int uid) {

	std::vector<User>::iterator iter;

	if ((iter = std::find(loggedUsers.begin(), loggedUsers.end(), uid)) != loggedUsers.end()) {
		User host = *iter;
		std::cout << host.nickname << std::endl;

		Room room(host);

		room.id = lastRid++;
		room.addUser(*iter);

		activeRooms.push_back(room);

		return room.id;
	}
	else {
		lastError = "This user is not logged in";
		return 0;
	}

}

bool AudioServer::userConnected(std::string userData) {

	json data;

	try {
		data = json::parse(userData);
	}
	catch (json::parse_error& err) {
		std::cerr << "parse error: " << err.what() << std::endl;
	}


	try {
		std::string nickname = data["nickname"].get<std::string>();
		std::string password = data["password"].get<std::string>();
		User user(nickname, lastUid++);
		loggedUsers.push_back(user);
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return true;
}

bool AudioServer::joinRoom(int roomId, int uid, json &roomUsers) {
	std::vector<User>::iterator iter;
	if ((iter = std::find(loggedUsers.begin(), loggedUsers.end(), uid)) != loggedUsers.end()) {
		User user = *iter;
		std::vector<Room>::iterator roomiter;
		if ((roomiter = std::find(activeRooms.begin(), activeRooms.end(), roomId)) != activeRooms.end()) {
			roomUsers = roomiter->toJson();
			roomiter->addUser(user);
			return true;
		}
		else {
			lastError = "This room doesn't exist";
			return false;
		}
	}
	else {
		lastError = "This user is not logged in";
		return false;
	}
}

bool AudioServer::quitRoom(int roomId, int uid) {
	std::vector<User>::iterator iter;
	if ((iter = std::find(loggedUsers.begin(), loggedUsers.end(), uid)) != loggedUsers.end()) {
		User user = *iter;
		std::vector<Room>::iterator roomiter;
		if ((roomiter = std::find(activeRooms.begin(), activeRooms.end(), roomId)) != activeRooms.end()) {
			roomiter->kick(uid);
			return true;
		}
		else {
			lastError = "This room doesn't exist";
			return false;
		}
		return true;
	}
	else {
		lastError = "This user is not logged in";
		return false;
	}
}

bool AudioServer::roomCheck(int roomId, int uid, json &room) {
	std::vector<std::string> roomUsers = {};
	std::vector<Room>::iterator iter;
	iter = std::find(activeRooms.begin(), activeRooms.end(), uid);
	if (iter != activeRooms.end()) {
		room = iter->toJson();
		return true;
	}
	else {
		lastError = "This room doesn't exist";
		return false;
	}
}

std::vector<std::string> AudioServer::friendListCheck(int uid) {
	std::vector<std::string> nicknames = {};
	for (const User& user : loggedUsers) {
		if (user.uid != uid)
			nicknames.push_back(user.nickname);
	}
	return nicknames;
}

void AudioServer::notifyRoom(int roomId) {
	std::vector<Room>::iterator iter = std::find(activeRooms.begin(), activeRooms.end(), roomId);
	if (iter != activeRooms.end()) {
		
		boost::asio::io_context ioc;

		tcp::socket socket(ioc);

		ioc.run();

		json data = iter->toJson();

		for (const User& user : iter->users) {
			socket.connect(user.tcpEndpoint);
			socket.write_some(boost::asio::buffer(data.dump()));
			socket.close();
		}
	}
}

void AudioServer::notifyFriends(int uid, const std::vector<User> &_loggedUsers, std::string notifMessage) {

	boost::asio::io_context ioc;
	tcp::socket socket(ioc);
	ioc.run();

	json js;
	js["users"] = json::array();

	json::array_t users;

	for (const User& user : _loggedUsers)
		users.push_back(user.nickname);
	
	js["users"] = users;
	js["type"] == "NOTIFYFRIENDS";

	for (const User& user : _loggedUsers) {
		if (user.uid != uid) {
			try {
				std::cout << "Notifying " << user.tcpEndpoint.address().to_string() << ":" << user.tcpEndpoint.port() << std::endl;
				tcp::endpoint ep(user.udpEndpoint.address(), 3009);
 				socket.connect(ep);
				socket.write_some(boost::asio::buffer(js.dump()));
				socket.close();
			}
			catch (const std::exception& e) {
				std::cout << "THREW: " << e.what() << std::endl;
			}
		}
	}
}

bool AudioServer::setAvatar(int uid, std::string data) {

	std::vector<User>::iterator iter = std::find(loggedUsers.begin(), loggedUsers.end(), uid);
	if (iter != loggedUsers.end()) {
		
		json js = json::parse(data);

		const char *avatar = js["avatar"].get<std::string>().c_str();
		iter->avatarPicture = (char*)avatar;
		return true;
	}
	else {
		lastError = "No user with this id";
		return false;
	}
}

std::string AudioServer::base64_decode(const std::string& in) {

	std::string out;

	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

	int val = 0, valb = -8;
	for (char c : in) {
		if (T[c] == -1) break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
	return out;
}

std::string AudioServer::base64_encode(const std::string & in) {

	std::string out;

	int val = 0, valb = -6;
	for (char c : in) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4) out.push_back('=');
	return out;
}