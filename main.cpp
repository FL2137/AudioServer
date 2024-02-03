#include "AudioServer.hpp"
#include "Request.hpp"

int main()
{
    AudioServer audioServer;

    TcpServer::asyncServer("192.168.1.109", PORT, [&audioServer](std::string request, std::string& response) {
        std::cout << "Request: " << request << " of size: " << request.size() << std::endl;
        json jsRequest = request;


        if (jsRequest["type"] == "CREATEROOM") {
            if (audioServer.createRoom(jsRequest["uid"].get<int>())) {
                response = "OK";
            }
            else {
                response = audioServer.lastError;
            }
            return;
        }
        else {
            return;
        }

    });
}