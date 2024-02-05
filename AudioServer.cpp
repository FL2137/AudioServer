#include "AudioServer.hpp"


bool AudioServer::createRoom(int uid) {
	std::vector<User>::iterator iter;

	if ((iter = std::find(loggedUsers.begin(), loggedUsers.end(), uid)) != loggedUsers.end()) {
		User host = *iter;
		std::cout << "uid:" << host.uid << std::endl;

		return true;
	}
	else {
		lastError = "This user is not logged in";
		return false;
	}
}

bool AudioServer::userConnected(json userData) {

	try {
		std::string nickname = userData["nickname"].get<std::string>();
		std::string password = userData["password"].get<std::string>();
		std::cout << "nick: " << nickname << " pass: " << password << std::endl;
		loggedUsers.push_back(User(nickname, lastUid++));
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return true;
}
