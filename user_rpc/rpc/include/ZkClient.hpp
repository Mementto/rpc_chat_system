#pragma once

#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient {

public:
    ZkClient();
    ~ZkClient();

    // 启动连接Server
    void start();
    // 创建节点
    void create(const char* path, const char* data, int dataLen, int state = 0);

    // 获取结点
    std::string getData(const char* path);

private:
    zhandle_t* m_zHandle;

};