#include "merchant.h"
#include <iostream>

Merchant::Merchant(std::string n) : name(n) {}

void Merchant::addItem(const std::string& itemName, int price) {
    inventory.emplace_back(itemName, price);
}

void Merchant::displayInventory() const {
    std::cout << "商人 " << name << " 的商品列表：" << std::endl;
    for (const auto& item : inventory) {
        std::cout << "- " << item.first << ": " << item.second << " 金币" << std::endl;
    }
}

bool Merchant::purchaseItem(const std::string& itemName, int& playerGold, Backpack& backpack) {
    for (auto it = inventory.begin(); it != inventory.end(); ++it) {
        if (it->first == itemName) {
            if (playerGold >= it->second) {
                playerGold -= it->second;
                inventory.erase(it);
                Weapon newWeapon(itemName, "购买的武器", 10, 10, 0, 0); // 假设购买的物品是武器
                backpack.addWeapon(newWeapon);
                std::cout << "成功购买了 [" << itemName << "]！" << std::endl;
                return true;
            } else {
                std::cout << "金币不足，无法购买 [" << itemName << "]！" << std::endl;
                return false;
            }
        }
    }
    std::cout << "商品 [" << itemName << "] 不存在！" << std::endl;
    return false;
}