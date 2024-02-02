#include "AudioServer.hpp"

int main()
{
    AudioServer audioServer;


    TcpServer::asyncServer("192.168.1.109", PORT, [](std::string request, std::string& response) {
        std::cout << "Request: " << request << " of size: " << request.size() << std::endl;

        if (request == "CREATEROOM") {

            audioServer.



            return;
        }
        else {
            response = "dzialaez";
            return;
        }

        response = "ehh";
        });
}