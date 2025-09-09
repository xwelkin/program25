#include "enemy.h"
#include <iostream>
#include <random>

Enemy::Enemy(std::string n, int h, int a, int d, int l, std::vector<Skill>& sks)
    : Life(n, h, a, d, l, sks) {}

int Enemy::attack(Life& target) {
    // 随机选择技能
    int skillIndex = getRandom(skills.size());
    int damage = skills[skillIndex].use(*this, target);
    std::cout << name << " ʹ�ü��� [" << skills[skillIndex].name << "] �� " << target.name
              << " ����� " << damage << " ���˺���" << std::endl;
    return damage;
}



//暂定创造角色，技能待定义
Enemy createEnemyKarthus() {
    std::vector<Skill> skills = {sk1}; //  假设技能 sk1 已定义为普通攻击
    return Enemy("����", 200, 50, 100, 0, skills);
}

Enemy createEnemyLion() {
    std::vector<Skill> skills = {sk1, sk2}; // 假设技能 sk2 为 "嘶吼"
    return Enemy("ʨ��", 400, 70, 40, 0, skills);
}

Enemy createEnemyRomanWarrior() {
    std::vector<Skill> skills = {sk1, sk3, sk4}; // 假设技能 sk3 为 "防御"，sk4 为 "突击"
    return Enemy("�����Ƕ�ʿ", 500, 80, 100, 0, skills);
}

Enemy createEnemyLionKing() {
    std::vector<Skill> skills = {sk1, sk5, sk6}; //  假设技能 sk5 为 "绝对防御"，sk6 为 "逆势攻击"
    return Enemy("����¶�", 800, 100, 100, 0, skills);
}

Enemy createEnemyLionKingSearch() {
    std::vector<Skill> skills = {sk1, sk7, sk8}; //假设技能 sk7 为 "震慑意志"，sk8 为 "三月一斩"
    return Enemy("ʨ��ŵ��", 1000, 120, 120, 0, skills);
}