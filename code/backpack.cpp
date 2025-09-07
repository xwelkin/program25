#include "backpack.h"
#include <algorithm>
#include <stdexcept>

// 资金操作
void Backpack::addMoney(int amount) { 
    money += amount; //加钱！
}

bool Backpack::reduceMoney(int amount) {
    if (money >= amount) {
        money -= amount;
        return true;
    }
    return false;   //没钱了！
}

int Backpack::getMoney() const { 
    return money;   //查询余额
}

// 武器管理
void Backpack::addWeapon(const Weapon& weapon) {   //获得武器并加成
    weapons.push_back(weapon);
    currentBonus.hp += weapon.getHp();
    currentBonus.martial += weapon.getMartial();
    currentBonus.will += weapon.getWill();
    currentBonus.luck += weapon.getLuck();
}

bool Backpack::removeWeapon(const string& weaponName) {   //失去武器减去属性
    auto it = find_if(weapons.begin(), weapons.end(),
        [&](const Weapon& w) { return w.getName() == weaponName; });
    if (it != weapons.end()) {
        currentBonus.hp -= it->getHp();
        currentBonus.martial -= it->getMartial();
        currentBonus.will -= it->getWill();
        currentBonus.luck -= it->getLuck();
        
        weapons.erase(it);
        return true;
    }
    return false;
}

vector<string> Backpack::getWeaponNames() const {
    vector<string> names;
    for (const auto& weapon : weapons) {
        names.push_back(weapon.getName());
    }
    return names;
}

const vector<Weapon>& Backpack::getWeapons() const {
    return weapons;
}