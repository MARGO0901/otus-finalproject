#include <malfunction.h>

bool Malfunction::isActive(const std::map<std::string, double> &params) const {
    for(const auto& [paramName, range] : conditions) {
        auto it = params.find(paramName);
        if (it != params.end()) {
            double value = it->second;
            if (value < range.first || value < range.second) {
                return true;
            }
        }
    }
    return false;
}