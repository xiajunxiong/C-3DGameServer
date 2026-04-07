#pragma once
#include "GameWorldObject.h"

class GameWorld
{
public:

    std::map<std::string, GameWorldObject*> objects;

    // 添加对象
    void Add(GameWorldObject* obj)
    {
        objects[obj->objId] = obj;
    }

    // 查找对象
    GameWorldObject* Find(const std::string& id)
    {
        auto it = objects.find(id);
        if (it != objects.end())
            return it->second;
        return nullptr;
    }

    // 销毁对象（你最需要的）
    void Destroy(const std::string& id)
    {
        auto it = objects.find(id);
        if (it != objects.end())
        {
            delete it->second;
            objects.erase(it);
            std::cout << "✅ 销毁对象：" << id << "\n";
        }
    }

    // 更新所有对象
    void UpdateAll(DeltaTime dt)
    {
        for (auto& pair : objects)
        {
            pair.second->Update(dt);
        }
    }

    ~GameWorld()
    {
        for (auto& pair : objects)
            delete pair.second;
        objects.clear();
    }

};

