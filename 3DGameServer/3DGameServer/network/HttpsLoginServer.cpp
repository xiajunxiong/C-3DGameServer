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

    svr->Post("/api/login", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(R"({"code":200,"msg":"登录成功"})", "application/json");
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