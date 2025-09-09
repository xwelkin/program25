#include "merchant.h"
#include <iostream>

Merchant::Merchant(std::string n) : name(n) {}

void Merchant::addItem(const std::string& itemName, int price) {
    inventory.emplace_back(itemName, price);
}

void Merchant::displayInventory() const {
    std::cout << "���� " << name << " ����Ʒ�б�" << std::endl;
    for (const auto& item : inventory) {
        std::cout << "- " << item.first << ": " << item.second << " ���" << std::endl;
    }
}

bool Merchant::purchaseItem(const std::string& itemName, int& playerGold, Backpack& backpack) {
    for (auto it = inventory.begin(); it != inventory.end(); ++it) {
        if (it->first == itemName) {
            if (playerGold >= it->second) {
                playerGold -= it->second;
                inventory.erase(it);
                Weapon newWeapon(itemName, "���������", 10, 10, 0, 0); // ���蹺�����Ʒ������
                backpack.addWeapon(newWeapon);
                std::cout << "�ɹ������� [" << itemName << "]��" << std::endl;
                return true;
            } else {
                std::cout << "��Ҳ��㣬�޷����� [" << itemName << "]��" << std::endl;
                return false;
            }
        }
    }
    std::cout << "��Ʒ [" << itemName << "] �����ڣ�" << std::endl;
    return false;
}