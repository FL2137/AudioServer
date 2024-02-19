#include "AudioServer.hpp"


int AudioServer::createRoom(int uid) {
	std::vector<User>::iterator iter;

	if ((iter = std::find(loggedUsers.begin(), loggedUsers.end(), uid)) != loggedUsers.end()) {
		User host = *iter;
		std::cout << "uid:" << host.uid << std::endl;
		return lastRid++;
	}
	else {
		lastError = "This user is not logged in";
		return 0;
	}
}

bool AudioServer::userConnected(std::string userData, tcp::endpoint *lastEndpoint) {
	json data = json::parse(userData);

	try {
		std::string nickname = data["nickname"];
		std::string password = data["password"];
		std::cout << "nick: " << nickname << " pass: " << password << std::endl;
		User user(nickname, lastUid++);
		user.tcpEndpoint = tcp::endpoint(lastEndpoint->address(), lastEndpoint->port());
		loggedUsers.push_back(user);
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

std::vector<std::string> AudioServer::roomCheck(int roomId, int uid) {
	std::vector<std::string> roomUsers = {};
	std::vector<Room>::iterator iter;
	iter = std::find(existingRooms.begin(), existingRooms.end(), uid);
	if (iter != existingRooms.end()) {
		for (User u : iter->users) {
			roomUsers.push_back(u.nickname);
		}
		return roomUsers;
	}
	else {
		lastError = "This room doesn't exist";
		return {};
	}
}

void AudioServer::notifyRoom(int roomId) {
	std::vector<Room>::iterator iter = std::find(existingRooms.begin(), existingRooms.end(), roomId);
	if (iter != existingRooms.end()) {
		
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