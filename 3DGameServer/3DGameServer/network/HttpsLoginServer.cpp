#include "HttpsLoginServer.h"
#include <iostream>
#include <fstream>


// 检查文件是否存在
bool fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

void HttpsLoginServer::Start()
{
    // 1. 检查证书文件
    if (!fileExists("server.crt") || !fileExists("server.key")) {
        std::cout << "证书丢失：server.crt / server.key\n";
        return;
    }

    // 2. 创建 SSL 服务
    svr = new httplib::SSLServer("server.crt", "server.key");

    // 3. 判断是否创建成功
    if (!svr || !svr->is_valid()) {
        std::cout << "SSL 服务启动失败（证书无效）\n";
        delete svr;
        svr = nullptr;
        return;
    }

    // 4. 注册路由（全部加 httplib:: 命名空间，无错）
    svr->Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("HTTPS Server Start", "text/plain; charset=utf-8");
        });

    svr->Post("/api/login", [this](const httplib::Request& req, httplib::Response& res) {
        const char* data = req.body.data();
        size_t dataLen = req.body.size();

        auto responseJson = router.HandleResponse(std::vector<uint8_t>(req.body.begin(), req.body.end()));
        res.set_content(responseJson, "application/json");
        });

    std::cout << "HTTPS 服务启动：https://127.0.0.1:8443\n";

    // 监听
    svr->listen("0.0.0.0", 8443);
}

void HttpsLoginServer::Stop()
{
    if (svr) {
        svr->stop();
        delete svr;
        svr = nullptr;
    }
}

std::string HttpsLoginServer::HandleLogin(const std::vector<uint8_t>& data)
{
    std::cout << "[Login] 收到登录请求，正在处理...\n";

    // TODO: 写你的登录逻辑（解析数据、验证 Token、查数据库等）

    return R"({"code":200,"msg":"登录成功","token":"jwt_token_12345"})";
}

std::string HttpsLoginServer::HandleRegistration(const std::vector<uint8_t>& data)
{
    std::cout << "[Registration] 收到注册请求\n";
    return R"({"code":200,"msg":"注册成功"})";
}

