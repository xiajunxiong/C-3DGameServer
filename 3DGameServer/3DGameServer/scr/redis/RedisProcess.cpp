#include "redis/RedisProcess.h"
#include <iostream>

PROCESS_INFORMATION RedisProcess::g_pi = { 0 };

bool RedisProcess::Start()
{
    // 1. 启动 redis-server.exe
    std::wstring redisPath = L"D:\\插件_工具\\redis\\Redis-x64-3.0.504\\redis-server.exe";

    STARTUPINFO si = { sizeof(si) };
    ZeroMemory(&g_pi, sizeof(g_pi));

    si.wShowWindow = SW_HIDE;
    si.dwFlags |= STARTF_USESHOWWINDOW;

    si.hStdOutput = CreateFileA("nul", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    si.hStdError = si.hStdOutput;
    si.dwFlags |= STARTF_USESTDHANDLES;

    BOOL ok = CreateProcess(
        NULL,
        (LPWSTR)redisPath.c_str(),
        NULL, NULL,
        TRUE,
        0, NULL, NULL,
        &si, &g_pi
    );

    CloseHandle(si.hStdOutput);

    if (!ok) {
        std::cerr << "启动Redis进程失败" << std::endl;
        return false;
    }

    // 等待redis启动
    Sleep(500);

    // 2. 连接Redis
    return ConnectRedis();
}

bool RedisProcess::ConnectRedis()
{
    if (m_ctx) redisFree(m_ctx);

    m_ctx = redisConnect("127.0.0.1", 6379);
    if (!m_ctx || m_ctx->err) {
        std::cerr << "Redis连接失败: " << (m_ctx ? m_ctx->errstr : "null") << std::endl;
        return false;
    }

    std::cout << "Redis连接成功！" << std::endl;
    return true;
}

void RedisProcess::Stop()
{
    // 关闭连接
    if (m_ctx) {
        redisFree(m_ctx);
        m_ctx = nullptr;
    }

    // 关闭进程
    if (g_pi.hProcess) {
        TerminateProcess(g_pi.hProcess, 0);
        CloseHandle(g_pi.hProcess);
        CloseHandle(g_pi.hThread);
        g_pi.hProcess = NULL;
    }
}

bool RedisProcess::Set(const std::string& key, const std::string& value)
{
    if (!m_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(m_ctx, "SET %s %s", key.c_str(), value.c_str());
    bool ok = reply && reply->type == REDIS_REPLY_STATUS;
    freeReplyObject(reply);
    return ok;
}

bool RedisProcess::SetEx(const std::string& key, int seconds, const std::string& value)
{
    if (!m_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(m_ctx, "SETEX %s %d %s", key.c_str(), seconds, value.c_str());
    bool ok = reply && reply->type == REDIS_REPLY_STATUS;
    freeReplyObject(reply);
    return ok;
}

std::string RedisProcess::Get(const std::string& key)
{
    if (!m_ctx) return "";
    redisReply* reply = (redisReply*)redisCommand(m_ctx, "GET %s", key.c_str());
    std::string val;
    if (reply && reply->type == REDIS_REPLY_STRING) {
        val = reply->str;
    }
    freeReplyObject(reply);
    return val;
}

bool RedisProcess::Del(const std::string& key)
{
    if (!m_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(m_ctx, "DEL %s", key.c_str());
    bool ok = reply && reply->type == REDIS_REPLY_INTEGER;
    freeReplyObject(reply);
    return ok;
}