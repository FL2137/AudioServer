#pragma once
#include "User.hpp"
#include <nlohmann/json.hpp>

using nlohmann::json;

class Room {
public:

    Room(User& host) {
        users.push_back(host);
    }

    int id;
    int size = 4;

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

    json toJson();

    std::vector<User> users;
};