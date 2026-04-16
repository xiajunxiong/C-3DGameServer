#include "MessageRouter.h"
#include <iostream>
#include <HttpsLoginServer.h>
#include <nlohmann/json.hpp>
static MessageRouter g_GlobalRouter;

void MessageRouter::Start()
{
    std::cout << "=== 开始注册所有消息处理函数 ===\n";


    // 登录
    g_GlobalRouter.RegisterResponse(MsgID::C2S_LoginRequest,
        &HttpsLoginServer::HandleLogin);
    // 注册
    g_GlobalRouter.RegisterResponse(MsgID::C2S_RegisterRequest,
        &HttpsLoginServer::HandleRegistration);

    std::cout << "=== 所有消息注册完成 ===\n";


}

void MessageRouter::RegisterResponse(MsgID msgID, ResponseHandler handler)
{
    if (handler) {
        m_responseHandlers[msgID] = std::move(handler);
        std::cout << "[MessageRouter] 注册 Response 消息: " << (uint32_t)msgID << std::endl;
    }
}

void MessageRouter::RegisterPush(MsgID msgID, PushHandler handler)
{
    if (handler) {
        m_pushHandlers[msgID] = std::move(handler);
        std::cout << "[MessageRouter] 注册 Push 消息: " << (uint32_t)msgID << std::endl;
    }
}

std::string MessageRouter::HandleResponse(const std::vector<uint8_t>& data)
{
    if (data.empty()) {
        return R"({"code":400,"msg":"数据为空"})";
    }

    try
    {
        std::string jsonStr(data.begin(), data.end());
        std::cout << "[Router] 收到 JSON: " << jsonStr << std::endl;
        auto j = nlohmann::json::parse(jsonStr);
        uint32_t msgIdValue = j.value("msgID", 0u);
        MsgID msgID = static_cast<MsgID>(msgIdValue);

        auto it = m_responseHandlers.find(msgID);
        if (it != m_responseHandlers.end()) {
            return it->second(data);
        }
        else {
            std::cout << "[Router] 未注册的消息ID: " << msgIdValue << std::endl;
            return R"({"code":404,"msg":"未注册的消息类型"})";
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "[Router] JSON 解析失败: " << e.what() << std::endl;
        return R"({"code":400,"msg":"JSON格式错误"})";
    }
}

void MessageRouter::HandlePush(uint64_t sessionId, const std::vector<uint8_t>& data)
{
    if (data.size() < sizeof(MsgHeader)) {
        std::cout << "[MessageRouter] Push 消息太短，无效，sessionId: " << sessionId << std::endl;
        return;
    }

    MsgHeader header{};
    memcpy(&header, data.data(), sizeof(MsgHeader));

    auto it = m_pushHandlers.find(header.msgID);
    if (it != m_pushHandlers.end()) {
        // 调用对应的 Push 处理函数，并传入 sessionId
        it->second(sessionId, data);
    }
    else {
        std::cout << "[MessageRouter] 未注册的 Push 消息ID: " << (uint32_t)header.msgID
            << "，sessionId: " << sessionId << std::endl;
    }
}