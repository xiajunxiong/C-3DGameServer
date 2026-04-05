#pragma once
#include <iostream>

using DeltaTime = float;

class GameWorldObject
{
public:
	virtual ~GameWorldObject() = default;
    virtual void Update(DeltaTime deltaTime) = 0;

};

class Player : public GameWorldObject
{
public:
    void Update(DeltaTime deltaTime) override
    {
        std::cout << "Player update" << std::endl;
    }
};
class Enemy : public GameWorldObject
{
public:
    void Update(DeltaTime deltaTime) override
    {
        std::cout << "Enemy update" << std::endl;
    }
};
class Map : public GameWorldObject
{
public:
    void Update(DeltaTime deltaTime) override
    {
        std::cout << "Map update" << std::endl;
    }

};
