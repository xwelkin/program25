#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "battle.h"
using namespace std;

class Weapon {
private:
    string name;
    string description;
    int hp = 0;
    int martial = 0;
    int will = 0;
    int luck = 0;
    Skill* skill = nullptr; // 使用Skill类的指针

public:
    Weapon(std::string name, std::string description, int hp ,int martial, int will, int luck);
    Weapon(std::string name, std::string description, int hp, int martial, int will, int luck, const Skill& skill);
    const string& getName() const;
    int getHp() const;
    int getMartial() const;
    int getWill() const;
    int getLuck() const;

    // 设置武器技能
    void setSkill(Skill* skill);
    
    // 检查是否有技能
    bool hasSkill() const { return skill != nullptr; }
    
    // 获取技能
    Skill* getSkill() const { return skill; }
};