#include "AudioServer.hpp"
#include "Request.hpp"
#include "nlohmann/json.hpp"

using nlohmann::json;

int main()
{
    AudioServer audioServer;

    TcpServer::asyncServer("192.168.1.109", PORT, [&audioServer](std::string request, std::string& response) {
        std::cout << "Request: " << request << " of size: " << request.size() << std::endl;
        json jsRequest = request;


        if (jsRequest["type"] == "CREATEROOM") {

            int uid = jsRequest["uid"].get<int>();

            audioServer.createRoom(uid);

            return;
        }
        else {
            response = "lalalalal";

            return;
        }

    });
}