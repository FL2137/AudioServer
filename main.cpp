#include "AudioServer.hpp"
#include "Request.hpp"
#include "nlohmann/json.hpp"

using nlohmann::json;

int main() {

    AudioServer audioServer;

    boost::asio::io_context ioc;

    TcpServer tcpServer(ioc, "192.168.1.109", PORT, [&](std::string request, std::string& response, tcp::endpoint *ep) {
        std::cout << "Request: " << request << " of size: " << request.size() << std::endl;
        json jsRequest = json::parse(request.c_str());
       
        std::cout << jsRequest["type"] << std::endl;
        

        if (jsRequest["type"] == "CREATEROOM") {

            int uid = jsRequest["uid"].get<int>();

            if (audioServer.createRoom(uid)) {
                response = "OK";
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
                response = "OK";
            }
            else {
                response = audioServer.lastError;
            }
            return;
        }
        else if (jsRequest["type"] == "LOGIN") {
            std::cout << jsRequest["data"] << std::endl;
            if (audioServer.userConnected(jsRequest["data"].get<std::string>())) {
                json js;
                js["ok"] = "OK";
                js["uid"] = audioServer.lastUid - 1;
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
    });
    ioc.run();
}