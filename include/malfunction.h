#pragma once

#include <string>
#include <map>
#include <vector>

struct Solution {
    Solution(const std::string& desc = "", int sc = 0, const std::string& cmt = "")
        : description_(desc), score_(sc), comment_(cmt) {}

    virtual ~Solution() = default;

    std::string description_;
    int score_;
    std::string comment_;
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
    std::string name_;               // "Перегрев"

    // Условия активации: {"температура", {min, max}}
    std::map<std::string, std::pair<double, double>> conditions_;
    //Варианты решений
    std::vector<Solution> solutions_;
};
