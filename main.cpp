#include "AudioServer.hpp"
#include "Request.hpp"
#include "nlohmann/json.hpp"

using nlohmann::json;

int main()
{
    AudioServer audioServer;

    TcpServer::asyncServer("192.168.1.109", PORT, [&audioServer](std::string request, std::string& response) {
        std::cout << "Request: " << request << " of size: " << request.size() << std::endl;
        json jsRequest = json::parse(request);

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
            else if(jsRequest["type"] == "LOGIN") {
                std::cout << jsRequest["data"] << std::endl;
                if (audioServer.userConnected(jsRequest["data"].get<json>())) {
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
       
    });
}