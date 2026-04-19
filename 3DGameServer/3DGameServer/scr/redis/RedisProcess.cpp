#include "redis/RedisProcess.h"

PROCESS_INFORMATION RedisProcess::g_pi = { 0 };

bool RedisProcess::Start()
{
    // Redis 路径
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
    return ok;
}

void RedisProcess::Stop()
{
    if (g_pi.hProcess)
    {
        TerminateProcess(g_pi.hProcess, 0);
        CloseHandle(g_pi.hProcess);
        CloseHandle(g_pi.hThread);
        g_pi.hProcess = NULL;
    }
}