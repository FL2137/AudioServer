#pragma once
#include "Tcp.hpp"
#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <algorithm>
#include "nlohmann/json.hpp"

#include "User.hpp"

#include "Room.hpp"


using nlohmann::json;

class User;
class Room;

class AudioServer {
public:
    AudioServer() {
        lastUid = 1;
        lastRid = 1;
    }

    bool userConnected(std::string userData, tcp::endpoint *lastEndpoint);
    int createRoom(int uid);
    bool joinRoom(int roomId, int uid);
    bool quitRoom(int roomId, int uid);
    bool setAvatar(int uid, std::string data);
    std::vector<std::string> roomCheck(int roomId, int uid);

    std::string lastError;

    //bad scalability here
    int lastUid;
    int lastRid;

    void notifyRoom(int roomId);
    void notifyFriends(int uid);

    static std::string base64_encode(const std::string& in);

    static std::string base64_decode(const std::string& in);



private:

    std::vector<User> loggedUsers = {};
    std::vector<Room> existingRooms = {};
};





