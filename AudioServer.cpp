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
	std::string nickname = userData["nickname"].get<std::string>();
	std::string password = userData["passhash"].get<std::string>();
	std::cout << "nick: " << nickname << " pass: " << password << std::endl;

	loggedUsers.push_back(User(nickname));
	return true;
}
