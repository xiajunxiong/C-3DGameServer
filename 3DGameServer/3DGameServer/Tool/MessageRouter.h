#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include "Protocol.h"


class MessageRouter {
public:
    using ResponseHandler = std::function<std::string(const std::vector<uint8_t>& data)>;     // 用于 HTTPS（需要返回结果）
    using PushHandler = std::function<void(uint64_t sessionId, const std::vector<uint8_t>& data)>; // 用于 TCP（需要 sessionId）

    static void Start();

    // 注册需要返回响应的消息（HTTPS登录用）
    void RegisterResponse(MsgID msgID, ResponseHandler handler);

    // 注册只需要推送的消息（TCP游戏逻辑用）
    void RegisterPush(MsgID msgID, PushHandler handler);

    // HTTPS 调用（需要返回结果）
    std::string HandleResponse(const std::vector<uint8_t>& data);

    // TCP 调用（需要知道是哪个客户端）
    void HandlePush(uint64_t sessionId, const std::vector<uint8_t>& data);

private:
    std::unordered_map<MsgID, ResponseHandler> m_responseHandlers;
    std::unordered_map<MsgID, PushHandler>     m_pushHandlers;

};