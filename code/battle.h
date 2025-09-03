#pragma once
#include <string>
#include <vector>
#include <functional>

class Life;
class Skill {
public:
    std::string name;
    std::string description;
    std::function<int(Life&, Life&)> func;
    int use(Life &self, Life &enemy);
    Skill(std::string n, std::string d, std::function<int(Life&, Life&)> f);
};

class Life {
public:
    std::string name;
    int hp;
    int attack;
    int defense;
    int luck;
    std::vector<Skill> skills;
    Life(std::string n ,int h, int a, int d, int l, std::vector<Skill> &sks);
};

void fight(Life &l1, Life &l2);