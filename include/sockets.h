//
// Created on 12/11/25.
//

#ifndef CONNECTIONS_SOCKETS_H
#define CONNECTIONS_SOCKETS_H
#include <functional>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

namespace connections
{
    enum class SocketType
    {
        in,
        out,
        io
    };

    class Socket
    {
    protected:
        SocketType type = SocketType::io;
        std::variant<std::monostate, Socket*, std::vector<Socket*>> connected;
        std::map<char, std::map<std::string, std::function<void (Socket*, void*)>>> callbacks;
        bool single = false;

        void updateConnection(Socket* socket);

        [[nodiscard]] virtual const std::type_info* connectableType() const {return &typeid(Socket);}

    public:
        Socket(const SocketType type, const bool single) : type{type}, single{single} {}
        virtual ~Socket() = default;

        void setSingle(const bool value) {single = value;}

        void addCallback(char event, const std::string& name, std::function<void(Socket*, void*)> callback);
        void removeCallback(char event, const std::string& name);

        [[nodiscard]] bool canConnect(const Socket* socket) const;
        void connect(Socket* socket);
        void disconnect(const Socket& socket);

        [[nodiscard]] SocketType getType() const {return type;}

        enum class EventType : char
        {
            connected = 0,
            disconnected = 1
        };
    };

    template <typename T>
    struct SocketValue
    {
        bool single = true;
        bool empty = false;
        std::variant<T, std::vector<T>> data;
        SocketValue(std::vector<T> data) : single{false}, data{data} {}
        SocketValue(T data) : data{data} {}
        SocketValue(): empty{true} {}

        T getSingle() {
            if (empty) throw std::runtime_error{"Socket is empty"};
            if (!single) throw std::logic_error{"Attempted to read single value from multiple socket"};
            return std::get<T>(data);
        }
        std::vector<T> getMultiple() {
            if (empty) throw std::runtime_error{"Socket is empty"};
            if (single) throw std::logic_error{"Attempted to read multiple values from single socket"};
            return std::get<std::vector<T>>(data);
        }
    };

    template <typename T>
    class DataSocket : public Socket
    {
        SocketValue<T> cachedValue;

    protected:
        [[nodiscard]] const std::type_info* connectableType() const override {return &typeid(DataSocket);}

    public:
        explicit DataSocket(const SocketType socketType, const bool single):
            Socket{socketType, single}, cachedValue{} {}

        void update(T value)
        {
            if (type == SocketType::out || type == SocketType::io)
            {
                cachedValue = value;
                std::visit([&]<typename U>(U &&conn)
                {
                    if constexpr (std::is_same_v<U, std::vector<Socket*>>)
                    {
                        for (auto socket : conn)
                        {
                            if (socket->getType() == SocketType::in)
                                static_cast<DataSocket*>(socket)->update(value);
                        }
                    }
                    else if constexpr (std::is_same_v<U, Socket*>)
                    {
                        if (conn->getType() == SocketType::in)
                            static_cast<DataSocket*>(conn)->update(value);
                    }
                }, connected);
            }
            if (!callbacks.contains(static_cast<char>(EventType::updated))) return;
            for (auto callback :
                callbacks.at(static_cast<char>(EventType::updated)) | std::views::values)
            {
                callback.operator()(this, &value);
            }
        }

        SocketValue<T> getValue()
        {
            if (type == SocketType::out)
            {
                return cachedValue;
            }
            if (!cachedValue.empty) return cachedValue;
            if (single)
            {
                if (std::get<Socket*>(connected)->getType() == SocketType::out)
                {
                    return SocketValue{std::get<int>(
                        static_cast<DataSocket*>(std::get<Socket*>(connected))->getValue().data)};
                }
                return SocketValue<T>{};
            }
            std::vector<T> values{};
            for (auto socket : std::get<std::vector<Socket*>>(connected))
            {
                if (socket->getType() == SocketType::out)
                {
                    values.push_back(std::get<T>(static_cast<DataSocket*>(socket)->getValue().data));
                } else
                {
                    return SocketValue<T>{};
                }
            }
            return SocketValue{values};
        }

        enum class EventType : char
        {
            updated = 2
        };
    };
}

#endif //CONNECTIONS_SOCKETS_H