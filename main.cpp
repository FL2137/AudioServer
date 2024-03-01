
#include "Request.hpp"
#include "nlohmann/json.hpp"
#include "AudioServer.hpp"

#include "Database.hpp"


int main() {

    {
        Database db;
    }


    AudioServer audioServer;

    boost::asio::io_context ioc;

    std::vector<User> users;
    tcp::endpoint ep(boost::asio::ip::make_address_v4("192.168.1.109"), 3009);
    tcp::socket socket(ioc);
    

    std::thread notifyThread;

    TcpServer tcpServer(ioc, "192.168.1.109", PORT, [&](std::string request, std::string& response, tcp::endpoint *ep) {
        //std::cout << "Request: " << request << " of size: " << request.size() << std::endl;
        json jsRequest = json::parse(request.c_str());
       
        std::cout << ">" << jsRequest["type"] << std::endl;
        

        if (jsRequest["type"] == "CREATEROOM") {

            int uid = jsRequest["uid"].get<int>();

            int rid;
            if ((rid = audioServer.createRoom(uid))) {
                json js;
                js["ok"] = "OK";
                js["rid"] = rid;
                response = js.dump();
            }
            else {
                response = audioServer.lastError;
            }
            return;
        }
        else if (jsRequest["type"] == "JOINROOM") {

            int uid = jsRequest["uid"].get<int>();
            int roomId = jsRequest["rid"].get<int>();

            if (audioServer.joinRoom(roomId, uid)) {
                json js;
                js["ok"] = "OK";
                response = js.dump();
            }
            else {
                json js;
                js["ok"] = audioServer.lastError;
                response = js.dump();
            }
            return;
        }
        else if (jsRequest["type"] == "LOGIN") {
           
            std::cout << ep->address().to_string() << std::endl;
            if (audioServer.userConnected(jsRequest["data"].get<std::string>(), ep)) {
                json js;
                js["ok"] = "OK";
                js["uid"] = audioServer.lastUid - 1;
                response = js.dump();
                std::thread s(&AudioServer::notifyFriends, &audioServer, audioServer.lastUid - 1, audioServer.loggedUsers);
                s.detach();
            }
            else {
                json js;
                js["ok"] = audioServer.lastError;
                response = js.dump();
            }

            return;
        }
        else if (jsRequest["type"] == "SETAVATAR") {
            int uid = jsRequest["uid"].get<int>();
            
            if (audioServer.setAvatar(uid, jsRequest["data"].get<std::string>())) {
                json js;
                js["ok"] = "OK";
                response = js.dump();
            }
            else {
                json js;
                js["ok"] = audioServer.lastError;
                response = js.dump();
            }
            return;
        }
        else if (jsRequest["type"] == "QUITROOM") {
            int uid = jsRequest["uid"].get<int>();
            int roomId = jsRequest["rid"].get<int>();

            if (audioServer.quitRoom(roomId, uid)) {
                response = "OK";
            }
            else {
                response = audioServer.lastError;
            }
            return;
        }
        else if (jsRequest["type"] == "ROOMCHECK") {
            int uid = jsRequest["uid"].get<int>();
            int rid = jsRequest["rid"].get<int>();
        }
        else if (jsRequest["type"] == "FRIENDSCHECK") {
            int uid = jsRequest["uid"].get<int>();
            
            std::vector<std::string> friends = audioServer.friendListCheck(uid);
            json js;
            js["data"] = json::array();
            for (const std::string& _friend : friends) {
                js["data"].push_back(_friend);
            }
            response = js.dump();
        }

    });
    
    ioc.run();
}