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
    bool joinRoom(int roomId, int uid);

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

    inline void kick(int uid) {
        users.erase(std::remove(users.begin(), users.end(), uid), users.end());
    }

    inline void addUser(const User& user) {
        users.push_back(user);
    }

    bool operator==(int rid) {
        if (rid == this->id)
            return true;
        else
            return false;
    }


    std::vector<User> users;
};

