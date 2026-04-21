#pragma once
#include <cstdint>

// 登录相关 (10000 - 10999)
enum class MsgID : uint32_t
{
    // 客户端 -> 服务端
    C2S_LoginRequest = 10001,    // 登录请求（带 Token）
    C2S_RefreshToken = 10002,    // Refresh Token 请求
    C2S_RegisterRequest = 10003,    // 注册请求（账号+密码+昵称等）
    C2S_LogoutRequest = 10004,      // 退出登录请求
    C2S_LoginWithToken = 10005,    // Token 自动登录（免账号密码）


    // 服务端 -> 客户端
    S2C_LoginSuccess = 10051,    // 登录成功
    S2C_LoginFailed = 10052,    // 登录失败
    S2C_RegisterSuccess = 10053,    // 注册成功
    S2C_RegisterFailed = 10054,     // 注册失败（通用错误）
    S2C_AccountAlreadyExist = 10055,// 注册失败：账号已被注册
    S2C_InvalidParam = 10056,       // 注册/登录失败：参数格式错误

    S2C_LogoutSuccess = 10057,      // 退出登录成功
    S2C_LoginWithTokenSuccess = 10058, // Token 登录成功
    S2C_LoginWithTokenFailed = 10059, // Token 登录失败（过期/无效）
    S2C_AccountBanned = 10060, // 账号被封禁（你要的）
};


struct MsgHeader
{
    uint32_t length;
    MsgID    msgID;
};

// 1. 登录请求 C2S_LoginRequest
struct C2S_Login
{
    char account[32];
    char password[32];
};

// 2. 注册请求 C2S_RegisterRequest
struct C2S_Register
{
    char account[32];
    char password[32];
    char nickname[32];
};

// 3. 通用返回结果
struct S2C_Result
{
    int32_t code;
    char msg[64];    // 提示信息
};