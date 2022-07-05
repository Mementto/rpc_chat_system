#pragma once

#include "MpRPCController.hpp"
#include "MpRPCConfig.hpp"
#include "Logger.hpp"

class MpRPCApplication {

public:
    static void init(int argc, char** argv);
    static MpRPCApplication& getInstance();
    static MpRPCConfig& getConfig();

private:
    MpRPCApplication() {}
    MpRPCApplication(const MpRPCApplication&) = delete;
    MpRPCApplication(MpRPCApplication&&) = delete;
    MpRPCApplication& operator=(const MpRPCApplication&) = delete;
    MpRPCApplication& operator=(MpRPCApplication&&) = delete;

private:
    static MpRPCConfig m_config;

};