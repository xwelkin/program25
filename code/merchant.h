#pragma once
#include <string>
#include <vector>
#include <utility> 
#include "backpack.h"

class Merchant {
private:
    std::string name;
    std::vector<std::pair<std::string, int>> inventory; // 商品名称和价格


public:
    // 构造函数
    Merchant(std::string n);

    //  添加商品
    void addItem(const std::string& itemName, int price);

    //显示商品列表
    void displayInventory() const;

    // 购买商品
    bool purchaseItem(const std::string& itemName, int& playerGold, Backpack& backpack);
};