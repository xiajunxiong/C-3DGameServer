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
	std::string create_time; // 注册创建不可改动
	std::string last_login_time;
	std::string offline_time;
};

class PqxxData
{
public:
	void Start();

	std::string Login(const std::string& account, const std::string& password);

	int AddUser(User user);
	int AddUser(const std::string& username, const std::string& account, const std::string& password);
	void UpdateLastLoginTime(const std::string& account);
	User QueryUser(const std::string& account);
private:

	std::unique_ptr<pqxx::connection> conn;

	std::string generateLoginToken(long long uid, const std::string& account);
	long long verifyToken(const std::string& token);
};

