#pragma once
#include <string>
#include <vector>
#include <utility> 
#include "backpack.h"

class Merchant {
private:
    std::string name;
    std::vector<std::pair<std::string, int>> inventory; // ��Ʒ���ƺͼ۸�

public:
    // ���캯��
    Merchant(std::string n);

    // �����Ʒ
    void addItem(const std::string& itemName, int price);

    // ��ʾ��Ʒ�б�
    void displayInventory() const;

    // ������Ʒ
    bool purchaseItem(const std::string& itemName, int& playerGold, Backpack& backpack);
};