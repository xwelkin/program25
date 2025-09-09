#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <random>
#include "battle.h"
using namespace std;


int getRandom(int n) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, n);
    return dis(gen);
}
int is_criticalstrike(int x, int luck) {
    static random_device rd;
    static mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    float probability = std::min(static_cast<float>(luck) / 100.0f, 1.0f);
    float randomValue = dis(gen);
    if (randomValue < probability) {
        return x;
    } else {
        return 1;
    }
}

Skill::Skill(std::string n, std::string d, std::function<int(Life&, Life&)> f)
    : name(n), description(d), func(f) {}

int Skill::use(Life &self, Life &enemy) {
    return func(self, enemy);
}

Life::Life(string n ,int h, int a, int d, int l, std::vector<Skill> &sks)
    :name(n) ,maxhp(h), hp(h), maxattack(a), attack(a), maxdefense(d), defense(d), maxluck(l), luck(l), skills(sks) {}


void fight(Life &l1, Life &l2, const Backpack& backpack) {
    cout << "=== 武器效果生效 ===" << endl;
    const vector<Weapon>& weapons = backpack.getWeapons();
    for (const auto& weapon : weapons) {
        if (weapon.hasSkill()) {
            Skill* skill = weapon.getSkill();
            cout << l1.name << " 的武器 [" << weapon.getName() << "] 发动 [" 
             << skill->name << "] - " << skill->description << "！" << endl;
            skill->use(l1, l2);
        }
    }
    cout << "==================" << endl;

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


//技能功能函数
int func1(Life &self,Life &enemy){
    int num = self.attack * (50.0 / (50.0 + enemy.defense)) * is_criticalstrike(2, self.luck);   //普攻伤害计算
    enemy.hp-=num;
    return num;
}

int func2(Life &self,Life &enemy){
    int num = self.maxhp * 0.2;                                  //生命恢复20%
    if(self.hp + num > self.maxhp) num = self.maxhp - self.hp;   //防止溢出
    self.hp+=num;
    return num;
}
int func3(Life &self,Life &enemy){
    int num = func1(self, enemy) + 20; //剑气斩伤害计算
    enemy.hp-=num;
    return num;
}
int func4(Life &self,Life &enemy){
    int num = enemy.maxattack;                                    //斩杀
    enemy.hp-=num;
    return num;
}
int func5(Life &self,Life &enemy){
    int num = enemy.maxhp*0.2;                                //削弱对方20%生命值
    if(enemy.hp - num < 0) num = enemy.hp;                //防止生命值为负
    enemy.hp-=num;
    return num;
}
int func6(Life &self,Life &enemy){
    int num = enemy.maxattack*0.2;                                //削弱对方20%攻击力
    if(enemy.attack - num < 0) num = enemy.attack;                //防止攻击为负
    enemy.attack-=num;
    return num;
}
int func7(Life &self,Life &enemy){
    int num = enemy.maxdefense*0.2;                                //削弱对方20%防御力
    if(enemy.defense - num < 0) num = enemy.defense;                //防止防御为负
    enemy.defense-=num;
    return num;
}
int func8(Life &self,Life &enemy){
    int num = enemy.maxluck*0.2;                                //削弱对方20%运气
    if(enemy.luck - num < 0) num = enemy.luck;                //防止运气为负
    enemy.luck-=num;
    return num;
}

//技能库

Skill sk1("普攻","造成伤害",func1);
Skill sk2("治疗","恢复生命",func2);
Skill sk3("剑气斩","对敌人造成额外伤害",func3);
Skill sk4("大荒囚天指","直接将敌人斩杀",func4);
Skill sk5("削弱生命","削弱敌人20%生命值",func5);
Skill sk6("削弱攻击","削弱敌人20%攻击力",func6);
Skill sk7("削弱防御","削弱敌人20%防御力",func7);
Skill sk8("削弱运气","削弱敌人20%运气",func8);

//武器库
Weapon w1("长剑","角斗场上常见的武器",10,10,0,0);
Weapon w2("破伤风之刃","从微观上瓦解敌人的兵器",0,10,0,10,sk7);
Weapon w3("屠龙刀","屠龙的利器",20,20,0,0,sk3);
Weapon w4("倚天剑","宝剑中的极品",30,30,0,0,sk5);
Weapon w5("紫色心情","令人开心的神器",0,0,0,0,sk8);
Weapon w6("鲁国的纸","传承之纸",0,0,0,0,sk8);

//测试用例
int main(){

    //构造自己和对手的数值和技能
    vector<Skill> sks1;
    sks1.push_back(sk1);
    sks1.push_back(sk2);
    sks1.push_back(sk3);
    sks1.push_back(sk4);

    vector<Skill> sks2;
    sks2.push_back(sk1);
    sks2.push_back(sk4);

    
    //构造背包和武器
    Backpack player_backpack;
    player_backpack.addWeapon(w1);
    player_backpack.addWeapon(w2);

    //生命 攻击 防御 运气 技能列表
    Life l1("索尔维娅",100,20,5,1,sks1);
    Life l2("小狗",100,15,3,1,sks2);
    
    //战斗调用
    fight(l1,l2,player_backpack);
    return 0;
}