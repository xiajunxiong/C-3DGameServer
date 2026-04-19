#pragma once
#include <iostream>
#include <pqxx/pqxx>

enum OperationType
{
	// 增
	Add,
	// 删
	Delete,
	// 改
	Change,
	// 查
	Query
};

struct User {
	long long uid = 0; // 自增id不可改动
	std::string account;
	std::string username;
	std::string password;
	int status = 0; // 0 状态正常 1 被管理员封禁
	std::chrono::system_clock::time_point create_time;// 注册创建不可改动
	std::chrono::system_clock::time_point last_login_time;
	std::chrono::system_clock::time_point offline_time;
};

class PqxxData
{
public:
	void Start();

	int AddUser(User user);
	User QueryUser(int Account);
private:

	std::unique_ptr<pqxx::connection> conn;
};

