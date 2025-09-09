#pragma once
#include "battle.h"

class Enemy : public Life {
public:
    // 构造函数，直接调用 Life 的构造函数
    Enemy(std::string n, int h, int a, int d, int l, std::vector<Skill>& sks);

    // 敌人行为：随机选择技能攻击
    int attack(Life& target);
};