#include "data/pqxx/PqxxData.h"
#include <bcrypt/bcrypt.h>
#include <jwt-cpp/jwt.h>

void PqxxData::Start() {
    try
    {
        // 重点：new 出来，存在成员变量里，不会被销毁
        conn = std::make_unique<pqxx::connection>(
            "dbname=3dGameDemo "
            "user=postgres "
            "password=123456 "
            "host=127.0.0.1 "
            "port=5432"
        );

        if (conn->is_open())
        {
            std::cout << "数据库连接成功！数据库名: " << conn->dbname() << std::endl;
        }
        else
        {
            std::cout << "数据库连接失败！" << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "异常: " << e.what() << std::endl;
        conn = nullptr;
    }
}

std::string PqxxData::Login(const std::string& account, const std::string& password)
{
    User user = QueryUser(account);
    if (user.uid == 0) return "";

    if (!bcrypt::validatePassword(password, user.password)) {
        std::cout << "[登录] 密码错误\n";
        return "";
    }

    UpdateLastLoginTime(account);
    return generateLoginToken(user.uid, user.account);
}

int PqxxData::AddUser(User user)
{
    if (!conn) return -1;

    try {
        pqxx::work tx(*conn);

        std::string hashed = bcrypt::generateHash(user.password, 12);

        tx.exec(
            "INSERT INTO \"user\".player_login (account, password, username, status) "
            "VALUES ($1, $2, $3, $4)",
            pqxx::params{
                user.account,
                hashed,
                user.username,
                user.status
            }
        );

        tx.commit();
        std::cout << "[DB] 添加用户成功: " << user.account << std::endl;

        return 0;
    }
    catch (const pqxx::unique_violation& e) {
        std::cerr << "[DB] 账号已存在: " << user.account << std::endl;
        return 1; 
    }
    catch (const std::exception& e) {
        std::cerr << "[DB] 添加用户失败: " << e.what() << std::endl;
        return -1;
    }
}

int PqxxData::AddUser(const std::string& username, const std::string& account, const std::string& password)
{
    User user{};
    user.username = username;
    user.account = account;
    user.password = password;
    user.status = 0;

    return AddUser(user);
}

void PqxxData::UpdateLastLoginTime(const std::string& account)
{
    if (!conn) return;

    try {
        pqxx::work tx(*conn);
        tx.exec(
            "UPDATE \"user\".player_login SET last_login_time = now() WHERE account = $1",
            pqxx::params{ account }
        );
        tx.commit();
    }
    catch (...) {}
}

User PqxxData::QueryUser(const std::string& account)
{
    if (!conn) return {};

    try {
        pqxx::work tx(*conn);

        auto res = tx.exec(
            "SELECT uid, account, username, password, status, create_time, last_login_time "
            "FROM \"user\".player_login WHERE account = $1",
            pqxx::params{ account }
        );

        if (res.empty()) return {};

        User user{};
        user.uid = res[0][0].as<long long>();
        user.account = res[0][1].as<std::string>();
        user.username = res[0][2].as<std::string>();
        user.password = res[0][3].as<std::string>();
        user.status = res[0][4].as<int>();
        user.create_time = res[0][5].as<std::string>();

        if (!res[0][6].is_null())
            user.last_login_time = res[0][6].as<std::string>();

        return user;
    }
    catch (const std::exception& e) {
        std::cerr << "[DB] 查询失败: " << e.what() << std::endl;
        return {};
    }
}



std::string PqxxData::generateLoginToken(long long uid, const std::string& account)
{
    auto expire = std::chrono::system_clock::now() + std::chrono::hours(24);
    auto expire_time = std::chrono::duration_cast<std::chrono::seconds>(expire.time_since_epoch()).count();

    std::string token = jwt::create()
        .set_issuer("game_server")
        .set_payload_claim("exp", jwt::claim(std::to_string(expire_time)))
        .set_payload_claim("uid", jwt::claim(std::to_string(uid)))
        .set_payload_claim("account", jwt::claim(account))
        .sign(jwt::algorithm::hs256{ "431982283@qq.com" });

    return token;
}

long long PqxxData::verifyToken(const std::string& token)
{
    try
    {

        auto decoded = jwt::decode(token);

        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ "431982283@qq.com" })  // 密钥必须和生成时一致
            .with_issuer("game_server");                                       // 签发者一致

        verifier.verify(decoded);

        // 4. 校验成功 → 取出 uid
        std::string uid_str = decoded.get_payload_claim("uid").as_string();
        return std::stoll(uid_str); // 转成 long long 并返回
    }
    catch (const std::exception& e)
    {
        std::cout << "[Token 校验失败] " << e.what() << std::endl;
        return -1; // 失败返回 -1
    }
}