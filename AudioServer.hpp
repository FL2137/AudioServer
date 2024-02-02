#pragma once
#include "Tcp.hpp"
#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

using nlohmann::json;

class User {
    tcp::endpoint tcpEndpoint;
    udp::endpoint udpEndpoint;
};

class Room {
public:

    Room(User& host) {

    }

    int id;
    int size;

    void kick(int uid);
    void addUser(int uid);

    std::vector<User> users;
};

class AudioServer {
public:

    void userConnected(json userData);
    void createRoom(json userData);
    void joinRoom(json userData);

private:

    std::vector<User> loggedUsers = {};
    std::vector<Room> existingRooms = {};
};