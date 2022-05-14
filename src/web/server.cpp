#include "main.hpp"
#include "ModConfig.hpp"

#include "web/server.hpp"
#include "utils/WebUtils.hpp"

#include "socket_lib/shared/ServerSocket.hpp"
#include "socket_lib/shared/SocketHandler.hpp"
#include "socket_lib/shared/SocketLogger.hpp"

#include "TMPro/TextMeshProUGUI.hpp"
#include "TMPro/TextAlignmentOptions.hpp"

using namespace SocketLib;

namespace WEB{
    ServerSocket* socket;

    void connectEvent(Channel& channel, bool connected){
        getLogger().info("Socket Connected");
    }

    void listenOnEvents(Channel& client, const Message &message, TMPro::TextMeshProUGUI* text){
        auto msgStr = std::string(message.toString());
        auto id = msgStr.substr(5, 36);

        getLogger().info("User Adopt Request From User: %s", id.c_str());

        client.queueWrite(Message("HTTP/1.1 200 OK\nAccept-Ranges: bytes\nContent-Length: 8\nAccess-Control-Allow-Origin: *\nContent-Type: text/html\n\nHelo 123"));
        socket->closeClient(client.clientDescriptor);
        text->set_text("User Adopt Request From UserID: "+id);

        getLogger().info("Fetching JSON");
        auto json = WebUtils::GetJSON("http://192.168.11.13/api/v1/devices/adopt?token="+getModConfig().Token.GetValue()+"&uid="+id);

        if(json->IsObject()){
            auto jsonData = json->GetObject();

            getLogger().info("Error: %s", jsonData["error"].GetBool() ? "true" : "false");

            auto userData = jsonData["u"].GetObject();

            text->set_text("Hello "+std::string(userData["name"].GetString()));
        }
    }

    void Init(TMPro::TextMeshProUGUI* text){
        text->set_text("Starting HTTP Server...");

        socket = SocketHandler::getCommonSocketHandler().createServerSocket(6452);
        socket->bindAndListen();
        getLogger().info("HTTP Server Listening");
        text->set_text("Waiting For Adoption Request...");

        socket->connectCallback += [text](Channel& client, bool connected){
            text->set_text("Socket Connected, Waiting For Request...");
            connectEvent(client, connected);
        };

        socket->listenCallback += [text](Channel& client, const Message& message){
            listenOnEvents(client, message, text);
        };
    }

    void Close(){
        socket->notifyStop();
        SocketHandler::getCommonSocketHandler().destroySocket(socket);
    }
}