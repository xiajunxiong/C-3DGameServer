#pragma once
#include <windows.h>
#include <string>

class RedisProcess
{
public:
    static bool Start();
    static void Stop();

private:
    static PROCESS_INFORMATION g_pi;
};