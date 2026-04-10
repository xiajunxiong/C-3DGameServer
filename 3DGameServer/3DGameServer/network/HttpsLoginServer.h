#pragma once
#define CPPHTTPLIB_NO_OS_CHECK
#define CPPHTTPLIB_OPENSSL_SUPPORT
#define _WIN32_WINNT 0x0A00
#pragma warning(disable:4996)
#include "httplib.h"
#include <string>

class HttpsLoginServer
{
public:
    void Start();
    void Stop();
private:
    httplib::SSLServer* svr = nullptr;
};