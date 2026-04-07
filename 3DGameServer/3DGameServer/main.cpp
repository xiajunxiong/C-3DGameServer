#include <iostream>
#define BOOST_TIMER_ENABLE_DEPRECATED
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <libs/date_time/src/gregorian/greg_names.hpp>
#include <libs/date_time/src/gregorian/greg_month.cpp>
#include <libs/date_time/src/gregorian/gregorian_types.cpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <hiredis.h>
#include "GameWorld.h"
#include <chrono>
#include <vector>
#include "GameWorldObject.h"
#include "BoostService.h"
#include <thread>

bool isRunning = false;
std::vector<GameWorldObject*> worldObjects;
void startServer() {
	isRunning = true;
	std::cout << "Starting the game server..." << std::endl;
}
void stopServer() {
	isRunning = false;
	std::cout << "Stopping the game server..." << std::endl;
}
void runGameServer() {
    try
    {
        boost::asio::io_context io;
        TcpServer server(io, 12345); // 监听端口 12345

        std::cout << "异步TCP服务器已启动，端口 12345" << std::endl;
        io.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "异常: " << e.what() << std::endl;
    }
}

void startGame() {

}
void runGameLoop() {


    using Clock = std::chrono::high_resolution_clock;
    auto lastTime = Clock::now();
    const auto currentTime = Clock::now();
    const DeltaTime deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;
    for (auto obj : worldObjects)
    {
        obj->Update(deltaTime);
    }
}

void runRedis() {
    redisContext* ctx = redisConnect("127.0.0.1", 6379);

    if (ctx == nullptr || ctx->err != 0)
    {
        std::cout << "❌ 连接失败：" << (ctx ? ctx->errstr : "未知错误") << std::endl;
        if (ctx != nullptr)
        {
            redisFree(ctx);
        }
        return;
    }
    std::cout << "✅ Redis 连接成功！" << std::endl;

    redisReply* reply = (redisReply*)redisCommand(ctx, "SET test hello");
    if (reply == nullptr)
    {
        std::cout << "❌ SET 命令执行失败" << std::endl;
        redisFree(ctx);
        return;
    }
    freeReplyObject(reply);

    reply = (redisReply*)redisCommand(ctx, "GET test");
    if (reply != nullptr && reply->type == REDIS_REPLY_STRING)
    {
        std::cout << "✅ 读取结果：" << reply->str << std::endl;
    }
    else
    {
        std::cout << "❌ GET 命令执行失败 / 数据类型错误" << std::endl;
    }
    freeReplyObject(reply);

    redisFree(ctx);
    std::cout << "✅ Redis 操作完成" << std::endl;
}

//void test_boost_install() {
//	boost::timer t;
//	boost::gregorian::date dt(1978, 12, 18);
//	assert(dt.year() == 1978);
//	assert(dt.day() == 18);
//	boost::gregorian::date::ymd_type ymd = dt.year_month_day();
//	std::cout << "\n" << ymd.year << "/" << ymd.month << "/" << ymd.day << " the day is "
//		<< dt.day_of_year() << " days of this year" << std::endl;
//
//	std::cout << boost::gregorian::to_iso_extended_string(dt) << std::endl;
//	std::cout << boost::gregorian::to_iso_string(dt) << std::endl;
//	std::cout << boost::gregorian::to_simple_string(dt) << std::endl << std::endl;
//	std::cout << t.elapsed() << "s" << std::endl;
//	system("pause");
//
//}

#include <mutex>
#include <atomic>
#include <conio.h>
using namespace std;
mutex mtx;
GameWorld world;
void addMap() {
    this_thread::sleep_for(chrono::seconds(3));

    //lock_guard<mutex> lock(mtx);
    //worldObjects.push_back(new Map());
    //cout << "地图已添加！" << endl;
    world.Destroy("player_1001");
    Player* p1 = new Player();
    p1->objId = "player_1005";  // 像数据库ID
    world.Add(p1);
}


int main() {
	startServer();
	startGame();
	runRedis();
	//test_boost_install();
    
    //Player* p1 = new Player();
    //p1->objId = "player_1001";  // 像数据库ID
    //world.Add(p1);
    //Enemy* e1 = new Enemy();
    //e1->objId = "enemy_2001";
    //world.Add(e1);
    Map* m1 = new Map();
    m1->objId = "map_3001";
    world.Add(m1);
    thread t(addMap);
    t.detach();

	jthread serverThread(runGameServer);

	while (isRunning)
	{
        world.UpdateAll(0.016f);
		//runGameLoop();
	}

	return 0;
}





