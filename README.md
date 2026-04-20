基于Boost、PostgreSQL、Redis 实现的3D游戏服务端
c++ 20
PostgreSQL 18.3_x64
boos 1_88_0
redis 3.0

# 登录注册模块 (Login & Register System)

## 流程流程图
```mermaid
flowchart TD
    Start[收到HTTPS请求]
    
    Start -->|注册接口| Registration[HandleRegistration]
    Registration --> ParseReg[解析JSON: username, account, password]
    ParseReg --> CheckRegEmpty{参数为空?}
    CheckRegEmpty -->|是| RetReg400[{"code":400, msg:"null"}]
    CheckRegEmpty -->|否| DBAdd[调用 pqxxdata.AddUser()]
    DBAdd --> RegSuccess{返回值 == 0?}
    RegSuccess -->|是| RetReg200[{"code":200, 注册成功}]
    RegSuccess -->|否| RetReg500[{"code":500, 注册失败}]
    
    Start -->|登录接口| Login[HandleLogin]
    Login --> ParseLogin[解析JSON: account, password]
    ParseLogin --> CheckLoginEmpty{参数为空?}
    CheckLoginEmpty -->|是| RetLogin400[{"code":400, msg:"不能为空"}]
    CheckLoginEmpty -->|否| DBLogin[调用 pqxxdata.Login()]
    DBLogin --> CheckToken{登录成功?}
    CheckToken -->|失败| RetLogin500[{"code":500, 登录失败}]
    CheckToken -->|成功| GenToken[生成JWT Token]
    GenToken --> Blacklist[旧Token加入Redis黑名单]
    Blacklist --> RetLogin200[{"code":200, 返回游戏服务器地址+Token}]
    
    Registration & Login --> CatchEx[异常捕获: JSON/服务器错误]
    CatchEx --> RetEx[{"code":400/500, 返回错误信息}]