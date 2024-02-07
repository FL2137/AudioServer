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

bool AudioServer::joinRoom(int roomId, int uid) {
	std::vector<User>::iterator iter;
	if ((iter = std::find(loggedUsers.begin(), loggedUsers.end(), uid)) != loggedUsers.end()) {
		User user = *iter;
		std::vector<Room>::iterator roomiter;
		if ((roomiter = std::find(existingRooms.begin(), existingRooms.end(), roomId)) != existingRooms.end()) {
			roomiter->addUser(user);
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

bool AudioServer::quitRoom(int roomId, int uid) {
	std::vector<User>::iterator iter;
	if ((iter = std::find(loggedUsers.begin(), loggedUsers.end(), uid)) != loggedUsers.end()) {
		User user = *iter;
		std::vector<Room>::iterator roomiter;
		if ((roomiter = std::find(existingRooms.begin(), existingRooms.end(), roomId)) != existingRooms.end()) {
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