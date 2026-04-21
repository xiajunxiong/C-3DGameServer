#pragma once
#define CPPHTTPLIB_NO_OS_CHECK
#define CPPHTTPLIB_OPENSSL_SUPPORT
#define _WIN32_WINNT 0x0A00
#pragma warning(disable:4996)
#include "httplib.h"
#include <string>
#include "MessageRouter.h"

class HttpsLoginServer
{
public:
    void Start();
    void Stop();
    static std::string HandleLogin(const std::vector<uint8_t>& data);
    static std::string HandleRegistration(const std::vector<uint8_t>& data);
    static std::string HandleTokenLogin(const std::vector<uint8_t>& data);
private:
    httplib::SSLServer* svr = nullptr;
    MessageRouter router;
    static std::string  generateLoginToken(const std::string& uid, const std::string& account);
    static long long verifyToken(const std::string& token);
};