#pragma once

#include <iostream>
#include <atomic>

#define DEBUG_ALPHA 0.9

/*
    we're using relaxed memory ordering as we don't *really* care about how recent the information is, and just that the actions are done atomically
    which is kind of wonky otherwise in emscripten for some reason
*/
struct DebugTimer {
    std::atomic<int> count;
    std::atomic<double> lastTime;
    std::atomic<double> averageTime;

    DebugTimer() : count(0), lastTime(0), averageTime(0) {}

    void newEntry(double time) {
        count++;
        lastTime.store(time, std::memory_order_relaxed);
        averageTime = DEBUG_ALPHA * time + (1 - DEBUG_ALPHA) * averageTime;
    }

    int getCount() {
        return count.load(std::memory_order_relaxed);
    }

    double getLastTime() {
        return lastTime.load(std::memory_order_relaxed);
    }
    
    double getAverageTime() {
        return averageTime.load(std::memory_order_relaxed);
    }
};

struct DebugInfo {
    DebugTimer render;
    DebugTimer mapgen;
};

extern DebugInfo debug;
