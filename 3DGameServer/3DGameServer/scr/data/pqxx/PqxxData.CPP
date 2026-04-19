#include "data/pqxx/PqxxData.h"

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

int PqxxData::AddUser(User user)
{
    //if (!conn) return -1;

    //try {
    //    pqxx::work tx(*conn);
    //    //pqxx::time_point now = pqxx::now();
    //    
    //    tx.exec_params(
    //        "INSERT INTO users (account, password, username, status) "
    //        "VALUES ($1, $2, $3, $4)",
    //        user.account,
    //        user.password,
    //        user.username,
    //        user.status
    //    );
    //    tx.commit();
    //    return 0;
    //}
    //catch (const std::exception& e) {
    //    std::cerr << "添加失败：" << e.what() << std::endl;
    //    return -1;
    //}
    return 0;
}

User PqxxData::QueryUser(int Account)
{
	return User();
}
