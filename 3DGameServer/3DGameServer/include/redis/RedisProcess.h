#pragma once
#include <windows.h>
#include <string>
#include <hiredis.h>

class RedisProcess
{
public:
    static RedisProcess& Instance() {
        static RedisProcess ins;
        return ins;
    }

    bool Start();      // 启动redis-server.exe + 连接redis
    void Stop();       // 关闭连接 + 关闭进程

    // 操作Redis
    bool Set(const std::string& key, const std::string& value);
    bool SetEx(const std::string& key, int seconds, const std::string& value);
    std::string Get(const std::string& key);
    bool Del(const std::string& key);

private:
    bool ConnectRedis(); // 内部连接

private:
    static PROCESS_INFORMATION g_pi;
    redisContext* m_ctx = nullptr;
};