#pragma once
#include "battle.h"

class Enemy : public Life {
public:
    // ���캯����ֱ�ӵ��� Life �Ĺ��캯��
    Enemy(std::string n, int h, int a, int d, int l, std::vector<Skill>& sks);

    // ������Ϊ�����ѡ���ܹ���
    int attack(Life& target);
};