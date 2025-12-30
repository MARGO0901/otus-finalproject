#pragma once

#include <string>
#include <map>
#include <vector>

struct Solution {
    std::string description;
    int score;
    std::string comment;
};

class Malfunction {
public: 
    std::string name;               // "Перегрев"
    std::string description;        // "Температура выше нормы"

    // Условия активации: {"температура", {min, max}}
    std::map<std::string, std::pair<double, double>> conditions;
    //Варианты решений
    std::vector<Solution> solutions;

    // Проверить, активна ли неисправность для данных параметров
    bool isActive(const std::map<std::string, double> &params) const;
};
