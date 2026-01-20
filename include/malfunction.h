#pragma once

#include <string>
#include <map>
#include <vector>

struct Solution {
    Solution(const std::string& desc = "", int sc = 0, const std::string& cmt = "")
        : description(desc), score(sc), comment(cmt) {}

    virtual ~Solution() = default;

    std::string description;
    int score;
    std::string comment;
};


class Optimal : public Solution {
public:
    explicit Optimal(std::string desc, int sc) : Solution(std::move(desc), sc, "Правильно!") {}
};


class Accept : public Solution {
public:
    explicit Accept(std::string desc, int sc) : Solution(std::move(desc), sc, "Хорошо, но можно лучше") {}
};


class Critical : public Solution {
public:
    explicit Critical(std::string desc, int sc) : Solution(std::move(desc), sc, "Минимально допустимое решение") {}
};


class Failure : public Solution {
public:
    explicit Failure(std::string desc, int sc) : Solution(std::move(desc), sc, "Неправильно! Действия приведут к аварии") {}
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
