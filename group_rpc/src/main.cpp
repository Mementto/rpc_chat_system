#include "../include/GroupService.hpp"
#include "../rpc/include/MpRPCApplication.hpp"
#include "../rpc/include/RPCProvider.hpp"

int main(int argc, char** argv) {
    MpRPCApplication::init(argc, argv);
    RPCProvider provider;
    provider.NotifyService(new GroupsService);
    provider.run();
    return 0;
}