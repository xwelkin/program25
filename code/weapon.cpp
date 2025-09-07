#include "weapon.h"

Weapon::Weapon(std::string name, std::string description, int hp ,int martial, int will, int luck)
    : name(name), description(description), hp(hp), martial(martial), will(will), luck(luck), skill(nullptr) {}

Weapon::Weapon(std::string name, std::string description, int hp ,int martial, int will, int luck, const Skill& skill)
    : name(name), description(description), hp(hp), martial(martial), will(will), luck(luck), skill(new Skill(skill)) {}

const string& Weapon::getName() const { 
    return name; 
}
int Weapon::getHp() const { 
    return hp; 
}
int Weapon::getMartial() const { 
    return martial; 
}
int Weapon::getWill() const { 
    return will; 
}
int Weapon::getLuck() const { 
    return luck; 
}

void Weapon::setSkill(Skill* skill) { 
    this->skill = skill; 
}