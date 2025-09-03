#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <random>
#include "battle.h"
using namespace std;

//[1, n] 
int getRandom(int n) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, n);
    return dis(gen);
}

Skill::Skill(std::string n, std::string d, std::function<int(Life&, Life&)> f)
    : name(n), description(d), func(f) {}

int Skill::use(Life &self, Life &enemy) {
    return func(self, enemy);
}

Life::Life(string n ,int h, int a, int d, int l, std::vector<Skill> &sks)
    :name(n) ,hp(h), attack(a), defense(d), luck(l), skills(sks) {}


void fight(Life &l1, Life &l2) {
    int order; 
    while (l1.hp > 0 && l2.hp > 0) {
        //打印
        cout << endl << l1.name << " HP: " << l1.hp << " | " << l2.name << " HP: " << l2.hp << endl;

        //打印技能列表
        cout << "选择技能：" << endl;
        for (size_t i = 0; i < l1.skills.size(); ++i) {
            cout << i + 1 << ". [" << l1.skills[i].name << "] - " << l1.skills[i].description << endl;
        }
        //玩家选择技能
        cin>>order;
        int num = l1.skills[order-1].use(l1, l2);
        cout << l1.name << "使用[" << l1.skills[order-1].name << "]" << l1.skills[order-1].description << num << endl;

        if (l2.hp <= 0) {
            cout << "战斗胜利!" << endl;
            break;
        }
        //随机放技能
        order = getRandom(l2.skills.size());
        int num2 = l2.skills[order-1].use(l2, l1);
        cout << endl << l2.name << "对" << l1.name << "使用[" << l2.skills[order-1].name << "]" << l2.skills[order-1].description << num2 << endl;
        if (l1.hp <= 0) {
            cout << "遗憾失败" << endl;
            break;
        }
    }
}

//技能库(返回数值用来描述)
int func1(Life &self,Life &enemy){
    int num = self.attack * (50.0 / (50.0 + enemy.defense));
    enemy.hp-=num;
    return num;
}
int func2(Life &self,Life &enemy){
    int num = self.hp * 0.2;
    self.hp+=num;
    return num;
}
int func3(Life &self,Life &enemy){
    int num = self.attack * (50.0 / (50.0 + enemy.defense)) * 2;
    enemy.hp-=num;
    return num;
}

int main(){//测试用例

    //构造自己和对手的数值和技能
    vector<Skill> sks1;
    sks1.push_back(Skill("普攻","造成伤害",func1));
    sks1.push_back(Skill("治疗","恢复生命",func2));

    vector<Skill> sks2;
    sks2.push_back(Skill("咬","造成伤害",func1));
    sks2.push_back(Skill("狂暴","造成大量伤害",func3));

            //生命 攻击 防御 运气 技能列表
    Life l1("索尔维娅",100,20,5,1,sks1);
    Life l2("小狗",100,15,3,1,sks2);
    
    //战斗调用
    fight(l1,l2);
    return 0;
}