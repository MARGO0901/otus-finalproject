#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void updateMessage(const std::string& message) = 0;
    virtual void updateMood(const std::string& mood) = 0;
    virtual void updateBody() = 0;
};


class ObserverManager {
protected:
    std::vector<std::weak_ptr<IObserver>> observers_;

public:
    virtual ~ObserverManager() = default;

    void addObserver(std::weak_ptr<IObserver> observer);
    void removeObserver(const std::shared_ptr<IObserver>& observer);
    void notifyMessage(const std::string& message);
    void notifyMood(const std::string& message);
    void cleanupObservers();                        // для очистки мертвых указателей
    size_t countObservers() const;
    void notifyRedraw();
};