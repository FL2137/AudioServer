#pragma once
#include "Tcp.hpp"
#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <algorithm>
#include "nlohmann/json.hpp"


using nlohmann::json;

class User;
class Room;

class AudioServer {
public:
    AudioServer() {
        lastUid = 0;
    }

    bool userConnected(json userData);
    bool createRoom(int uid);
    bool joinRoom();

    std::string lastError;

    int lastUid;

private:

    std::vector<User> loggedUsers = {};
    std::vector<Room> existingRooms = {};
};

class User {
public:

    User(std::string nickname, int uid) {
        this->nickname = nickname;
        this->uid = uid;
    }

    tcp::endpoint tcpEndpoint;
    udp::endpoint udpEndpoint;

    std::string nickname;

    int uid;

    bool operator==(int _uid) {
        if (_uid == this->uid)
            return true;
        else
            return false;
    }
};

class Room {
public:

    Room(User& host) {
        users.push_back(host);
    }

    int id;
    int size;

    void kick(int uid);
    void addUser(int uid);

    std::vector<User> users;
};

