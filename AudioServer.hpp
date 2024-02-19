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
        lastUid = 0;
    }

    bool userConnected(std::string userData, tcp::endpoint *lastEndpoint);
    int createRoom(int uid);
    bool joinRoom(int roomId, int uid);
    bool quitRoom(int roomId, int uid);
    std::vector<std::string> roomCheck(int roomId, int uid);

    std::string lastError;

    //bad scalability here
    int lastUid;
    int lastRid;

    void notifyRoom(int roomId);

private:

    std::vector<User> loggedUsers = {};
    std::vector<Room> existingRooms = {};
};





