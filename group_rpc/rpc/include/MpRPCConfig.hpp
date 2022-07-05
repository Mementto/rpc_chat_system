#pragma once

#include <unordered_map>
#include <string>

/**
 * @brief 读取配置文件类
 * 
 */
class MpRPCConfig {

public:
    /**
     * @brief 解析配置文件
     * 
     * @param configFile 
     */
    void loadConfigFile(const char* configFile);

    /**
     * @brief 查询配置项信息
     * 
     * @param key 
     * @return std::string 
     */
    std::string load(const std::string& key);

private:
    std::unordered_map<std::string, std::string> m_configMap;

private:
    void remoteSpace(std::string& str);

};