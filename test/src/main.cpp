//
// Created on 12/11/25.
//

#include <iostream>

#include <sockets.h>

int main()
{
    connections::DataSocket<int> source(connections::SocketType::out, false);
    connections::DataSocket<int> destination1(connections::SocketType::in, true);
    connections::DataSocket<int> destination2(connections::SocketType::in, true);

    source.connect(&destination1);
    source.connect(&destination2);

    destination1.addCallback(
        static_cast<char>(connections::DataSocket<int>::EventType::updated), "updated",
        [&](connections::Socket* socket, void* value) {
            std::cout << "Callback for value" << *static_cast<int*>(value) << std::endl;
        });

    source.update(8);

    std::cout << destination1.getValue().getSingle() << std::endl;
    //std::cout << destination2.getValue().getSingle() << std::endl;
}
