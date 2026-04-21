#include "network/HttpsLoginServer.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <network/Protocol.h>
#include <data/pqxx/PqxxData.h>
#include <bcrypt/bcrypt.h>
#include <jwt-cpp/jwt.h>
#include <redis/RedisProcess.h>

extern PqxxData pqxxdata;

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
    try
    {

        std::string jsonStr(data.begin(), data.end());
        std::cout << "[Login] 收到请求: " << jsonStr << std::endl;

        auto j = nlohmann::json::parse(jsonStr);

        std::string account = j.value("account", "");
        std::string password = j.value("password", "");

        if (account.empty() || password.empty())
        {
            return R"({"code":400,"msg":"账号或密码不能为空"})";
        }
        std::string respStr;

        User user = pqxxdata.Login(account, password);

        if (user.status != 0) {
            std::cout << "[登录] 账号被封禁\n";
            nlohmann::json response = {
                {"code", 500},
                {"msgID", static_cast<uint32_t>(MsgID::S2C_AccountBanned)},
                {"msg", "status: " + std::to_string(user.status)}
            };
            respStr = response.dump(4);
            return respStr;
        }

        if (bcrypt::validatePassword(password, user.password)) {
            pqxxdata.UpdateLastLoginTime(user.account);
            std::string token = generateLoginToken(std::to_string(user.uid), user.account);
            if (token != "") {
                std::string oldToken = RedisProcess::Instance().Get("whitelist:uid_" + std::to_string(user.uid));
                if (!oldToken.empty()) {
                    // 旧Token加入黑名单，有效期 24 小时
                    RedisProcess::Instance().SetEx("blacklist:" + oldToken, 86400, "invalid");
                }
                RedisProcess::Instance().SetEx(
                    "whitelist:uid_" + std::to_string(user.uid),  // key
                    86400,                                        // 有效期 24小时
                    token                                         // value：新token
                );

                nlohmann::json response = {
                    {"code", 200},
                    {"msgID", static_cast<uint32_t>(MsgID::S2C_LoginSuccess)},
                    {"gameServer", "127.0.0.1"},
                    {"port", 12345},
                    {"token", token}
                };
                respStr = response.dump(4);
            }
        }
        else
        {
            std::cout << "[登录] 密码错误\n";
            nlohmann::json response = {
                {"code", 500},
                {"msgID", static_cast<uint32_t>(MsgID::S2C_LoginFailed)},
                {"msg", "account,password NO!"},
            };
            respStr = response.dump(4);


        }

        std::cout << "[Login] 账号: " << account << std::endl;

        return respStr;
    }
    catch (const nlohmann::json::exception& e)
    {
        std::cout << "[Login] JSON 解析错误: " << e.what() << std::endl;
        return R"({"code":400,"msg":"JSON格式错误"})";
    }
    catch (const std::exception& e)
    {
        std::cout << "[Login] 处理异常: " << e.what() << std::endl;
        return R"({"code":500,"msg":"服务器内部错误"})";
    }
}

std::string HttpsLoginServer::HandleRegistration(const std::vector<uint8_t>& data)
{
    try
    {
    std::string jsonStr(data.begin(), data.end());
    std::cout << "[Login] 收到请求: " << jsonStr << std::endl;

    auto j = nlohmann::json::parse(jsonStr);

    std::string username = j.value("username", "");
    std::string account = j.value("account", "");
    std::string password = j.value("password", "");

    if (username.empty() || account.empty() || password.empty())
    {
        return R"({"code":400,"msg":"null"})";
    }
    std::string respStr;

    int flag = pqxxdata.AddUser(username, account, password);
    if (flag != 0) {
        nlohmann::json response = {
        {"code", 500},
        {"msgID", static_cast<uint32_t>(MsgID::S2C_RegisterFailed)},
        {"msg", "return: " + std::to_string(flag)},
        };
        respStr = response.dump(4);
        return respStr;
    }
    nlohmann::json response = {
    {"code", 200},
    {"msgID", static_cast<uint32_t>(MsgID::S2C_RegisterSuccess)},
    {"msg", "return: " + std::to_string(flag)},
        };
    respStr = response.dump(4);
    return respStr;
    }
    catch (const nlohmann::json::exception& e)
    {
        std::cout << "[Registration] JSON 解析错误: " << e.what() << std::endl;
        return R"({"code":400,"msg":"JSON格式错误"})";
    }
    catch (const std::exception& e)
    {
        std::cout << "[Registration] 异常: " << e.what() << std::endl;
        return R"({"code":500,"msg":"服务器内部错误"})";
    }
}

std::string HttpsLoginServer::HandleTokenLogin(const std::vector<uint8_t>& data)
{
    try
    {
        std::string jsonStr(data.begin(), data.end());
        std::cout << "[TokenLogin] 收到请求: " << jsonStr << std::endl;

        auto j = nlohmann::json::parse(jsonStr);
        std::string token = j.value("token", "");

        if (token.empty())
        {
            return R"({"code":400,"msg":"token不能为空"})";
        }

        std::string respStr;
        long long uid = verifyToken(token);

        if (uid != -1)
        {
            // 检查是否在黑名单
            std::string blackCheck = RedisProcess::Instance().Get("blacklist:" + token);
            if (!blackCheck.empty())
            {
                // 在黑名单 → 失效
                nlohmann::json response = {
                    {"code", 500},
                    {"msgID", static_cast<uint32_t>(MsgID::S2C_LoginWithTokenFailed)},
                    {"msg","token -1"}
                };
                respStr = response.dump(4);
            }
            else
            {
                // 检查白名单是否匹配
                std::string realToken = RedisProcess::Instance().Get("whitelist:uid_" + std::to_string(uid));

                if (realToken.empty() || realToken != token)
                {
                    // 不匹配 → 旧token
                    nlohmann::json response = {
                        {"code", 500},
                        {"msgID", static_cast<uint32_t>(MsgID::S2C_LoginWithTokenFailed)},
                        {"msg","token-1"}
                    };
                    respStr = response.dump(4);
                }
                else
                {
                    std::cout << "[TokenLogin] 登录成功 UID: " << uid << std::endl;
                    
                    pqxxdata.UpdateLoginTime(std::to_string(uid));

                    nlohmann::json response = {
                        {"code", 200},
                        {"msgID", static_cast<uint32_t>(MsgID::S2C_LoginSuccess)},
                        {"gameServer", "127.0.0.1"},
                        {"port", 12345},
                        {"uid", uid},
                        {"token", token}
                    };
                    respStr = response.dump(4);
                }
            }
        }
        else
        {
            nlohmann::json response = {
                {"code", 500},
                {"msgID", static_cast<uint32_t>(MsgID::S2C_LoginWithTokenFailed)},
                {"msg","jwt-1"}
            };
            respStr = response.dump(4);
        }

        return respStr;
    }
    catch (const nlohmann::json::exception& e)
    {
        std::cout << "[TokenLogin] JSON 解析错误: " << e.what() << std::endl;
        return R"({"code":400,"msg":"JSON格式错误"})";
    }
    catch (const std::exception& e)
    {
        std::cout << "[TokenLogin] 异常: " << e.what() << std::endl;
        return R"({"code":500,"msg":"服务器内部错误"})";
    }
}



std::string HttpsLoginServer::generateLoginToken(const std::string& uid, const std::string& account)
{
    std::string token = jwt::create()
        .set_issuer("game_server")
        .set_issued_at(std::chrono::system_clock::now())
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{ 3600 })
        .set_payload_claim("uid", jwt::claim(uid))
        .set_payload_claim("account", jwt::claim(account))
        .sign(jwt::algorithm::hs256{ "431982283@qq.com" });

    return token;
}

long long HttpsLoginServer::verifyToken(const std::string& token)
{
    try
    {
        auto decoded = jwt::decode(token);

        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ "431982283@qq.com" })
            .with_issuer("game_server");

        verifier.verify(decoded);

        std::string uid_str = decoded.get_payload_claim("uid").as_string();
        return std::stoll(uid_str);
    }
    catch (const std::exception& e)
    {
        std::cout << "[Token 校验失败] " << e.what() << std::endl;
        return -1; // 失败返回 -1
    }
}