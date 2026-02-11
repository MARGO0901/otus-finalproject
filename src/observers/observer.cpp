#include <observers/observer.h>

void ObserverManager::addObserver(std::weak_ptr<IObserver> observer) {
    observers_.push_back(observer);
}


void ObserverManager::removeObserver(const std::shared_ptr<IObserver>& observer) {
    cleanupObservers();
    observers_.erase(
        std::remove_if(observers_.begin(), observers_.end(),
            [&observer](const std::weak_ptr<IObserver>& weakObs) {
                auto obs = weakObs.lock();
                return obs && obs == observer;
            }),
        observers_.end()
    );
}


void ObserverManager::notifyMessage(const std::string& message) {
    cleanupObservers();
    for (auto it = observers_.begin(); it != observers_.end();) {
        if (auto observer = it->lock()) {
            observer->updateMessage(message);
            ++it;
        } else {
            it = observers_.erase(it);
        }
    }
}


void ObserverManager::notifyMood(const std::string& message) {
    cleanupObservers();
    for (auto it = observers_.begin(); it != observers_.end();) {
        if (auto observer = it->lock()) {
            observer->updateMood(message);
            ++it;
        } else {
            it = observers_.erase(it);
        }
    }
}


// для очистки мертвых указателей
void ObserverManager::cleanupObservers() {
    observers_.erase(
        std::remove_if(observers_.begin(), observers_.end(),
            [](const std::weak_ptr<IObserver>& weakObs){
                return weakObs.expired();
            }),
        observers_.end()
    );
}


size_t ObserverManager::countObservers() const {
    return observers_.size();
}

void ObserverManager::notifyRedraw() {
    cleanupObservers();
    for (auto it = observers_.begin(); it != observers_.end();) {
        if (auto observer = it->lock()) {
            observer->updateBody();
            ++it;
        } else {
            it = observers_.erase(it);
        }
    }
}