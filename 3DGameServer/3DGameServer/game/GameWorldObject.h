#pragma once
#include <iostream>
#include <map>

using DeltaTime = float;

class GameWorldObject
{
public:
    std::string objId;
    virtual ~GameWorldObject() = default;
    virtual void Update(DeltaTime deltaTime) = 0;
};

class Player : public GameWorldObject
{
public:
    void Update(DeltaTime deltaTime) override
    {
        std::cout << "玩家[" << objId << "] 更新\n";
    }
};
class Enemy : public GameWorldObject
{
public:
    void Update(DeltaTime deltaTime) override
    {
        std::cout << "敌人[" << objId << "] 更新\n";
    }
};

class Map : public GameWorldObject
{
public:
    void Update(DeltaTime deltaTime) override
    {
        std::cout << "地图[" << objId << "] 更新\n";
    }

};

