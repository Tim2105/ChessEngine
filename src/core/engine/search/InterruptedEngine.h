#ifndef INTERRUPTED_ENGINE_H
#define INTERRUPTED_ENGINE_H

#include "core/engine/search/Engine.h"

#include <chrono>
#include <functional>

class InterruptedEngine : public Engine {

    private:
        std::chrono::system_clock::time_point lastCheckupTime;
        std::chrono::milliseconds checkupInterval;

        std::function<void()> checkupCallback;

    protected:
        inline virtual bool isCheckupTime() {
            return std::chrono::system_clock::now() >= lastCheckupTime + checkupInterval;
        }

        inline virtual void checkup() {
            lastCheckupTime = std::chrono::system_clock::now();

            if(checkupCallback)
                checkupCallback();
        }

    public:
        InterruptedEngine(Evaluator& e, uint32_t numVariations = 1,
                          uint32_t checkupInterval = 2, std::function<void()> checkupCallback = nullptr)
            : Engine(e, numVariations), checkupInterval(checkupInterval), checkupCallback(checkupCallback) {
                lastCheckupTime = std::chrono::system_clock::now();
            }

        ~InterruptedEngine() {}

        virtual void setTime(uint32_t time, bool treatAsTimeControl = false) = 0;

        constexpr std::chrono::milliseconds getCheckupInterval() {
            return checkupInterval;
        }

        constexpr std::chrono::system_clock::time_point getLastCheckupTime() {
            return lastCheckupTime;
        }

        virtual inline void setCheckupCallback(std::function<void()> checkupCallback) {
            this->checkupCallback = checkupCallback;
        }
};


#endif
