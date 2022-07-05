#include "../include/FriendService.hpp"
#include "../rpc/include/MpRPCApplication.hpp"
#include "../rpc/include/RPCProvider.hpp"

int main(int argc, char** argv) {
    MpRPCApplication::init(argc, argv);
    RPCProvider provider;
    provider.NotifyService(new FriendsService);
    provider.run();
    return 0;
}