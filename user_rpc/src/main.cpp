#include "../include/ChatServer.hpp"
#include "../include/ChatService.hpp"
#include "../rpc/include/MpRPCApplication.hpp"

#include <signal.h>
#include <time.h>
#include <iostream>

void resetHanler(int) {
    ChatService::instance()->resetUserState();
    exit(0);
}

int main(int argc, char **argv) {
    
    EventLoop loop;
    InetAddress addr(argv[1], atoi(argv[2]));
    ChatServer server(&loop, addr, "ChatServer");
    MpRPCApplication::init(argc - 2, argv + 2);

    signal(SIGINT, resetHanler);
    
    server.start();
    loop.loop();

    return 0;
}