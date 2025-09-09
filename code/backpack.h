#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "weapon.h"
using namespace std;

class Backpack {
private:
    int money = 0; //万恶之源
    std::vector<Weapon> weapons;//武器汇总

    //武器总加成
    struct TotalBonus {   
        int hp = 0;                                 //物品总生命加成
        int martial = 0;                            //物品总武艺加成
        int will = 0;                               //物品总意志加成
        int luck = 0;                               //物品总幸运加成
    } currentBonus;
    
public:
    // 资金操作
    void addMoney(int amount); //赚钱了！！
    bool reduceMoney(int amount); //购物
    int getMoney() const; //数钱！诶嘿...

    // 武器管理
    void addWeapon(const Weapon& weapon); //获得武器
    bool removeWeapon(const string& weaponName); //失去武器

    // 获取所有武器名称（背包展示可用，如果不用背包展示就不用）
    std::vector<std::string> getWeaponNames() const;
    // 获取所有武器对象（直接返回vector引用）
    const vector<Weapon>& getWeapons() const;
};