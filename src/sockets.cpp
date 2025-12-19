//
// Created on 12/11/25.
//

#include "sockets.h"

namespace connections
{
    bool Socket::canConnect(const Socket* socket) const
    {
        if (connectableType() != socket->connectableType()) return false;
        return  ((type == SocketType::in || type == SocketType::io) &&
                    (socket->type == SocketType::out || socket->type == SocketType::io)) ||
                ((type == SocketType::out || type == SocketType::io) &&
                    (socket->type == SocketType::in || socket->type == SocketType::io));
    }

    void Socket::updateConnection(Socket* socket)
    {
        if (single)
        {
            if (std::holds_alternative<Socket*>(connected)) disconnect(*std::get<Socket*>(connected));
            connected = socket;
        } else
        {
            if (std::holds_alternative<std::monostate>(connected)) connected = std::vector<Socket*>();
            std::get<std::vector<Socket*>>(connected).push_back(socket);
        }
    }

    void Socket::connect(Socket* socket)
    {
        if (canConnect(socket))
        {
            updateConnection(socket);
            socket->updateConnection(this);
        }
    }

    void Socket::disconnect(const Socket& socket)
    {
        if (single)
        {
            connected = std::monostate{};
        } else
        {
            std::erase(std::get<std::vector<Socket*>>(connected), &socket);
        }
    }

    void Socket::addCallback(char event, const std::string& name, std::function<void(Socket*, void*)> callback)
    {
        if (!callbacks.contains(event))
            callbacks.emplace(event, std::map<std::string, std::function<void (Socket*, void*)>>{});
        callbacks.at(event).emplace(name, std::move(callback));
    }

    void Socket::removeCallback(char event, const std::string& name)
    {
        callbacks.at(event).erase(name);
    }
}
