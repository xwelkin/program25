#include "enemy.h"
#include <iostream>
#include <random>

Enemy::Enemy(std::string n, int h, int a, int d, int l, std::vector<Skill>& sks)
    : Life(n, h, a, d, l, sks) {}

int Enemy::attack(Life& target) {
    // ���ѡ����
    int skillIndex = getRandom(skills.size());
    int damage = skills[skillIndex].use(*this, target);
    std::cout << name << " ʹ�ü��� [" << skills[skillIndex].name << "] �� " << target.name
              << " ����� " << damage << " ���˺���" << std::endl;
    return damage;
}



//�ݶ������ɫ�����ܴ�����
Enemy createEnemyKarthus() {
    std::vector<Skill> skills = {sk1}; // ���輼�� sk1 �Ѷ���Ϊ��ͨ����
    return Enemy("����", 200, 50, 100, 0, skills);
}

Enemy createEnemyLion() {
    std::vector<Skill> skills = {sk1, sk2}; // ���輼�� sk2 Ϊ "˻��"
    return Enemy("ʨ��", 400, 70, 40, 0, skills);
}

Enemy createEnemyRomanWarrior() {
    std::vector<Skill> skills = {sk1, sk3, sk4}; // ���輼�� sk3 Ϊ "����"��sk4 Ϊ "ͻ��"
    return Enemy("����Ƕ�ʿ", 500, 80, 100, 0, skills);
}

Enemy createEnemyLionKing() {
    std::vector<Skill> skills = {sk1, sk5, sk6}; // ���輼�� sk5 Ϊ "���Է���"��sk6 Ϊ "���ƹ���"
    return Enemy("����¶�", 800, 100, 100, 0, skills);
}

Enemy createEnemyLionKingSearch() {
    std::vector<Skill> skills = {sk1, sk7, sk8}; // ���輼�� sk7 Ϊ "������־"��sk8 Ϊ "����һն"
    return Enemy("ʨ��ŵ��", 1000, 120, 120, 0, skills);
}